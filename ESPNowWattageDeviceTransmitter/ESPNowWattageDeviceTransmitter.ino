#include <Arduino.h>
#include "esp_now.h"
#include <esp_mac.h>
#include "WiFi.h"

#include <Adafruit_MLX90614.h>
#include <Adafruit_INA219.h>
#include "config.h"
#include "sanityCheck.h"

#ifndef debug
Adafruit_MLX90614 mlx1, mlx2, mlx3, mlx4;
Adafruit_INA219 ina219;
#endif

float volt[2];
float curr[2];
float real_p[2];
float app_p[2];
float max_volt[2];
float max_curr[2];
float max_real[2];
float max_app[2];
float pf[2];
float imp[2];
float min_imp[2];
float max_imp[2];
float freq[2];
float svx[2];
float cvx;
float hvx;
float lvx;
float temperature[4];
float maxTemp[4];
int noOfSensors;
bool clipping[2];
bool unitC;
bool enteredFirstTime = true;

uint8_t receiverMac[] = RECEIVER_MAC;

typedef struct __attribute__((packed)) wattageDeviceData {
  float volt[2];
  float curr[2];
  float real_p[2];
  float app_p[2];
  float max_volt[2];
  float max_curr[2];
  float max_real[2];
  float max_app[2];
  float pf[2];
  float imp[2];
  float min_imp[2];
  float max_imp[2];
  float freq[2];
  float svx[2];
  bool clipping[2];
  float cvx;
  float hvx;
  float lvx;
  float temperature[4];
  float maxTemp[4];
  bool unitC;
  int noOfSensors;
  int hash;
} wattageDeviceData;

wattageDeviceData sendingData;
esp_now_peer_info_t peerInfo;

int calculateHash(const wattageDeviceData &data) {
  const uint8_t* bytes = (const uint8_t*)&data;
  size_t len = offsetof(wattageDeviceData, hash); // exclude hash field

  uint32_t hash = 0;
  for (size_t i = 0; i < len; i++) {
    hash = (hash * 31) ^ bytes[i]; // simple multiplicative hash
  }

  return (int)(hash & 0x7FFFFFFF);
}

void onDataSent(const uint8_t *macAddr, esp_now_send_status_t sendStatus) {
  if (sendStatus == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Message sent successfully");
    Serial.printf("Calculated Hash: %d / Sent Hash: %d\n", sendingData.hash, sendingData.hash);
    Serial.printf("Receiver MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", receiverMac[0], receiverMac[1], receiverMac[2], receiverMac[3], receiverMac[4], receiverMac[5]);
    Serial.printf("Volt: %.2f / %.2f\n", sendingData.volt[0], sendingData.volt[1]);
    Serial.printf("Current: %.2f / %.2f\n", sendingData.curr[0], sendingData.curr[1]);
    Serial.printf("Real Power: %.2f / %.2f\n", sendingData.real_p[0], sendingData.real_p[1]);
    Serial.printf("Apparant Power: %.2f / %.2f\n", sendingData.app_p[0], sendingData.app_p[1]);
    Serial.printf("Max Volt: %.2f / %.2f\n", sendingData.max_volt[0], sendingData.max_volt[1]);
    Serial.printf("MAx Current: %.2f / %.2f\n", sendingData.max_curr[0], sendingData.max_curr[1]);
    Serial.printf("Max Real Power: %.2f / %.2f\n", sendingData.max_real[0], sendingData.max_real[1]);
    Serial.printf("Max Apparant Power: %.2f / %.2f\n", sendingData.max_app[0], sendingData.max_app[1]);
    Serial.printf("Power Factor: %.2f / %.2f\n", sendingData.pf[0], sendingData.pf[1]);
    Serial.printf("Imp: %.2f / %.2f\n", sendingData.imp[0], sendingData.imp[1]);
    Serial.printf("Min Imp: %.2f / %.2f\n", sendingData.min_imp[0], sendingData.min_imp[1]);
    Serial.printf("Max Imp: %.2f / %.2f\n", sendingData.max_imp[0], sendingData.max_imp[1]);
    Serial.printf("Frequency: %.2f / %.2f\n", sendingData.freq[0], sendingData.freq[1]);
    Serial.printf("Starting Volt: %.2f / %.2f\n", sendingData.svx[0], sendingData.svx[1]);
    Serial.printf("Clipping: %.2f / %.2f\n", sendingData.clipping[0], sendingData.clipping[1]);
    Serial.print("Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", sendingData.temperature[i]);
    }
    Serial.println();
    Serial.print("Maximum Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", sendingData.maxTemp[i]);
    }
    Serial.println();
    Serial.printf("Unit_C: %d / Used_Sensors:  %d / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n", sendingData.unitC, sendingData.noOfSensors, sendingData.cvx, sendingData.hvx, sendingData.lvx);
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
    for (int i = 0; i < 4; i++) {
      maxTemp[i] = 0;
      temperature[i] = 0;
    }
    for (int i = 0; i < 2; i++) {
      svx[i] = random(-26, +26);
      clipping[i] = random(0, 1);
      freq[i] = random(10, 100);
      volt[i] = random(10, 250);
      curr[i] = random(2, 50);
      real_p[i] = random(10, 50);
      app_p[i] = random(10, 50);
      max_volt[i] = random(10, 250);
      max_curr[i] = random(2, 60);
      max_real[i] = random(10, 50);
      max_app[i] = random(10, 50);
      pf[i] = random(10, 100);
      imp[i] = random(10, 100);
      min_imp[i] = random(10, 100);
      max_imp[i] = random(10, 100);
    }
    cvx = svx[0];
    lvx = svx[0];
    hvx = svx[0];
    enteredFirstTime = false;
  } else {
    for (int i = 0; i < 2; i++) {
      clipping[i] = random(0, 1);
      freq[i] = random(10, 100);
      volt[i] = random(10, 250);
      curr[i] = random(2, 50);
      real_p[i] = random(10, 50);
      app_p[i] = random(10, 50);
      max_volt[i] = random(10, 250);
      max_curr[i] = random(2, 60);
      max_real[i] = random(10, 50);
      max_app[i] = random(10, 50);
      pf[i] = random(10, 100);
      imp[i] = random(10, 100);
      min_imp[i] = random(10, 100);
      max_imp[i] = random(10, 100);
    }
    cvx = random(-26, 26);
    if (cvx >= hvx) hvx = cvx;
    if (cvx <= lvx) lvx = cvx;
  }
  noOfSensors = SENSORS_USED;
