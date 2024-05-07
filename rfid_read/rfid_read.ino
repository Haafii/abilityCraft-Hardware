#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   22          // Reset pin connected to RST on MFRC522
#define SS_PIN    5           // Slave select pin connected to SDA on MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);  // Wait for serial monitor to open

  SPI.begin();       // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);         // Optional delay. Some board may need more time after init to be ready
  mfrc522.PCD_DumpVersionToSerial();   // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Get the UID of the card
  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }

  Serial.println("Card UID: " + cardUID);

  // Halt PICC to stop communication
  mfrc522.PICC_HaltA();

  // Stop encryption on PCD (RFID reader)
  mfrc522.PCD_StopCrypto1();

  delay(1000);  // Add a delay to avoid rapid readings from the same card
}
