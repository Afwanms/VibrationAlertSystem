#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Assign a unique ID to this specific student device
const int MY_STUDENT_ID = 1; 

// Pin connected to the vibration motor or testing LED
const int MOTOR_PIN = 13; 

// Structure must exactly match the Master Node's layout
typedef struct struct_message {
    char alertType; // 'J' = Jadwal, 'E' = Emergency, 'C' = Call
    int studentID;  // 0 = Broadcast, or specific ID
} struct_message;

struct_message incomingData;

// Function to handle custom vibration rhythms
void vibrate(int pulses, int durationOn, int durationOff) {
  for (int i = 0; i < pulses; i++) {
    digitalWrite(MOTOR_PIN, HIGH);
    delay(durationOn);
    digitalWrite(MOTOR_PIN, LOW);
    delay(durationOff);
  }
}

// Callback function executed when data arrives (ESP32 structure signature)
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataRaw, int len) {
  memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));
  
  Serial.print("Packet Received! Type: "); Serial.print(incomingData.alertType);
  Serial.print(" | Target Student ID: "); Serial.println(incomingData.studentID);

  // Filter: Process if it's a global broadcast (0) OR targeted at this student
  if (incomingData.studentID == 0 || incomingData.studentID == MY_STUDENT_ID) {
    
    // Pattern 1: Alert Jadwal / Bell -> 2 Quick Taps
    if (incomingData.alertType == 'J') {
      Serial.println("Action: Triggering Bell Vibe...");
      vibrate(2, 200, 200); 
    }
    
    // Pattern 2: Personal Teacher Call -> 1 Long Pulse
    else if (incomingData.alertType == 'C') {
      Serial.println("Action: Triggering Teacher Call Vibe...");
      vibrate(1, 1000, 0); 
    }
    
    // Pattern 3: Emergency -> Continuous Rapid Pulses
    else if (incomingData.alertType == 'E') {
      Serial.println("Action: Triggering EMERGENCY Vibe...");
      vibrate(10, 100, 100); 
    }
  } else {
    Serial.println("Data ignored: Packet meant for a different student.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW); // Keep off at boot

  // Put ESP32 into Wi-Fi Station Mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Initialize ESP-NOW on the ESP32 architecture
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register the incoming message event handler
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("ESP32 Receiver Online. Ready and listening...");
}

void loop() {
  // Free execution path; processing is handled strictly via core interruptions
  delay(10);
}