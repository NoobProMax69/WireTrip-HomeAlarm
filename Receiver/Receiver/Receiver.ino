#include <esp_now.h>
#include <WiFi.h>

// -------- Pins --------
#define LDR_PIN    34
#define BUZZER_PIN 26

int threshold = 2000;

// -------- ESP-NOW --------
uint8_t emitterMAC[] = {0x68, 0xFE, 0x71, 0x86, 0xB0, 0x50};

typedef struct {
  char command[12];
} Message;

Message outMsg;

bool armed    = false;
bool alarming = false;

void sendCommand(const char* cmd) {
  strncpy(outMsg.command, cmd, sizeof(outMsg.command));
  esp_now_send(emitterMAC, (uint8_t *)&outMsg, sizeof(outMsg));
}

// Updated signature for ESP32 core 3.x
void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// Updated signature for ESP32 core 3.x
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  Message incoming;
  memcpy(&incoming, data, sizeof(incoming));

  if (strcmp(incoming.command, "ARMED") == 0) {
    armed    = true;
    alarming = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("System ARMED — monitoring beam");
  }
  if (strcmp(incoming.command, "DISARMED") == 0) {
    armed    = false;
    alarming = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("System DISARMED — idling");
  }
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); return; }
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, emitterMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void setup() {
  Serial.begin(115200);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  initESPNow();
  Serial.println("Receiver ready — waiting for arm signal");
}

void loop() {
  if (!armed) return;

  int lightValue = analogRead(LDR_PIN);
  Serial.print("Light value: ");
  Serial.println(lightValue);

  if (lightValue < threshold && !alarming) {
    alarming = true;
    digitalWrite(BUZZER_PIN, HIGH);
    sendCommand("ALARM");
    Serial.println("BEAM BROKEN — alarm triggered");
  }

  delay(100);
}