#include <SPI.h>
#include <MFRC522.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define RST_PIN         22
#define SS_PIN          5
#define SERVICE_UUID        "0000180D-0000-1000-8000-00805F9B34FB"  // Example UUID for a standard BLE service
#define CHARACTERISTIC_UUID "00002A37-0000-1000-8000-00805F9B34FB"  // Example UUID for a standard BLE characteristic


MFRC522 mfrc522(SS_PIN, RST_PIN);

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  
  BLEDevice::init("ESP32-RFID");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  if (!deviceConnected) {
    delay(500);
    return;
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  String cardData = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardData += String(mfrc522.uid.uidByte[i], HEX);
  }
  
  pCharacteristic->setValue(cardData.c_str());
  pCharacteristic->notify();
  delay(1000);
}
