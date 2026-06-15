#include <ESP8266WiFi.h>
#include <espnow.h>
#define STUDENT_ID 1
#define VIBRATOR_PIN 16

enum AlertType {
  ALERT_NONE,
  ALERT_TEACHER,
  ALERT_BELL,
  ALERT_EMERGENCY
};

struct AlertMessage {
  uint8_t targetID;
  uint8_t alertType;
};

AlertType currentAlert = ALERT_NONE;
bool newAlertReceived = false;

void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  AlertMessage msg;
  memcpy(&msg, incomingData, sizeof(msg));
  Serial.println("Packet received");
  Serial.print("Target: ");
  Serial.println(msg.targetID);
  Serial.print("Alert: ");
  Serial.println(msg.alertType);

  if(msg.targetID == STUDENT_ID ||
     msg.targetID == 0)
  {
      currentAlert = (AlertType)msg.alertType;
      newAlertReceived = true;

      Serial.println("Alert accepted");
  }
}

void vibrate(int onTime, int offTime, int repetitions) {
  for(int i = 0; i < repetitions; i++) {
    digitalWrite(VIBRATOR_PIN, HIGH);
    delay(onTime);
    digitalWrite(VIBRATOR_PIN, LOW);
    delay(offTime);
  }
}

void playAlert(AlertType alert) {
    switch(alert) {
        case ALERT_TEACHER:
            vibrate(1500, 250, 3);
            break;
        case ALERT_BELL:
            vibrate(2000, 500, 5);
            break;
        case ALERT_EMERGENCY:
            vibrate(1000, 100, 10);
            break;
    }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  pinMode(VIBRATOR_PIN, OUTPUT);
  digitalWrite(VIBRATOR_PIN, LOW);
  Serial.println("Initialization...");
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("ESP-NOW Ready");
}

void loop() {
  if(newAlertReceived) {
    playAlert(currentAlert);
    newAlertReceived = false;
  }
}
