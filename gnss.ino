#include <TinyGPSPlus.h>           // Library that turns GPS text into numbers

TinyGPSPlus gps;                   // Creates the decoder object named "gps"

#define GPS_RX 16                  // ESP32 pin that listens to the board's TX/SDO
#define GPS_TX 17                  // ESP32 pin that talks to the board's RX/SDI

unsigned long lastPrint = 0;       // Remembers when we last printed (in milliseconds)

void setup() {
  Serial.begin(115200);            // Starts the Serial Monitor connection to your PC
  Serial2.begin(38400, SERIAL_8N1, GPS_RX, GPS_TX);  // Starts the GPS connection (38400 is the NEO-M9N default)
  Serial.println("Waiting for GPS...");              // Shows that the program has started
}

void loop() {
  while (Serial2.available() > 0) {  // While the GPS has sent characters...
    gps.encode(Serial2.read());      // ...read one and give it to the decoder
  }

  if (millis() - lastPrint >= 1000) {  // Run this part once every second
    lastPrint = millis();              // Save the current time

    if (gps.charsProcessed() < 10) {   // If almost nothing was received from the GPS...
      Serial.println("No data from GPS. Check wiring (TX/RX swapped?).");
    }
    else if (gps.location.isValid()) { // If the GPS has a real position...
      Serial.print("Latitude:   ");
      Serial.println(gps.location.lat(), 6);   // Latitude, 6 decimal places
      Serial.print("Longitude:  ");
      Serial.println(gps.location.lng(), 6);   // Longitude, 6 decimal places
      Serial.print("Satellites: ");
      Serial.println(gps.satellites.value());  // Number of satellites used
      Serial.println("-------------------");
    }
    else {                             // Data is arriving but there is no fix yet
      Serial.print("Searching for satellites... Satellites: ");
      Serial.println(gps.satellites.value());
    }
  }
}