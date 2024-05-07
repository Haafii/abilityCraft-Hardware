#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         22          // Configurable, see typical pin layout above
#define SS_PIN          5          // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssid = "abilityCraft";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient* CLIENTS[10];
int num_of_clients = 0;

void Broadcast(String data) {
  for (int i = 0; i < num_of_clients; i++)
    CLIENTS[i]->text(data);
}

void WebsocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Websocket client connection received");
    client->text("{\"status\":\"connected\"}");
    CLIENTS[num_of_clients++] = client;
  } else if (type == WS_EVT_DISCONNECT) {
    client->text("{\"status\":\"disconnected\"}");
    // Serial.println("Client disconnected");
  } else if (type == WS_EVT_DATA) {
    // Handle data received from a client
    String receivedData = String((char*)data);
    Serial.println("Data received from client: " + receivedData);
  }
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);  // will pause Zero, Leonardo, etc until serial console opens

  // Set up Wi-Fi access point
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  // Set up WebSocket server
  ws.onEvent(WebsocketEvent);
  server.addHandler(&ws);
  server.begin();

  SPI.begin();       // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);         // Optional delay. Some board do need more time after init to be ready, see Readme
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

  // Send the card UID to WebSocket clients
  Broadcast(cardUID);

  // Dump debug info about the card; PICC_HaltA() is automatically called
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  delay(1000);  // Add a delay to avoid rapid readings from the same card
}

