#include <BluetoothSerial.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         22
#define SS_PIN          5
MFRC522 mfrc522(SS_PIN, RST_PIN);

BluetoothSerial SerialBT;

void Broadcast(String data) {
  SerialBT.print(data);
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  SerialBT.begin("abilityCraft"); // Bluetooth device name
  Serial.println("Bluetooth initialized");

  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }

  Broadcast(cardUID);

  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  delay(1000);
}
