#include <Arduino.h>
#include <Wire.h>

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Initialize I2C on default pins (GPIO 21 for SDA, GPIO 22 for SCL)
    Wire.begin();

    Serial.println("\nI2C Scanner Initialized.");
}

void loop()
{
    byte error, address;
    int nDevices = 0;

    Serial.println("Scanning for I2C devices...");

    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmission to see if
        // a device acknowledged the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println("  !");
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }

    if (nDevices == 0)
    {
        Serial.println("No I2C devices found. Check your wiring!");
    }
    else
    {
        Serial.println("Scan complete.\n");
    }

    delay(5000); // Wait 5 seconds before the next scan
}