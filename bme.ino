#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);  // SDA, SCL

  if (!bme.begin(0x76)) {
    Serial.println("BME680 not detected!");
    while (1) delay(1000);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
}

void loop() {
  if (bme.performReading()) {
    Serial.print("Temperature: ");
    Serial.print(bme.temperature);
    Serial.println(" °C");

    Serial.print("Pressure: ");
    Serial.print(bme.pressure / 100.0);
    Serial.println(" hPa");

    Serial.print("Humidity: ");
    Serial.print(bme.humidity);
    Serial.println(" %");

    Serial.println("----------------");
  } else {
    Serial.println("Sensor reading failed");
  }

  delay(2000);
}