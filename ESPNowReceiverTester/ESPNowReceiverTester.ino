#include <Arduino.h>
#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>
#include <esp_mac.h>
#include "WiFi.h"

#include "config.h"
#include "sanityCheck.h"


double temperature[MAX_SENSOR];
double maxTemp[MAX_SENSOR];
int noOfSensors;
float ivx;
float hvx;
float lvx;
float svx;
bool unitC;

uint8_t transmittererMAC[] = TRANSMITTER_MAC;

typedef struct espNowData {
  double temperature[16];  // Temperature Array Containing Temperature Readings from all sensors
  float maxTemp[16];      // Maximum Temperature Array Containing Maximum Temperature Readings from all sensors
  int noOfSensors;         // Number of sensors being used
  float ivx;               // Instantaneus(Live) Voltage Value
  float hvx;               // Maximum(Highest) Voltage Value
  float lvx;               // Minimum(Lowest) Voltage Value
  float svx;               // Starting(First Reading) Voltage Value
  bool unitC;              // Flag to select Temperature Unit (°C or °F)
  int hash;                // Hash Calculated based on all other values in the Structure. Used to Detect Data Corruption ot Error
} espNowData;

espNowData receivedData;
esp_now_peer_info_t peerInfo;

int calculateHash(const espNowData &data) {
  unsigned long hash = 0;

  // Process temperature and maxTemp arrays
  for (int i = 0; i < data.noOfSensors && i < 16; i++) {
    hash ^= (unsigned long)(data.temperature[i] * 1000);  // scale to avoid float loss
    hash ^= (unsigned long)(data.maxTemp[i] * 1000) << 1;
  }

  // Process voltage values
  hash ^= (unsigned long)(data.ivx * 1000) << 2;
  hash ^= (unsigned long)(data.hvx * 1000) << 3;
  hash ^= (unsigned long)(data.lvx * 1000) << 4;
  hash ^= (unsigned long)(data.svx * 1000) << 5;

  // Add simple values
  hash ^= data.noOfSensors << 6;
  hash ^= (data.unitC ? 0xAAAA : 0x5555);  // arbitrary distinction

  // Final mixing
  hash = (hash ^ (hash >> 16)) & 0x7FFFFFFF;  // keep it positive 32-bit int

  return (int)hash;
}

void onDataReceived(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  uint8_t * src_addr = recv_info->src_addr;
  Serial.printf("Transmitter MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  int temporaryHash = calculateHash(receivedData);
  Serial.printf("Calculated Hash: %d / Received Hash: %d\n",temporaryHash,receivedData.hash);
  Serial.print("Data received: ");
  Serial.println(len);
  if(temporaryHash == receivedData.hash){
    Serial.println("Data Received Successfully");
    Serial.print("Temperature: ");
    for(int i =0 ; i< MAX_SENSOR; i++){
      Serial.printf("%.2f / ",receivedData.temperature[i]);
    }
    Serial.println();
    Serial.print("Maximum Temperature: ");
    for(int i =0 ; i< MAX_SENSOR; i++){
      Serial.printf("%.2f / ",receivedData.maxTemp[i]);
    }
    Serial.println();
    Serial.printf("Unit_C: %d / Used_Sensors:  %d / Start_volts: %.2f / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n",receivedData.unitC,receivedData.noOfSensors,receivedData.svx,receivedData.ivx,receivedData.hvx,receivedData.lvx);
    Serial.printf("\n\n");
  }
  else{
    Serial.println("Data is Corrupted");
  }
}

void esp_init() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataReceived);
}

void updateReceivedData() {
  svx = receivedData.svx;
  ivx = receivedData.ivx;
  lvx = receivedData.lvx;
  hvx = receivedData.hvx;
  for (int i = 0; i < MAX_SENSOR; i++) {
    temperature[i] = receivedData.temperature[i];
    maxTemp[i] = receivedData.maxTemp[i];
  }
  unitC = receivedData.unitC;
  noOfSensors = receivedData.noOfSensors;
}

void setup() {
  Serial.begin(115200);
  esp_init();
}

void loop() {
  updateReceivedData();
  delay(DELAY);
}