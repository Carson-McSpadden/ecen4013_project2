#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>

Adafruit_ICM20948 imu;

const unsigned long REPORT_INTERVAL_MS = 20;
unsigned long lastReportTime = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Wire.begin();

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

    // Print CSV Header
    Serial.println("Timestamp_ms,Accel_X_mss,Accel_Y_mss,Accel_Z_mss,Gyro_X_rads,Gyro_Y_rads,Gyro_Z_rads,Mag_X_uT,Mag_Y_uT,Mag_Z_uT");
}

void loop()
{
    unsigned long currentTime = millis();

    if (currentTime - lastReportTime >= REPORT_INTERVAL_MS)
    {
        lastReportTime = currentTime;

        // Declare individual raw/event variables
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t mag;
        sensors_event_t temp;

        // Populate events from the sensor library
        imu.getEvent(&accel, &gyro, &temp, &mag);

        // Convert gyro values from degrees/sec to radians/sec
        float gyroX_rads = gyro.gyro.x * (PI / 180.0);
        float gyroY_rads = gyro.gyro.y * (PI / 180.0);
        float gyroZ_rads = gyro.gyro.z * (PI / 180.0);

        // Stream formatted CSV line over USB
        Serial.print(currentTime);
        Serial.print(",");
        Serial.print(accel.acceleration.x, 4);
        Serial.print(",");
        Serial.print(accel.acceleration.y, 4);
        Serial.print(",");
        Serial.print(accel.acceleration.z, 4);
        Serial.print(",");
        Serial.print(gyroX_rads, 4);
        Serial.print(",");
        Serial.print(gyroY_rads, 4);
        Serial.print(",");
        Serial.print(gyroZ_rads, 4);
        Serial.print(",");
        Serial.print(mag.magnetic.x, 2);
        Serial.print(",");
        Serial.print(mag.magnetic.y, 2);
        Serial.print(",");
        Serial.println(mag.magnetic.z, 2);
    }
}