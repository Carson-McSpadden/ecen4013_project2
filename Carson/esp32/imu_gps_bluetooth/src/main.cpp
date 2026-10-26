#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GPS.h>
#include <RH_RF95.h>

// Define Radio pins for RFM95/96 module (sharing SPI bus: 18, 19, 23)
#define RFM95_CS 33
#define RFM95_RST 32
#define RFM95_INT 27
#define RF95_FREQ 915.0 // Match your module frequency (e.g., 915.0 or 433.0)

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Define GPS over I2C (Wire / STEMMA QT)
Adafruit_GPS GPS(&Wire);

// Define IMU over I2C
Adafruit_ICM20948 imu;

// Update interval (set to 1000ms = 1 Hz for stable testing)
const unsigned long REPORT_INTERVAL_MS = 1000;
unsigned long lastReportTime = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Wire.begin();

    // 1. Initialize GPS
    if (!GPS.begin(0x10))
    {
        Serial.println("ERROR: Failed to find GPS module over I2C!");
        while (1)
        {
            delay(10);
        }
    }
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

    // 2. Initialize IMU
    if (!imu.begin_I2C())
    {
        Serial.println("ERROR: Failed to find ICM20948 chip over I2C!");
        while (1)
        {
            delay(10);
        }
    }
    imu.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
    imu.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);

    // 3. Initialize Radio Transmitter
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    delay(100);
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!rf95.init())
    {
        Serial.println("ERROR: LoRa radio init failed!");
        while (1)
            ;
    }
    if (!rf95.setFrequency(RF95_FREQ))
    {
        Serial.println("ERROR: setFrequency failed!");
        while (1)
            ;
    }
    rf95.setTxPower(23, false); // Max power output

    // Print CSV Header to USB Serial stream
    Serial.println("Date,Time,Satellites,Latitude,Longitude,Elevation_MSL_m,Accel_X_mss,Accel_Y_mss,Accel_Z_mss,Gyro_X_rads,Gyro_Y_rads,Gyro_Z_rads,Mag_X_uT,Mag_Y_uT,Mag_Z_uT");
}

void loop()
{
    // Continuously read GPS sentence bytes
    char c = GPS.read();
    if (GPS.newNMEAreceived())
    {
        GPS.parse(GPS.lastNMEA());
    }

    unsigned long currentTime = millis();

    if (currentTime - lastReportTime >= REPORT_INTERVAL_MS)
    {
        lastReportTime = currentTime;

        // Gather IMU data events
        sensors_event_t accel, gyro, mag, temp;
        imu.getEvent(&accel, &gyro, &temp, &mag);

        float gyroX_rads = gyro.gyro.x * (PI / 180.0);
        float gyroY_rads = gyro.gyro.y * (PI / 180.0);
        float gyroZ_rads = gyro.gyro.z * (PI / 180.0);

        // Gather GPS values
        int satellites = (int)GPS.satellites;
        float latitude = GPS.latitudeDegrees;
        float longitude = GPS.longitudeDegrees;
        float elevation = GPS.altitude;

        String dateStr = String(GPS.month) + "/" + String(GPS.day) + "/" + String(2000 + GPS.year);
        String timeStr = String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds);

        // Build the standard CSV row packet matching project formatting requirements
        String csvRow = dateStr + "," +
                        timeStr + "," +
                        String(satellites) + "," +
                        String(latitude, 6) + "," +
                        String(longitude, 6) + "," +
                        String(elevation, 2) + "," +
                        String(accel.acceleration.x, 4) + "," +
                        String(accel.acceleration.y, 4) + "," +
                        String(accel.acceleration.z, 4) + "," +
                        String(gyroX_rads, 4) + "," +
                        String(gyroY_rads, 4) + "," +
                        String(gyroZ_rads, 4) + "," +
                        String(mag.magnetic.x, 2) + "," +
                        String(mag.magnetic.y, 2) + "," +
                        String(mag.magnetic.z, 2);

        // Output 1: Live USB Stream to Serial Monitor (feeds GUI later)
        Serial.println(csvRow);

        // Output 2: Live UHF Radio Broadcast Stream
        rf95.send((uint8_t *)csvRow.c_str(), csvRow.length());
        rf95.waitPacketSent();
    }
}