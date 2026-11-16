#include <TinyGPSPlus.h>           // Library that turns GPS text into numbers

TinyGPSPlus gps;                   // Creates the decoder object named "gps"

#define GPS_RX 16                  // ESP32 pin that listens to the board's TX/SDO
#define GPS_TX 17                  // ESP32 pin that talks to the board's RX/SDI

unsigned long lastPrint = 0;       // Remembers when we last printed (in milliseconds)

// Converts a heading in degrees (0-360) into one of 8 compass names
String cardinal8(double deg) {
  const char* names[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};  // The 8 names in order
  int index = (int)((deg + 22.5) / 45.0) % 8;   // Each name covers 45 degrees
  return String(names[index]);
}

// Prints a number with a leading zero if it is below 10 (so 5 becomes 05)
void printTwoDigits(int number) {
  if (number < 10) Serial.print("0");
  Serial.print(number);
}

void setup() {
  Serial.begin(115200);            // Starts the Serial Monitor connection to your PC
  Serial2.begin(38400, SERIAL_8N1, GPS_RX, GPS_TX);  // GPS connection (38400 is the NEO-M9N default)
  Serial.println("NEO-M9N GPS started. Waiting for fix...");
}

void loop() {
  while (Serial2.available() > 0) {  // While the GPS has sent characters...
    gps.encode(Serial2.read());      // ...read one and give it to the decoder
  }

  if (millis() - lastPrint >= 1000) {  // Run this part once every second
    lastPrint = millis();              // Save the current time

    // Check 1: is any data arriving from the GPS at all?
    if (gps.charsProcessed() < 10) {
      Serial.println("No data from GPS. Check wiring (swap TX and RX).");
      return;                          // Stop here and try again next second
    }

    // Check 2: does the GPS have a position fix?
    if (!gps.location.isValid()) {
      Serial.print("Searching for satellites... Satellites: ");
      Serial.println(gps.satellites.value());
      return;                          // Stop here and try again next second
    }

    Serial.println("------------------------");

    // 1. Latitude (north-south position)
    Serial.print("Latitude:  ");
    Serial.println(gps.location.lat(), 6);        // 6 decimals is about 0.1 m

    // 2. Longitude (east-west position)
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);

    // 3. Elevation (height above sea level)
    Serial.print("Elevation: ");
    Serial.print(gps.altitude.meters());          // Altitude in meters
    Serial.println(" m");

    // 4. Speed (rate of movement)
    Serial.print("Speed:     ");
    Serial.print(gps.speed.kmph());               // Speed in km/h
    Serial.println(" km/h");

    // 5. Direction (compass heading and cardinal name)
    Serial.print("Direction: ");
    Serial.print(gps.course.deg());               // Heading in degrees (0 = North, 90 = East)
    Serial.print(" deg  ");
    Serial.println(cardinal8(gps.course.deg()));  // N, NE, E, SE, S, SW, W or NW

    // 6. Precise time in UTC (from the satellites' atomic clocks)
    Serial.print("Time UTC:  ");
    printTwoDigits(gps.time.hour());
    Serial.print(":");
    printTwoDigits(gps.time.minute());
    Serial.print(":");
    printTwoDigits(gps.time.second());
    Serial.println();

    // Same time converted to India time (UTC + 5:30)
    int totalMinutes = gps.time.hour() * 60 + gps.time.minute() + 330;  // Add 330 minutes
    int istHour = (totalMinutes / 60) % 24;       // Wrap around after 24 hours
    int istMinute = totalMinutes % 60;
    Serial.print("Time IST:  ");
    printTwoDigits(istHour);
    Serial.print(":");
    printTwoDigits(istMinute);
    Serial.print(":");
    printTwoDigits(gps.time.second());
    Serial.println();

    // Date (UTC)
    Serial.print("Date UTC:  ");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.println(gps.date.year());

    // Extra: number of satellites used
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
  }
}