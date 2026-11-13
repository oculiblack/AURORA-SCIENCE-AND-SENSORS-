#include <Wire.h>
#include "SparkFun_AS7265X.h"        // sensor library

AS7265X sensor;                      // the sensor object

float white860 = 0, white940 = 0;    // white reference values (filled by 'w')
bool haveWhite = false;              // true only after a GOOD white reference

const int MAX_POINTS = 20;           // maximum number of calibration samples
float xs[MAX_POINTS];                // stores the moisture index of each sample
float ys[MAX_POINTS];                // stores the known moisture % of each sample
int n = 0;                           // how many samples are stored

// Read 860 nm and 940 nm ten times and average them
// Returns true only if the readings are valid
bool readRaw(float &v860, float &v940) {
  float s860 = 0, s940 = 0;
  for (int i = 0; i < 10; i++) {
    sensor.takeMeasurementsWithBulb();     // LEDs on, then measure
    s860 += sensor.getCalibratedW();       // 860 nm channel
    s940 += sensor.getCalibratedL();       // 940 nm channel
    delay(50);                             // short pause between readings
  }
  v860 = s860 / 10.0;                      // average
  v940 = s940 / 10.0;
  if (isnan(v860) || isnan(v940)) return false;   // sensor gave nan
  if (v860 <= 0 || v940 <= 0) return false;       // sensor gave zero
  return true;                                    // readings are fine
}

// Work out the best straight line: moisture = A * index + B
void fitLine() {
  if (n < 2) { Serial.println("Need at least 2 samples"); return; }

  float sx = 0, sy = 0, sxy = 0, sxx = 0;
  for (int i = 0; i < n; i++) {
    sx  += xs[i];
    sy  += ys[i];
    sxy += xs[i] * ys[i];
    sxx += xs[i] * xs[i];
  }
  float denom = n * sxx - sx * sx;
  if (fabs(denom) < 1e-9) { Serial.println("All index values are the same, check the sensor"); return; }

  float A = (n * sxy - sx * sy) / denom;   // slope
  float B = (sy - A * sx) / n;             // intercept

  float meanY = sy / n;
  float ssTot = 0, ssRes = 0;
  for (int i = 0; i < n; i++) {
    float pred = A * xs[i] + B;
    ssRes += (ys[i] - pred) * (ys[i] - pred);
    ssTot += (ys[i] - meanY) * (ys[i] - meanY);
  }
  float r2 = (ssTot > 0) ? 1.0 - ssRes / ssTot : 0;

  Serial.println("---- COPY THESE INTO THE MEASUREMENT SKETCH ----");
  Serial.print("const float moistA = "); Serial.print(A, 4); Serial.println(";");
  Serial.print("const float moistB = "); Serial.print(B, 4); Serial.println(";");
  Serial.print("Fit quality R2 = "); Serial.println(r2, 3);
  if (r2 < 0.9) Serial.println("Warning: poor fit, check height, filling, and samples");
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);                              // SDA = 21, SCL = 22
  Wire.setClock(100000);                           // FIX 1: slow I2C to 100 kHz
  Wire.setTimeOut(1000);                           // FIX 1: give the sensor time to answer
  if (!sensor.begin()) { Serial.println("Sensor not found"); while (1); }
  sensor.disableIndicator();                       // blue LED off
  sensor.setGain(AS7265X_GAIN_16X);                // keep same as measurement sketch
  sensor.setIntegrationCycles(49);                 // keep same as measurement sketch
  Serial.println("MOISTURE CALIBRATION");
  Serial.println("t = test live readings | w = white reference | a 20 = add sample");
  Serial.println("l = list | d = delete last | f = fit | r = reset");
}

void loop() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;
  char cmd = line.charAt(0);

  if (cmd == 't') {                                // ---- NEW: test raw readings ----
    sensor.takeMeasurements();                     // room light only, LEDs off
    Serial.print("No LEDs   860: "); Serial.print(sensor.getCalibratedW());
    Serial.print("  940: "); Serial.println(sensor.getCalibratedL());
    sensor.takeMeasurementsWithBulb();             // white LEDs on
    Serial.print("With LEDs 860: "); Serial.print(sensor.getCalibratedW());
    Serial.print("  940: "); Serial.println(sensor.getCalibratedL());
  }

  else if (cmd == 'w') {                           // ---- white reference ----
    if (!readRaw(white860, white940)) {            // FIX 2: reject bad readings
      haveWhite = false;                           // do not accept a bad reference
      Serial.println("Bad white reading (0 or nan). Type t to test the sensor");
      return;
    }
    haveWhite = true;
    Serial.print("White 860 = "); Serial.print(white860, 2);
    Serial.print("  White 940 = "); Serial.println(white940, 2);
    if (white860 > 60000 || white940 > 60000) Serial.println("Saturated, lower the gain");
  }

  else if (cmd == 'a') {                           // ---- add a sample ----
    if (!haveWhite) { Serial.println("Take a good white reference first (w)"); return; }
    if (line.length() < 2) { Serial.println("Type like: a 20"); return; }
    if (n >= MAX_POINTS) { Serial.println("Memory full, press f or r"); return; }
    float known = line.substring(1).toFloat();
    float v860, v940;
    if (!readRaw(v860, v940)) {                    // FIX 2: reject bad readings
      Serial.println("Bad reading, sample NOT stored. Try again");
      return;
    }
    float r860 = v860 / white860;
    float r940 = v940 / white940;
    float index = (r860 - r940) / (r860 + r940);
    xs[n] = index;
    ys[n] = known;
    n++;
    Serial.print("Sample "); Serial.print(n);
    Serial.print(": moisture "); Serial.print(known, 1);
    Serial.print("%  index "); Serial.println(index, 4);
  }

  else if (cmd == 'l') {
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1); Serial.print(": ");
      Serial.print(ys[i], 1); Serial.print("%  index ");
      Serial.println(xs[i], 4);
    }
  }

  else if (cmd == 'd') {
    if (n > 0) { n--; Serial.println("Last sample removed"); }
  }

  else if (cmd == 'f') fitLine();

  else if (cmd == 'r') { n = 0; Serial.println("All samples cleared"); }
}