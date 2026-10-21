#include <Arduino.h>

// LED_BUILTIN automatically maps to GPIO 25 on the standard Pico
// and the wireless module pin on the Pico W.
const int ledPin = LED_BUILTIN;

void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);

    while (!Serial && millis() < 3000)
    {
        // Wait for serial monitor to open (USB CDC)
    }

    Serial.println("========================================");
    Serial.println("Raspberry Pi Pico Hardware Sanity Test Initialized");
    Serial.println("========================================");
}

void loop()
{
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ON - Raspberry Pi Pico Running");
    delay(1000);

digitalBoard:
    digitalWrite(ledPin, LOW);
    Serial.println("LED OFF - Raspberry Pi Pico Running");
    delay(1000);
}