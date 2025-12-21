#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// LoRa Pins for ESP8266 (GPIO numbers)
#define LORA_CS 15      // GPIO15
#define LORA_RST 16     // GPIO16
#define LORA_DIO0 5     // GPIO5

#define RELAY_PIN 14    // GPIO14
#define VOLTAGE_PIN A0  // Analog pin

// WiFi & Firebase
#define SSID "YOUR_SSID"
#define PASSWORD "YOUR_PASSWORD"
#define FIREBASE_HOST "YOUR_PROJECT.firebaseio.com"
#define FIREBASE_AUTH "YOUR_DATABASE_SECRET"

// Variables
FirebaseData firebaseData;
float txVoltage = 0, rxVoltage = 0;
float THRESHOLD = 2.0;
unsigned long lastMsg = 0;
bool isOnline = false;

// Offline storage (max 50 readings)
struct Reading {
  float tx, rx;
  String status;
};
Reading offlineData[50];
int offlineCount = 0;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK");
    isOnline = true;
  } else {
    Serial.println("\nWiFi Failed - Offline Mode");
    isOnline = false;
  }
}

void initLoRa() {
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
  
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa Failed!");
    while (1);
  }
  
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  Serial.println("LoRa OK");
}

float readVoltage() {
  int raw = analogRead(VOLTAGE_PIN);
  return (raw / 1023.0) * 3.3;  // ESP8266 has 10-bit ADC
}

void sendToFirebase() {
  if (!isOnline) return;
  
  Firebase.setFloat(firebaseData, "/voltage/rx", rxVoltage);
  Firebase.setFloat(firebaseData, "/voltage/tx", txVoltage);
  Firebase.setFloat(firebaseData, "/voltage/diff", abs(txVoltage - rxVoltage));
}

void sendAlert(String msg) {
  Serial.println("ALERT: " + msg);
  
  if (isOnline) {
    Firebase.setString(firebaseData, "/alerts/latest", msg);
  }
  
  // Trigger relay
  digitalWrite(RELAY_PIN, HIGH);
  delay(2000);
  digitalWrite(RELAY_PIN, LOW);
}

void storeOffline(float tx, float rx, String status) {
  if (offlineCount < 50) {
    offlineData[offlineCount].tx = tx;
    offlineData[offlineCount].rx = rx;
    offlineData[offlineCount].status = status;
    offlineCount++;
  }
}

void uploadOfflineData() {
  if (offlineCount == 0) return;
  
  Serial.println("Uploading offline data...");
  for (int i = 0; i < offlineCount; i++) {
    String path = "/offline/reading_" + String(i);
    Firebase.setFloat(firebaseData, path + "/tx", offlineData[i].tx);
    Firebase.setFloat(firebaseData, path + "/rx", offlineData[i].rx);
    Firebase.setString(firebaseData, path + "/status", offlineData[i].status);
  }
  offlineCount = 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  Serial.println("\n\nLoRa Voltage Monitor (ESP8266)");
  
  initWiFi();
  initLoRa();
  
  FirebaseConfig config;
  FirebaseAuth auth;
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  
  Serial.println("Ready!");
}

void loop() {
  // Receive LoRa
  int pktSize = LoRa.parsePacket();
  if (pktSize) {
    lastMsg = millis();
    String msg = "";
    
    while (LoRa.available()) {
      msg += (char)LoRa.read();
    }
    
    // Parse: "tx:12.5" format
    if (msg.indexOf("tx:") != -1) {
      int idx = msg.indexOf(":") + 1;
      txVoltage = msg.substring(idx).toFloat();
      Serial.println("TX: " + String(txVoltage, 2) + "V");
    }
  }
  
  // Read receiver voltage
  rxVoltage = readVoltage();
  
  // Check mismatch every 2 seconds
  if (millis() - lastMsg > 2000) {
    lastMsg = millis();
    
    float diff = abs(txVoltage - rxVoltage);
    
    if (diff > THRESHOLD) {
      String alert = "Mismatch! TX:" + String(txVoltage, 1) + 
                     "V RX:" + String(rxVoltage, 1) + "V";
      
      sendAlert(alert);
      
      if (!isOnline) {
        storeOffline(txVoltage, rxVoltage, "MISMATCH");
      } else {
        sendToFirebase();
      }
    } else if (isOnline) {
      sendToFirebase();
    } else {
      storeOffline(txVoltage, rxVoltage, "OK");
    }
  }
  
  // Check WiFi reconnection
  if (!isOnline && WiFi.status() == WL_CONNECTED) {
    isOnline = true;
    Serial.println("WiFi Reconnected!");
    uploadOfflineData();
  } else if (isOnline && WiFi.status() != WL_CONNECTED) {
    isOnline = false;
    Serial.println("WiFi Disconnected!");
  }
  
  delay(100);
}