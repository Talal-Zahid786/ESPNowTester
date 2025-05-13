#include <Arduino.h>
#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>
#include <esp_mac.h>
#include "WiFi.h"

#include <Adafruit_INA219.h>
#include <Adafruit_MLX90614.h>

#include "config.h"
#include "sanityCheck.h"

#ifndef debug
Adafruit_MLX90614 mlx1, mlx2, mlx3, mlx4, mlx5, mlx6, mlx7, mlx8, mlx9, mlx10, mlx11, mlx12, mlx13, mlx14, mlx15, mlx16;
Adafruit_INA219 ina219;
#endif

#ifdef USE_UNITS_C
String unit_str = "-C";
#elif defined(USE_UNITS_F)
String unit_str = "-F";
#endif

double temperature[MAX_SENSOR];
double maxTemp[MAX_SENSOR];
int noOfSensors;
float ivx;
float hvx;
float lvx;
float svx;
bool unitC;
bool enteredFirstTime = true;

uint8_t receiverMac[] = RECEIVER_MAC;

typedef struct espNowData {
  double temperature[16];
  float maxTemp[16];
  int noOfSensors;
  float ivx;
  float hvx;
  float lvx;
  float svx;
  bool unitC;
  int hash;
} espNowData;

espNowData sendingData;
esp_now_peer_info_t peerInfo;

int calculateHash(const espNowData &data) {
  unsigned long hash = 0;
  for (int i = 0; i < data.noOfSensors && i < 16; i++) {
    hash ^= (unsigned long)(data.temperature[i] * 1000);
    hash ^= (unsigned long)(data.maxTemp[i] * 1000) << 1;
  }
  hash ^= (unsigned long)(data.ivx * 1000) << 2;
  hash ^= (unsigned long)(data.hvx * 1000) << 3;
  hash ^= (unsigned long)(data.lvx * 1000) << 4;
  hash ^= (unsigned long)(data.svx * 1000) << 5;
  hash ^= data.noOfSensors << 6;
  hash ^= (data.unitC ? 0xAAAA : 0x5555);
  hash = (hash ^ (hash >> 16)) & 0x7FFFFFFF;
  return (int)hash;
}

void onDataSent(const uint8_t *macAddr, esp_now_send_status_t sendStatus) {
  if (sendStatus == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Message sent successfully");
    Serial.printf("Calculated Hash: %d / Sent Hash: %d\n", sendingData.hash, sendingData.hash);
    Serial.printf("Receiver MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", receiverMac[0], receiverMac[1], receiverMac[2], receiverMac[3], receiverMac[4], receiverMac[5]);
    Serial.print("Temperature: ");
    for (int i = 0; i < MAX_SENSOR; i++) {
      Serial.printf("%.2f / ", sendingData.temperature[i]);
    }
    Serial.println();
    Serial.print("Maximum Temperature: ");
    for (int i = 0; i < MAX_SENSOR; i++) {
      Serial.printf("%.2f / ", sendingData.maxTemp[i]);
    }
    Serial.println();
    Serial.printf("Unit_C: %d / Used_Sensors:  %d / Start_volts: %.2f / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n", sendingData.unitC, sendingData.noOfSensors, sendingData.svx, sendingData.ivx, sendingData.hvx, sendingData.lvx);
  } else {
    Serial.println("Message failed to send");
  }
  Serial.printf("\n\n");
}

void esp_init() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_send_cb(onDataSent);

#ifndef debug
  for (int i = 0; i < SENSORS_USED; i++) {
    switch (i) {
      case 0: mlx1.begin(temperatureI2cAddress[i]); break;
      case 1: mlx2.begin(temperatureI2cAddress[i]); break;
      case 2: mlx3.begin(temperatureI2cAddress[i]); break;
      case 3: mlx4.begin(temperatureI2cAddress[i]); break;
      case 4: mlx5.begin(temperatureI2cAddress[i]); break;
      case 5: mlx6.begin(temperatureI2cAddress[i]); break;
      case 6: mlx7.begin(temperatureI2cAddress[i]); break;
      case 7: mlx8.begin(temperatureI2cAddress[i]); break;
      case 8: mlx9.begin(temperatureI2cAddress[i]); break;
      case 9: mlx10.begin(temperatureI2cAddress[i]); break;
      case 10: mlx11.begin(temperatureI2cAddress[i]); break;
      case 11: mlx12.begin(temperatureI2cAddress[i]); break;
      case 12: mlx13.begin(temperatureI2cAddress[i]); break;
      case 13: mlx14.begin(temperatureI2cAddress[i]); break;
      case 14: mlx15.begin(temperatureI2cAddress[i]); break;
      case 15: mlx16.begin(temperatureI2cAddress[i]); break;
    }
  }
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
  }