#ifdef USE_UNITS_C
  unitC = true;
#else
  unitC = false;
#endif
  for (int i = 0; i < SENSORS_USED; i++) {
    temperature[i] = random(10, 120);
    if (temperature[i] > maxTemp[i]) {
      maxTemp[i] = temperature[i];
    }
  }
#else
  if (enteredFirstTime) {
    for (int i = 0; i < 4; i++) {
      maxTemp[i] = 0;
      temperature[i] = 0;
    }
    svx = ina219.getBusVoltage_V();
    cvx = svx;
    lvx = svx;
    hvx = svx;
    for (int i = 0; i < 2; i++) {
      clipping[i] = random(0, 1);
      freq[i] = random(10, 100);
      volt[i] = random(10, 250);
      curr[i] = random(2, 50);
      real_p[i] = random(10, 50);
      app_p[i] = random(10, 50);
      max_volt[i] = random(10, 250);
      max_curr[i] = random(2, 60);
      max_real[i] = random(10, 50);
      max_app[i] = random(10, 50);
      pf[i] = random(10, 100);
      imp[i] = random(10, 100);
      min_imp[i] = random(10, 100);
      max_imp[i] = random(10, 100);
    }
    enteredFirstTime = false;
  } else {
    for (int i = 0; i < 2; i++) {
      clipping[i] = random(0, 1);
      freq[i] = random(10, 100);
      volt[i] = random(10, 250);
      curr[i] = random(2, 50);
      real_p[i] = random(10, 50);
      app_p[i] = random(10, 50);
      max_volt[i] = random(10, 250);
      max_curr[i] = random(2, 60);
      max_real[i] = random(10, 50);
      max_app[i] = random(10, 50);
      pf[i] = random(10, 100);
      imp[i] = random(10, 100);
      min_imp[i] = random(10, 100);
      max_imp[i] = random(10, 100);
    }
    cvx = ina219.getBusVoltage_V();
    if (cvx >= hvx) hvx = cvx;
    if (cvx <= lvx) lvx = cvx;
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
    }
    temperature[i] = temp;
    if (temperature[i] >= maxTemp[i]) {
      maxTemp[i] = temperature[i];
    }
    delay(DELAY);
  }
#endif

  memcpy(&sendingData.volt, &volt, sizeof(volt));
  memcpy(&sendingData.curr, &curr, sizeof(curr));
  memcpy(&sendingData.real_p, &real_p, sizeof(real_p));
  memcpy(&sendingData.app_p, &app_p, sizeof(app_p));
  memcpy(&sendingData.max_volt, &max_volt, sizeof(max_volt));
  memcpy(&sendingData.max_curr, &max_curr, sizeof(max_curr));
  memcpy(&sendingData.max_real, &max_real, sizeof(max_real));
  memcpy(&sendingData.max_app, &max_app, sizeof(max_app));
  memcpy(&sendingData.pf, &pf, sizeof(pf));
  memcpy(&sendingData.imp, &imp, sizeof(imp));
  memcpy(&sendingData.min_imp, &min_imp, sizeof(min_imp));
  memcpy(&sendingData.max_imp, &max_imp, sizeof(max_imp));
  memcpy(&sendingData.freq, &freq, sizeof(freq));
  memcpy(&sendingData.svx, &svx, sizeof(svx));
  memcpy(&sendingData.clipping, &clipping, sizeof(clipping));
  sendingData.cvx = cvx;
  sendingData.lvx = lvx;
  sendingData.hvx = hvx;
  memcpy(&sendingData.temperature, &temperature, sizeof(temperature));
  memcpy(&sendingData.maxTemp, &maxTemp, sizeof(maxTemp));
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
  delay(10000);
}