#include <Wire.h>
#include "SparkFun_AS7265X.h"

AS7265X sensor;                          // the sensor object
float white[18];                         // stores the white reference (18 channels)
bool haveWhite = false;                  // becomes true after you press 'w'

// Reads all 18 channels 10 times and averages them (averaging reduces noise)
void readAll(float out[18]) {
  for (int i = 0; i < 18; i++) out[i] = 0;          // start the sums at zero
  for (int n = 0; n < 10; n++) {                     // repeat 10 times
    sensor.takeMeasurementsWithBulb();               // LEDs on, then measure
    out[0]  += sensor.getCalibratedA();  // 410 nm
    out[1]  += sensor.getCalibratedB();  // 435 nm
    out[2]  += sensor.getCalibratedC();  // 460 nm
    out[3]  += sensor.getCalibratedD();  // 485 nm
    out[4]  += sensor.getCalibratedE();  // 510 nm
    out[5]  += sensor.getCalibratedF();  // 535 nm
    out[6]  += sensor.getCalibratedG();  // 560 nm
    out[7]  += sensor.getCalibratedH();  // 585 nm
    out[8]  += sensor.getCalibratedR();  // 610 nm
    out[9]  += sensor.getCalibratedI();  // 645 nm
    out[10] += sensor.getCalibratedS();  // 680 nm
    out[11] += sensor.getCalibratedJ();  // 705 nm
    out[12] += sensor.getCalibratedT();  // 730 nm
    out[13] += sensor.getCalibratedU();  // 760 nm
    out[14] += sensor.getCalibratedV();  // 810 nm
    out[15] += sensor.getCalibratedW();  // 860 nm
    out[16] += sensor.getCalibratedK();  // 900 nm
    out[17] += sensor.getCalibratedL();  // 940 nm
  }
  for (int i = 0; i < 18; i++) out[i] /= 10.0;       // sum divided by 10 = average
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  if (!sensor.begin()) { Serial.println("Sensor not found"); while (1); }
  sensor.disableIndicator();                         // blue LED off
  sensor.setGain(AS7265X_GAIN_16X);                  // keep the same in all sketches
  sensor.setIntegrationCycles(49);                   // keep the same in all sketches
  Serial.println("Type w = white reference, s = sample");
}

void loop() {
  if (!Serial.available()) return;                   // wait for a typed command
  char c = Serial.read();                            // read one character

  if (c == 'w') {                                    // ---- white reference ----
    readAll(white);                                  // measure the white sheet
    haveWhite = true;
    Serial.print("const float WHITE[18] = {");       // prints a line to paste later
    for (int i = 0; i < 18; i++) {
      Serial.print(white[i], 2);
      if (i < 17) Serial.print(", ");
    }
    Serial.println("};");
  }

  if (c == 's') {                                    // ---- sample ----
    if (!haveWhite) { Serial.println("Press w first"); return; }
    float v[18];
    readAll(v);                                      // measure the sample
    for (int i = 0; i < 18; i++) {
      Serial.print(v[i] / white[i], 4);              // reflectance = sample / white
      if (i < 17) Serial.print(",");                 // comma-separated for a spreadsheet
    }
    Serial.println();
  }
}