#include <Arduino.h>

void setup()
{
    // Initialize serial communication at 115200 baud
    Serial.begin(115200);
    delay(1000); // Allow time for serial monitor to initialize

    // Configure the built-in LED pin as an output
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("ESP32 Test Script Initialized.");
}

void loop()
{
    // Turn LED on
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("LED state: HIGH");
    delay(1000);

    // Turn LED off
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("LED state: LOW");
    delay(1000);
}