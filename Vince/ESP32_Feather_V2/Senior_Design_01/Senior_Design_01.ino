#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <LoRa.h>
#include <Adafruit_GPS.h>

Adafruit_ICM20948 icm;
File dataFile;

// Adafruit_GPS GPS(&Wire); // Use this For SPI

// Use this for UART 
#define GPS_RX_PIN 7
#define GPS_TX_PIN 8
// Define GPS over Hardware Serial1 (TX -> RX Pin 7, RX -> TX Pin 8)
Adafruit_GPS GPS(&Serial1);


// I2C Pins
#define SDA_PIN 22
#define SCL_PIN 20

// Shared SPI Pins
#define MISO_PIN 21
#define MOSI_PIN 19
#define SCK_PIN 5


// SD Pins
#define CS1_PIN 33
// #define CD_PIN 15

// RF Pins
#define CS2_PIN 27
#define RD_RST_PIN 32
#define RD_INT_PIN 14


// Timing Variables
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 50;

// Global variable that will hold session file name
String currentLogFile = "";

void setup() {
  Serial.begin(115200);

  // You need to wait for serial monitor if connected
  while(!Serial && millis() < 3000); 

  // Configure CS pins
  pinMode(CS1_PIN, OUTPUT);
  pinMode(CS2_PIN, OUTPUT);

  digitalWrite(CS1_PIN, HIGH);
  digitalWrite(CS2_PIN, HIGH);

  // Configure SD card detection
  // pinMode(CD_PIN, INPUT_PULLUP);

  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize SPI
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);

  // Initialize Peripherals
  ICM();
  SD_Card();
  RF_Module();
  GPS_Init();

}


void ICM() {
  if (!icm.begin_I2C(0x68, &Wire)) {
    dataError("ICM initialization failed!\n");
  }

  else {
    Serial.print("ICM initialized!\n");
  }

}

void RF_Module() {
  // Configure custom pins for LoRa library
  LoRa.setPins(CS2_PIN, RD_RST_PIN, RD_INT_PIN);
  
  if (!LoRa.begin(915E6)) { // Frequency in Hz (915 MHz)
    dataError("RF module initialization failed!\n");
  } 
  else {
    Serial.println("RF module initialized!\n");
    LoRa.setTxPower(13); // Set power in dBm
  }
}

void SD_Card() {
  // Let SD.begin() handle detection and initialization natively
  if (!SD.begin(CS1_PIN, SPI, 4000000)) {
    dataError("SD card initialization failed / not detected!\n");
  } else {
    Serial.print("SD card initialized!\n");
  }
}

void GPS_Init(){
  // For UART
  // Start Serial2 at 9600 baud using your chosen RX/TX pins
  Serial2.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  if (!GPS.begin(9600)) {
    dataError("GPS initialization failed!\n");
  } else {
    Serial.print("GPS initialized via UART!\n");
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  }

  // For SPI
  // PA1010D default I2C address is 0x10
  // if (!GPS.begin(0x10)) {
  //   dataError("GPS initialization failed!\n");
  // } 
  // else {
  //   Serial.print("GPS initialized!\n");
  //   // Turn on RMC (recommended minimum) and GGA (fix data) sentences
  //   GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  //   // Set update rate to 1 Hz
  //   GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  // }
}

void GPS_Update() {
    GPS.read();

    if (GPS.newNMEAreceived()) {
        if (!GPS.parse(GPS.lastNMEA())) {
            Serial.println("GPS sentence parsing failed!");
        }
    }
}



// Error handler
void dataError(String errorMessage){
  Serial.print(errorMessage);
  while(true){
  }
}


void loop() {
    GPS_Update();
    static unsigned long lastGPSPrint = 0;

if (millis() - lastGPSPrint >= 1000) {
    lastGPSPrint = millis();

    Serial.print("GPS Fix: ");
    Serial.println(GPS.fix);

    Serial.print("Satellites: ");
    Serial.println((int)GPS.satellites);

    Serial.print("Latitude: ");
    Serial.println(GPS.latitudeDegrees, 6);

    Serial.print("Longitude: ");
    Serial.println(GPS.longitudeDegrees, 6);

    Serial.print("Altitude: ");
    Serial.println(GPS.altitude, 2);

    Serial.println("----------------");
}

  unsigned long currentMillis = millis();

  // sample at the defined interval
  if(currentMillis - lastSampleTime >= sampleInterval){
    lastSampleTime = currentMillis;

    // get sensor events (Accel, Gyro, TimeStamp, Mag)
    sensors_event_t accel, gyro, temp, mag;
    icm.getEvent(&accel, &gyro, &temp, &mag);

    // Build GPS data strings conditionally based on whether we have a fix
    String latStr = "0.0";
    String lonStr = "0.0";
    String altStr = "0.0";
    String satStr = "0";

    if (GPS.fix) {
      latStr = String(GPS.latitudeDegrees, 6) + String(GPS.lat);
      lonStr = String(GPS.longitudeDegrees, 6) + String(GPS.lon);
      altStr = String(GPS.altitude);
      satStr = String((int)GPS.satellites);
    }

    // CSV file formatting
    String dataString = String(currentMillis) + "," +
                        String(accel.acceleration.x, 3) + "," +
                        String(accel.acceleration.y, 3) + "," +
                        String(accel.acceleration.z, 3) + "," + 
                        String(gyro.gyro.x, 3) + "," +
                        String(gyro.gyro.y, 3) + "," +
                        String(gyro.gyro.z, 3) + "," +
                        latStr + "," +
                        lonStr + "," +
                        altStr + "," +
                        satStr;

    // Log into SD card
    // if(digitalRead(CD_PIN) == LOW){
    //   dataFile = SD.open("/datalog.csv", FILE_WRITE);
    //   if(dataFile){
    //     dataFile.println(dataString);
    //     dataFile.close();
    //   }
    //   else{
    //     Serial.println("Error opening datalog.csv for writing");
    //   }
    // }

    // Append to the single log file
    dataFile = SD.open("/datalog.csv", FILE_APPEND);
    if(dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    } 
    else {
      Serial.println("Error opening datalog.csv for writing!");
    }

    // Radio Transmission
    LoRa.beginPacket();
      LoRa.print(dataString);
      LoRa.endPacket();

    // Show in serial monitor
    Serial.println(dataString);
  }
}
