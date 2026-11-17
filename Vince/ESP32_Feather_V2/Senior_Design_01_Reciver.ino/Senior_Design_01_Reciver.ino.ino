#include <SPI.h>
#include <LoRa.h>

// Replace these with the receiver's actual GPIO pins
#define SCK_PIN  8
#define MISO_PIN 9
#define MOSI_PIN 10

#define CS_PIN   4
#define RST_PIN  3
#define DIO0_PIN 2

void setup() {
    Serial.begin(115200);

    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);

    LoRa.setPins(CS_PIN, RST_PIN, DIO0_PIN);

    if (!LoRa.begin(915E6)) {
        Serial.println("LoRa initialization failed!");
        while (true);
    }

    Serial.println("LoRa receiver initialized!");
    Serial.println("Waiting for packets...");
}

void loop() {
    int packetSize = LoRa.parsePacket();

    if (packetSize) {
        Serial.print("Received: ");

        while (LoRa.available()) {
            Serial.print((char)LoRa.read());
        }

        Serial.println();

        Serial.print("RSSI: ");
        Serial.print(LoRa.packetRssi());
        Serial.println(" dBm");

        Serial.print("SNR: ");
        Serial.print(LoRa.packetSnr());
        Serial.println(" dB");

        Serial.println("--------------------");
    }
}