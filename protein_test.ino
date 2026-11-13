#include <Wire.h>                 // I2C communication library
#include <Adafruit_TCS34725.h>    // Library for the color sensor

// Sensor object: 154 ms reading time, 4x gain
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_4X);

float baselineHue = 0;            // Hue (in degrees) of the blue biuret reagent
float hueThreshold = 20.0;        // How many degrees of change counts as "color changed"

// This function converts R, G, B values into a hue angle (0 to 360 degrees).
// Blue is around 200-240, violet/purple is around 260-300.
float getHue(float r, float g, float b) {
  float maxVal = max(r, max(g, b));      // Biggest of the three colors
  float minVal = min(r, min(g, b));      // Smallest of the three colors
  float delta = maxVal - minVal;         // Difference between them

  if (delta == 0) return 0;              // No color (gray), so hue is 0

  float hue;
  if (maxVal == r) {
    hue = 60.0 * fmod(((g - b) / delta), 6.0);   // Red is strongest
  } else if (maxVal == g) {
    hue = 60.0 * (((b - r) / delta) + 2.0);      // Green is strongest
  } else {
    hue = 60.0 * (((r - g) / delta) + 4.0);      // Blue is strongest
  }

  if (hue < 0) hue += 360.0;             // Keep the angle between 0 and 360
  return hue;
}

// This function reads the sensor a few times and returns the average hue.
// Averaging makes the result more stable.
float readHue() {
  float total = 0;
  for (int i = 0; i < 5; i++) {          // Take 5 readings
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);      // Read red, green, blue, clear
    total += getHue(r, g, b);            // Add this hue to the total
    delay(50);
  }
  return total / 5.0;                    // Return the average
}

void setup() {
  Serial.begin(115200);                  // Start the Serial Monitor
  Wire.begin(21, 22);                    // I2C with SDA = 21, SCL = 22

  if (!tcs.begin()) {                    // Check if the sensor is connected
    Serial.println("Sensor not found! Check wiring.");
    while (1);                           // Stop here if sensor is missing
  }
  Serial.println("Sensor found!");

  Serial.println("Place the BLUE biuret reagent (without sample) in front of the sensor.");
  Serial.println("Calibrating for 10 seconds...");

  float total = 0;                       // Sum of hue readings
  int count = 0;                         // Number of readings
  unsigned long startTime = millis();    // Remember the start time

  while (millis() - startTime < 10000) { // Repeat for 10 seconds
    float hue = readHue();               // Get the current hue
    total += hue;
    count++;

    Serial.print("Calibrating... hue = ");
    Serial.println(hue);                 // Visual check: should stay steady
  }

  baselineHue = total / count;           // Average = hue of the biuret reagent
  Serial.print("Baseline hue (biuret reagent) = ");
  Serial.println(baselineHue);
  Serial.println("Calibration done. Now add your sample and monitor...");
}

void loop() {
  float hue = readHue();                         // Current hue
  float change = fabs(hue - baselineHue);        // How far the hue moved from the baseline

  Serial.print("Hue: ");
  Serial.print(hue);
  Serial.print("  |  Change: ");
  Serial.print(change);
  Serial.print(" deg  ->  ");

  if (change > hueThreshold) {                   // Color changed (blue -> violet)
    Serial.println("PROTEIN PRESENT (violet color)");
  } else {                                       // Still blue (brightness may differ)
    Serial.println("NO PROTEIN (blue color)");
  }

  delay(500);                                    // Wait before the next check
}