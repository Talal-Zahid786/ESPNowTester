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

uint8_t receierMAC[] = RECEIVER_MAC;

typedef struct espNowData {
  double temperature[16];  // Temperature Array Containing Temperature Readings from all sensors
  double maxTemp[16];      // Maximum Temperature Array Containing Maximum Temperature Readings from all sensors
  int noOfSensors;         // Number of sensors being used
  float ivx;               // Instantaneus(Live) Voltage Value
  float hvx;               // Maximum(Highest) Voltage Value
  float lvx;               // Minimum(Lowest) Voltage Value
  float svx;               // Starting(First Reading) Voltage Value
  bool unitC;              // Flag to select Temperature Unit (°C or °F)
  int hash;                // Hash Calculated based on all other values in the Structure. Used to Detect Data Corruption ot Error
} espNowData;

espNowData sendingData;
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

void onDataSent(const uint8_t *macAddr, esp_now_send_status_t sendStatus) {
  if (sendStatus == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Message failed to send");
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

  // Register peer
  memcpy(peerInfo.peer_addr, receierMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_send_cb(onDataSent);

#ifndef debug
  for (int i = 0; i < SENSORS_USED; i++) {
    switch (i) {
      case 0:
        mlx1.begin(temperatureI2cAddress[i]);
      case 1:
        mlx2.begin(temperatureI2cAddress[i]);
        break;
      case 2:
        mlx3.begin(temperatureI2cAddress[i]);
        break;
      case 3:
        mlx4.begin(temperatureI2cAddress[i]);
        break;
      case 4:
        mlx5.begin(temperatureI2cAddress[i]);
        break;
      case 5:
        mlx6.begin(temperatureI2cAddress[i]);
        break;
      case 6:
        mlx7.begin(temperatureI2cAddress[i]);
        break;
      case 7:
        mlx8.begin(temperatureI2cAddress[i]);
        break;
      case 8:
        mlx9.begin(temperatureI2cAddress[i]);
        break;
      case 9:
        mlx10.begin(temperatureI2cAddress[i]);
        break;
      case 10:
        mlx11.begin(temperatureI2cAddress[i]);
        break;
      case 11:
        mlx12.begin(temperatureI2cAddress[i]);
        break;
      case 12:
        mlx13.begin(temperatureI2cAddress[i]);
        break;
      case 13:
        mlx14.begin(temperatureI2cAddress[i]);
        break;
      case 14:
        mlx15.begin(temperatureI2cAddress[i]);
        break;
      case 15:
        mlx16.begin(temperatureI2cAddress[i]);
        break;
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
      if (ivx >= hvx) {
        hvx = ivx;
      } else if (ivx <= lvx) {
        lvx = ivx;
      }
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
    if (ivx >= hvx) {
      hvx = ivx;
    } else if (ivx <= lvx) {
      lvx = ivx;
    }
  }
  noOfSensors = SENSORS_USED;
#ifdef(USE_UNITS_C)
  unitC = true;
#else
  unitC = false;
#endif
  for (int i = 0; i < SENSORS_USED; i++) {
    switch (i) {
      case 0:
#ifdef USE_UNITS_C
        temperature[i] = mlx1.readObjectTempC();
#elif defined(USE_UNITS_F)
        temperature[i] = mlx1.readObjectTempF();
#endif
        break;
      case 1:
#ifdef USE_UNITS_C
        temperature[i] = mlx2.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx2.readObjectTempF();
#endif
          break;
        case 2:
#ifdef USE_UNITS_C
            temperature[i] = mlx3.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx3.readObjectTempF();
#endif
          break;
        case 3:
#ifdef USE_UNITS_C
            temperature[i] = mlx4.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx4.readObjectTempF();
#endif
          break;
        case 4:
#ifdef USE_UNITS_C
            temperature[i] = mlx5.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx5.readObjectTempF();
#endif
          break;
        case 5:
#ifdef USE_UNITS_C
            temperature[i] = mlx6.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx6.readObjectTempF();
#endif
          break;
        case 6:
#ifdef USE_UNITS_C
            temperature[i] = mlx7.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx7.readObjectTempF();
#endif
          break;
        case 7:
#ifdef USE_UNITS_C
            temperature[i] = mlx8.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx8.readObjectTempF();
#endif
          break;
        case 8:
#ifdef USE_UNITS_C
            temperature[i] = mlx9.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx9.readObjectTempF();
#endif
          break;
        case 9:
#ifdef USE_UNITS_C
            temperature[i] = mlx10.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx10.readObjectTempF();
#endif
          break;
        case 10:
#ifdef USE_UNITS_C
            temperature[i] = mlx11.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx11.readObjectTempF();
#endif
          break;
        case 11:
#ifdef USE_UNITS_C
            temperature[i] = mlx12.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx12.readObjectTempF();
#endif
          break;
        case 12:
#ifdef USE_UNITS_C
            temperature[i] = mlx13.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx13.readObjectTempF();
#endif
          break;
        case 13:
#ifdef USE_UNITS_C
            temperature[i] = mlx14.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx14.readObjectTempF();
#endif
          break;
        case 14:
#ifdef USE_UNITS_C
            temperature[i] = mlx15.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx15.readObjectTempF();
#endif
          break;
        case 15:
#ifdef USE_UNITS_C
            temperature[i] = mlx16.readObjectTempC();
#elif defined(USE_UNITS_F)
            temperature[i] = mlx16.readObjectTempF();
#endif
          break;
    }
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
  esp_err_t result = esp_now_send(receierMAC, (uint8_t *)&sendingData, sizeof(sendingData));
  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  } else {
    Serial.println("Sending error");
  }
}