#endif
}

void updateSendingData() {
#ifdef debug
  if (enteredFirstTime) {
    for (int i = 0; i < MAX_SENSOR; i++) {
      maxTemp[i] = 0;
      temperature[i] = 0;
    }
    svx = random(-26, +26);
    ivx = svx;
    lvx = svx;
    hvx = svx;
    enteredFirstTime = false;
  } else {
    ivx = random(-26, 26);
    if (ivx >= hvx) hvx = ivx;
    if (ivx <= lvx) lvx = ivx;
  }
  noOfSensors = SENSORS_USED;
#ifdef USE_UNITS_C
  unitC = true;
#else
  unitC = false;
#endif
  for (int i = 0; i < SENSORS_USED; i++) {
    temperature[i] = random(10, 120);
    if (temperature[i] >= maxTemp[i]) {
      maxTemp[i] = temperature[i];
    }
  }
#else
  if (enteredFirstTime) {
    for (int i = 0; i < MAX_SENSOR; i++) {
      maxTemp[i] = 0;
      temperature[i] = 0;
    }
    svx = ina219.getBusVoltage_V();
    ivx = svx;
    lvx = svx;
    hvx = svx;
    enteredFirstTime = false;
  } else {
    ivx = ina219.getBusVoltage_V();
    if (ivx >= hvx) hvx = ivx;
    if (ivx <= lvx) lvx = ivx;
  }
  noOfSensors = SENSORS_USED;
  unitC = defined(USE_UNITS_C);

  for (int i = 0; i < SENSORS_USED; i++) {
    double temp = 0;
    switch (i) {
      case 0: temp = unitC ? mlx1.readObjectTempC() : mlx1.readObjectTempF(); break;
      case 1: temp = unitC ? mlx2.readObjectTempC() : mlx2.readObjectTempF(); break;
      case 2: temp = unitC ? mlx3.readObjectTempC() : mlx3.readObjectTempF(); break;
      case 3: temp = unitC ? mlx4.readObjectTempC() : mlx4.readObjectTempF(); break;
      case 4: temp = unitC ? mlx5.readObjectTempC() : mlx5.readObjectTempF(); break;
      case 5: temp = unitC ? mlx6.readObjectTempC() : mlx6.readObjectTempF(); break;
      case 6: temp = unitC ? mlx7.readObjectTempC() : mlx7.readObjectTempF(); break;
      case 7: temp = unitC ? mlx8.readObjectTempC() : mlx8.readObjectTempF(); break;
      case 8: temp = unitC ? mlx9.readObjectTempC() : mlx9.readObjectTempF(); break;
      case 9: temp = unitC ? mlx10.readObjectTempC() : mlx10.readObjectTempF(); break;
      case 10: temp = unitC ? mlx11.readObjectTempC() : mlx11.readObjectTempF(); break;
      case 11: temp = unitC ? mlx12.readObjectTempC() : mlx12.readObjectTempF(); break;
      case 12: temp = unitC ? mlx13.readObjectTempC() : mlx13.readObjectTempF(); break;
      case 13: temp = unitC ? mlx14.readObjectTempC() : mlx14.readObjectTempF(); break;
      case 14: temp = unitC ? mlx15.readObjectTempC() : mlx15.readObjectTempF(); break;
      case 15: temp = unitC ? mlx16.readObjectTempC() : mlx16.readObjectTempF(); break;
    }
    temperature[i] = temp;
    if (temperature[i] >= maxTemp[i]) {
      maxTemp[i] = temperature[i];
    }
  }
#endif

  sendingData.svx = svx;
  sendingData.ivx = ivx;
  sendingData.lvx = lvx;
  sendingData.hvx = hvx;
  for (int i = 0; i < MAX_SENSOR; i++) {
    sendingData.temperature[i] = temperature[i];
    sendingData.maxTemp[i] = maxTemp[i];
  }
  sendingData.unitC = unitC;
  sendingData.noOfSensors = noOfSensors;
  sendingData.hash = calculateHash(sendingData);
}

void setup() {
  Serial.begin(115200);
  esp_init();
}

void loop() {
  updateSendingData();
  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&sendingData, sizeof(sendingData));
  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  } else {
    Serial.println("Sending error");
  }
  delay(DELAY);
}