#define VIBRATOR_PIN 2

enum AlertType {
  ALERT_NONE,
  ALERT_TEACHER,
  ALERT_BELL,
  ALERT_EMERGENCY
};

AlertType currentAlert = ALERT_NONE;
bool newAlertReceived = false;

void vibrate(int onTime, int offTime, int repetitions) {
  for(int i = 0; i < repetitions; i++) {
    digitalWrite(VIBRATOR_PIN, LOW);
    delay(onTime);
    digitalWrite(VIBRATOR_PIN, HIGH);
    if(i < repetitions - 1) {
      delay(offTime);
    }
  }
}

void playAlert(AlertType alert) {
    switch(alert) {
        case ALERT_TEACHER:
            vibrate(200, 200, 3);
            break;
        case ALERT_BELL:
            vibrate(300, 300, 5);
            break;
        case ALERT_EMERGENCY:
            vibrate(100, 100, 10);
            break;
    }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  pinMode(VIBRATOR_PIN, OUTPUT);
  digitalWrite(VIBRATOR_PIN, HIGH);
  Serial.println("Initialization...");
}

void loop() {
  if(Serial.available()) {
    char cmd = Serial.read();
    if(cmd == '1') {
      currentAlert = ALERT_TEACHER;
      newAlertReceived = true;
    }
    else if(cmd == '2') {
      currentAlert = ALERT_BELL;
      newAlertReceived = true;
    }
    else if(cmd == '3') {
      currentAlert = ALERT_EMERGENCY;
      newAlertReceived = true;
    }
  }

  if(newAlertReceived) {
    playAlert(currentAlert);
    newAlertReceived = false;
  }
}
