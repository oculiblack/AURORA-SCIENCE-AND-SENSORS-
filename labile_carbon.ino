#include <Wire.h>                 // I2C communication library
#include <Adafruit_TCS34725.h>    // Library for the color sensor

// Create the sensor object: 154 ms reading time, 4x gain (sensitivity)
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_4X);

float baselinePurple = 0;         // Purple intensity measured in the first 10 seconds

// This function turns R, G, B values into one "purple intensity" number.
// Purple = lots of red + lots of blue, but little green.
float getPurpleIntensity(uint16_t r, uint16_t g, uint16_t b) {
  float purple = ((r + b) / 2.0) - g;   // average of red and blue, minus green
  if (purple < 0) purple = 0;           // no negative values
  return purple;
}

void setup() {
  Serial.begin(115200);           // Start the Serial Monitor
  Wire.begin(21, 22);             // Start I2C with SDA = 21, SCL = 22

  if (!tcs.begin()) {             // Check if the sensor is connected
    Serial.println("Sensor not found! Check wiring.");
    while (1);                    // Stop here forever if sensor is missing
  }
  Serial.println("Sensor found!");

  Serial.println("Show the PURPLE color to the sensor now...");
  Serial.println("Calibrating for 10 seconds...");

  float total = 0;                // Sum of all purple readings
  int count = 0;                  // How many readings we took
  unsigned long startTime = millis();   // Remember when we started

  while (millis() - startTime < 10000) {   // Repeat for 10 seconds
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);        // Read red, green, blue, clear
    float purple = getPurpleIntensity(r, g, b);

    total += purple;              // Add to the sum
    count++;                      // Count this reading

    Serial.print("Calibrating... purple = ");
    Serial.println(purple);       // Visual check that readings look steady
    delay(200);
  }

  baselinePurple = total / count; // Average = our reference purple level
  Serial.print("Baseline purple intensity = ");
  Serial.println(baselinePurple);
  Serial.println("Calibration done. Now monitoring...");
}

void loop() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);            // Read the sensor
  float purple = getPurpleIntensity(r, g, b);

  float decrease = 0;                        // Percentage decrease
  if (baselinePurple > 0) {
    decrease = ((baselinePurple - purple) / baselinePurple) * 100.0;
  }
  if (decrease < 0) decrease = 0;            // If it got stronger, show 0%

  Serial.print("Purple: ");
  Serial.print(purple);
  Serial.print("  |  Decrease: ");
  Serial.print(decrease);
  Serial.println(" %");

  delay(300);                                // Wait a bit before the next reading
}