#include "ble_handler.h"

#ifdef ESP32

#include <NimBLEDevice.h>
#include "Buttons.h"
#include "config.h"

#define SERVICE_UUID "8B3D0001-57B4-4DFE-8A3E-2F0D5A5B8C01"
#define CHARACTERISTIC_UUID "8B3D0002-57B4-4DFE-8A3E-2F0D5A5B8C01"

static NimBLEServer *pServer = nullptr;
static NimBLECharacteristic *pCharacteristic = nullptr;
static bool deviceConnected = false;

class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer)
    {
        deviceConnected = true;
#ifdef DEBUG_BUTTONS
        Serial.println("[BLE] Client connected");
#endif
    }

    void onDisconnect(NimBLEServer *pServer)
    {
        deviceConnected = false;
        Buttons.setBleMask(0);
#ifdef DEBUG_BUTTONS
        Serial.println("[BLE] Client disconnected - cleared button mask");
#endif
        NimBLEDevice::startAdvertising();
#ifdef DEBUG_BUTTONS
        Serial.println("[BLE] Restarted advertising");
#endif
    }
};

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0)
        {
            uint8_t buttonMask = 0;
            char firstChar = value[0];
            
            // Support ASCII commands: 'l', 'o', 'r' or bitmask
            if (firstChar == 'l' || firstChar == 'L') {
                buttonMask = 0x01; // LEFT
            } else if (firstChar == 'o' || firstChar == 'O') {
                buttonMask = 0x02; // OK
            } else if (firstChar == 'r' || firstChar == 'R') {
                buttonMask = 0x04; // RIGHT
            } else {
                buttonMask = (uint8_t)firstChar; // Raw bitmask
            }
            
            Buttons.setBleMask(buttonMask);
#ifdef DEBUG_BUTTONS
            Serial.print("[BLE] Received: '");
            if (firstChar >= 32 && firstChar < 127) {
                Serial.print(firstChar);
            } else {
                Serial.print("0x");
                Serial.print((uint8_t)firstChar, HEX);
            }
            Serial.print("' -> mask: 0x");
            Serial.print(buttonMask, HEX);
            Serial.print(" (LEFT=");
            Serial.print(buttonMask & 0x01 ? "1" : "0");
            Serial.print(", OK=");
            Serial.print(buttonMask & 0x02 ? "1" : "0");
            Serial.print(", RIGHT=");
            Serial.print(buttonMask & 0x04 ? "1" : "0");
            Serial.println(")");
#endif
        }
    }
};

void initBLE()
{
#ifdef DEBUG_BUTTONS
    Serial.println("[BLE] Initializing BLE server...");
#endif

    NimBLEDevice::init("AquariumPet");

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);

    pCharacteristic->setCallbacks(new CharacteristicCallbacks());

    pService->start();

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    NimBLEDevice::startAdvertising();

#ifdef DEBUG_BUTTONS
    Serial.println("[BLE] BLE server started and advertising");
    Serial.println("[BLE] Device name: AquariumPet");
    Serial.println("[BLE] Service UUID: " SERVICE_UUID);
    Serial.println("[BLE] Characteristic UUID: " CHARACTERISTIC_UUID);
#endif
}

void updateBLE()
{
}

#endif // ESP32
