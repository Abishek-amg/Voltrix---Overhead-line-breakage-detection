#include <SPI.h>
#include <LoRa.h>

// Pin definitions
#define VOLTAGE_SENSOR_PIN A0  
#define LORA_NSS 5             
#define LORA_RST 14            
#define LORA_DIO0 2        


#define LORA_FREQUENCY 866E6   
#define LORA_TX_POWER 20       


#define SAMPLES 100
#define DELAY_BETWEEN_SAMPLES 1  // ms

// Function to calculate approximate RMS voltage (for 230V AC, calibrate as needed)
float readVoltage() {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    int sensorValue = analogRead(VOLTAGE_SENSOR_PIN);
    sum += sensorValue * sensorValue;  // Square for RMS
    delay(DELAY_BETWEEN_SAMPLES);
  }
  float rms = sqrt((float)sum / SAMPLES) * (3.3 / 4095.0);  
  return rms * 70.71;  
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize LoRa
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(10);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("LoRa init failed! Check wiring.");
    while (1);
  }
  LoRa.setPins(LORA_NSS, -1, LORA_DIO0);  // Set pins
  LoRa.setTxPower(LORA_TX_POWER);
  LoRa.setSpreadingFactor(7);  
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0xF3); 

  Serial.println("VOLTRIX Transmitter Ready! Sending voltage every 10s.");
}

void loop() {
  float voltage = readVoltage();
  
  String packet = "VOLT:" + String(voltage, 1); 
  
  LoRa.beginPacket();
  LoRa.print(packet);
  int rssi = LoRa.endPacket();
  
  if (rssi > 0) {
    Serial.println("Sent: " + packet + " | RSSI: " + String(rssi));
  } else {
    Serial.println("Send failed!");
  }
  
  delay(5000);  // Send every 5 seconds (adjust for needs)
}