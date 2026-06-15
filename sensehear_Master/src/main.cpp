#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include "SH1106Wire.h"
#include <Keypad.h>

// --- OLED Display Configuration ---
SH1106Wire display(0x3c, D2, D1); 

// --- 4x4 Keypad Configuration ---
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'}, // A = Alert Jadwal (Bell)
  {'4','5','6','B'}, // B = Emergency
  {'7','8','9','C'}, // C = Call Student Mode
  {'*','0','#','D'}  // # = Confirm Call, * = Clear Input
};

byte rowPins[ROWS] = {D0, D3, D4, D5}; 
byte colPins[COLS] = {D6, D7, D8, D10}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- ESP-NOW Configuration ---
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
    char alertType; // 'J' = Jadwal, 'E' = Emergency, 'C' = Call
    int studentID;  // 0 = Broadcast to all, otherwise specific target ID
} struct_message;

struct_message myData;

// Global State Variables
String inputBuffer = "";
bool callMode = false;

// --- Function Prototypes ---
void updateHomeScreen();
void debugAndSendData();
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);

// --- UI Display Rendering Functions ---
void updateHomeScreen() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT); 
  
  display.drawString(0, 0, "MASTER TRANSMITTER");
  display.drawString(0, 12, "---------------------");
  display.drawString(0, 24, "A: Alert Jadwal (Bell)");
  display.drawString(0, 36, "B: Emergency Alert");
  display.drawString(0, 48, "C: Call Student");
  display.display();
  
  Serial.println(F("\n--- [SYSTEM READY] ---"));
  Serial.println(F("Enter commands via Keypad or Serial Monitor:"));
  Serial.println(F("Type 'A' for Bell, 'B' for Emergency, 'C' to Call a student."));
}

void debugAndSendData() {
  Serial.println(F("\n============ ESP-NOW PACKET SENT ============"));
  Serial.print(F("Alert Type Payload: ")); Serial.println(myData.alertType);
  Serial.print(F("Target Student ID : ")); Serial.println(myData.studentID);
  Serial.println(F("============================================="));
  
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  if (sendStatus == 0) {
    display.drawString(0, 20, "Sent Successfully!");
    Serial.println(F("--> ESP-NOW Hardware Layer Confirmed: Packet dispatched."));
  } else {
    display.drawString(0, 20, "Delivery Failed.");
    Serial.println(F("--> ESP-NOW Hardware Layer: Packet drop (Normal for testing)."));
  }
  display.display();
  delay(1500);
  updateHomeScreen();
}

// --- Main Setup Core ---
void setup() {
  Serial.begin(74880);
  delay(500);
  Serial.println(F("\nInitializing Master Node Boot Sequences..."));
  
  display.init();
  display.flipScreenVertically(); 

  // Initialize Wi-Fi in Station Mode for ESP-NOW compatibility
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println(F("ESP-NOW Initialization Failed."));
    display.clear();
    display.drawString(0, 0, "ESP-NOW Error");
    display.display();
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  updateHomeScreen();
}

// --- Main Loop Execution ---
void loop() {
  char key = keypad.getKey(); 

  // Poll virtual Serial Monitor if no hardware keypad press is detected
  if (!key && Serial.available() > 0) {
    key = Serial.read();
    if (key == '\n' || key == '\r') {
      key = 0; 
    } else {
      Serial.print(F("Serial Monitor Input Detected: "));
      Serial.println(key);
    }
  }

  if (key) {
    if (!callMode) {
      if (key == 'A' || key == 'a') { 
        display.clear();
        display.drawString(0, 20, "Sending Bell Alert...");
        display.display();
        
        myData.alertType = 'J'; 
        myData.studentID = 0;   
        debugAndSendData();
      } 
      else if (key == 'B' || key == 'b') { 
        display.clear();
        display.drawString(0, 20, "!!! EMERGENCY !!!");
        display.drawString(0, 32, "Sending Blast...");
        display.display();
        
        myData.alertType = 'E'; 
        myData.studentID = 0;   
        debugAndSendData();
      } 
      else if (key == 'C' || key == 'c') { 
        callMode = true;
        inputBuffer = "";
        display.clear();
        display.drawString(0, 0, "ENTER STUDENT ID:");
        display.drawString(0, 20, "_");
        display.display();
        
        Serial.println(F("System shifted to Student Call Mode. Type ID digits, then '#' to confirm or '*' to cancel."));
      }
    } 
    else { 
      if (key >= '0' && key <= '9') {
        inputBuffer += key;
        display.clear();
        display.drawString(0, 0, "ENTER STUDENT ID:");
        display.drawString(0, 20, inputBuffer);
        display.display();
      } 
      else if (key == '#') { 
        if (inputBuffer.length() > 0) {
          int targetStudent = inputBuffer.toInt();
          
          display.clear();
          display.drawString(0, 0, "Calling Student: ");
          display.drawString(0, 20, inputBuffer);
          display.display();

          myData.alertType = 'C'; 
          myData.studentID = targetStudent;
          debugAndSendData();
          
          callMode = false;
        }
      } 
      else if (key == '*') { 
        callMode = false;
        Serial.println(F("Operation canceled by user."));
        updateHomeScreen();
      }
    }
  }
  yield(); // Keep the background watchdog timer happy
}