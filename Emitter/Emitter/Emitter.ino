#include <Wire.h>
#include <MFRC522_I2C.h>
#include <esp_now.h>
#include <WiFi.h>

// -------- Pins --------
#define LASER_PIN   25
#define BUZZER_PIN  26
#define SDA_PIN     21
#define SCL_PIN     22

// -------- RFID --------
MFRC522_I2C mfrc522(0x28, -1);

// -------- ESP-NOW --------
uint8_t receiverMAC[] = {0x20, 0xE7, 0xC8, 0xF6, 0xE2, 0xD4};

typedef struct {
  char command[12];
} Message;

Message outMsg;

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  Message incoming;
  memcpy(&incoming, data, sizeof(incoming));

  if (strcmp(incoming.command, "ALARM") == 0) {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  if (strcmp(incoming.command, "DISARMED") == 0) {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void sendCommand(const char* cmd) {
  strncpy(outMsg.command, cmd, sizeof(outMsg.command));
  esp_now_send(receiverMAC, (uint8_t *)&outMsg, sizeof(outMsg));
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); return; }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

bool armed = false;

bool rfidScanned() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial())  return false;
  mfrc522.PICC_HaltA();
  return true;
}

void arm() {
  armed = true;
  digitalWrite(LASER_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  sendCommand("ARMED");
  Serial.println("System ARMED");
}

void disarm() {
  armed = false;
  digitalWrite(LASER_PIN,  LOW);
  digitalWrite(BUZZER_PIN, LOW);
  sendCommand("DISARMED");
  Serial.println("System DISARMED — idling");
}

void setup() {
  Serial.begin(115200);
  pinMode(LASER_PIN,  OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LASER_PIN,  LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  mfrc522.PCD_Init();
  Serial.println("Scan card to arm...");
  initESPNow();
}

void loop() {
  if (!rfidScanned()) {
    delay(100);
    return;
  }
  delay(200); // debounce
  if (!armed) arm();
  else        disarm();
  delay(1000); // prevent double scan
}