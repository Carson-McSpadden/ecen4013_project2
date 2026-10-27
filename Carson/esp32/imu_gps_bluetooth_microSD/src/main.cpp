#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GPS.h>
#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please enable it in your configuration.
#endif

// Create Bluetooth Serial object (Laptop will see this as "ECEN4013_Tracker")
BluetoothSerial SerialBT;

// Define MicroSD Chip Select pin (Assigned to Pin 15)
#define SD_CS 15

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
    // Removed blocking while(!Serial) so the board boots immediately on power/battery

    Wire.begin();
    Wire.setClock(100000); // Set stable I2C clock speed (100 kHz) to prevent bus timeouts

    // 1. Initialize GPS
    if (!GPS.begin(0x10))
    {
        Serial.println("ERROR: Failed to find GPS module over I2C!");
    }
    else
    {
        Serial.println("GPS initialized.");
    }
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

    // 2. Initialize IMU
    if (!imu.begin_I2C())
    {
        Serial.println("ERROR: Failed to find ICM20948 chip over I2C!");
    }
    else
    {
        Serial.println("IMU initialized.");
    }
    imu.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
    imu.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);

    // 3. Initialize MicroSD Card (CS on Pin 15)
    Serial.print("Initializing SD card...");
    if (!SD.begin(SD_CS))
    {
        Serial.println("ERROR: SD card initialization failed!");
    }
    else
    {
        Serial.println("SD card initialized successfully.");
        // Create file and add header if it doesn't already exist
        File dataFile = SD.open("/telemetry.csv", FILE_WRITE);
        if (dataFile && dataFile.size() == 0)
        {
            dataFile.println("Date,Time,Satellites,Latitude,Longitude,Elevation_MSL_m,Accel_X_mss,Accel_Y_mss,Accel_Z_mss,Gyro_X_rads,Gyro_Y_rads,Gyro_Z_rads,Mag_X_uT,Mag_Y_uT,Mag_Z_uT");
            dataFile.close();
        }
    }

    // 4. Initialize Bluetooth Serial
    SerialBT.begin("ECEN4013_Tracker");
    Serial.println("Bluetooth device active, waiting for laptop pairing...");

    // Print CSV Header to USB Serial stream
    Serial.println("Date,Time,Satellites,Latitude,Longitude,Elevation_MSL_m,Accel_X_mss,Accel_Y_mss,Accel_Z_mss,Gyro_X_rads,Gyro_Y_rads,Gyro_Z_rads,Mag_X_uT,Mag_Y_uT,Mag_Z_uT");
}

void loop()
{
    // Allow FreeRTOS and background Bluetooth stack breathing room
    yield();

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
    }
}