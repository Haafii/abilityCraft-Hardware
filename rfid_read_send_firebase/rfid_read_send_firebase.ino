#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22
#define SS_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define WIFI_SSID "Redmi Note 10S"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyD2I1CPsN6GTPvzSTxe7vAQgkAioqBbefQ"
#define DATABASE_URL "https://abilitycraft-d37cc-default-rtdb.asia-southeast1.firebasedatabase.app/"

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
String cardUID = "";
String prevCardUID = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign up OK");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("Scan PICC to see UID..."));
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && Firebase.ready() && signupOK) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); // Print full card details to Serial Monitor

      cardUID = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        cardUID += String(mfrc522.uid.uidByte[i], HEX);
      }

      if (cardUID != prevCardUID) {
        prevCardUID = cardUID;

        if (Firebase.RTDB.setString(&fbdo, "rfid/cardUID", cardUID)) {
          Serial.println("Card UID sent to Firebase");
          Serial.println("UID: " + cardUID);
        }
        else {
          Serial.println("Failed to send card UID to Firebase");
          Serial.println("Reason: " + fbdo.errorReason());
        }
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
  }
}
