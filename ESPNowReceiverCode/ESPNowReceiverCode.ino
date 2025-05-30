#include <Arduino.h>
#include "esp_now.h"
#include <esp_mac.h>
#include "WiFi.h"

#include "config.h"
#include "sanityCheck.h"

float temperature[16];
float maxTemp[16];
int noOfSensors;
float cvx;
float hvx;
float lvx;
float svx;
bool unitC;

uint8_t temperatureTransmitterMAC[] = TEMPERATURE_TRANSMITTER_MAC;
uint8_t wattageTransmitter1MAC[] = Wattage_TRANSMITTER1_MAC;
uint8_t wattageTransmitter2MAC[] = Wattage_TRANSMITTER2_MAC;
uint8_t wattageTransmitter3MAC[] = Wattage_TRANSMITTER3_MAC;
uint8_t wattageTransmitter4MAC[] = Wattage_TRANSMITTER4_MAC;

typedef struct __attribute__((packed)) tempDeviceData {
  float temperature[16];  // Temperature Array Containing Temperature Readings from all sensors
  float maxTemp[16];      // Maximum Temperature Array Containing Maximum Temperature Readings from all sensors
  int noOfSensors;        // Number of sensors being used
  float cvx;              // Instantaneus(Live) Voltage Value
  float hvx;              // Maximum(Highest) Voltage Value
  float lvx;              // Minimum(Lowest) Voltage Value
  float svx;              // Starting(First Reading) Voltage Value
  bool unitC;             // Flag to select Temperature Unit (°C or °F)
  int hash;               // Hash Calculated based on all other values in the Structure. Used to Detect Data Corruption ot Error
} tempDeviceData;

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


typedef struct __attribute__((packed)) uartSendingData {
  float temperature_TemperatureDevice[16];  // Temperature Array Containing Temperature Readings from all sensors
  float maxTemp_TemperatureDevice[16];      // Maximum Temperature Array Containing Maximum Temperature Readings from all sensors
  float cvx_TemperatureDevice;              // Instantaneus(Live) Voltage Value
  float hvx_TemperatureDevice;              // Maximum(Highest) Voltage Value
  float lvx_TemperatureDevice;              // Minimum(Lowest) Voltage Value
  float svx_TemperatureDevice;              // Starting(First Reading) Voltage Value
  bool unitC_TemperatureDevice;             // Flag to select Temperature Unit (°C or °F)
  int noOfSensors_TemperatureDevice;        // Number of sensors being used
  float volt_WattageDevice1[2];
  float curr_WattageDevice1[2];
  float real_p_WattageDevice1[2];
  float app_p_WattageDevice1[2];
  float max_volt_WattageDevice1[2];
  float max_curr_WattageDevice1[2];
  float max_real_WattageDevice1[2];
  float max_app_WattageDevice1[2];
  float pf_WattageDevice1[2];
  float imp_WattageDevice1[2];
  float min_imp_WattageDevice1[2];
  float max_imp_WattageDevice1[2];
  float freq_WattageDevice1[2];
  float svx_WattageDevice1[2];
  bool clipping_WattageDevice1[2];
  float cvx_WattageDevice1;
  float hvx_WattageDevice1;
  float lvx_WattageDevice1;
  float temperature_WattageDevice1[4];
  float maxTemp_WattageDevice1[4];
  bool unitC_WattageDevice1;
  int noOfSensors_WattageDevice1;
  float volt_WattageDevice2[2];
  float curr_WattageDevice2[2];
  float real_p_WattageDevice2[2];
  float app_p_WattageDevice2[2];
  float max_volt_WattageDevice2[2];
  float max_curr_WattageDevice2[2];
  float max_real_WattageDevice2[2];
  float max_app_WattageDevice2[2];
  float pf_WattageDevice2[2];
  float imp_WattageDevice2[2];
  float min_imp_WattageDevice2[2];
  float max_imp_WattageDevice2[2];
  float freq_WattageDevice2[2];
  float svx_WattageDevice2[2];
  bool clipping_WattageDevice2[2];
  float cvx_WattageDevice2;
  float hvx_WattageDevice2;
  float lvx_WattageDevice2;
  float temperature_WattageDevice2[4];
  float maxTemp_WattageDevice2[4];
  bool unitC_WattageDevice2;
  int noOfSensors_WattageDevice2;
  float volt_WattageDevice3[2];
  float curr_WattageDevice3[2];
  float real_p_WattageDevice3[2];
  float app_p_WattageDevice3[2];
  float max_volt_WattageDevice3[2];
  float max_curr_WattageDevice3[2];
  float max_real_WattageDevice3[2];
  float max_app_WattageDevice3[2];
  float pf_WattageDevice3[2];
  float imp_WattageDevice3[2];
  float min_imp_WattageDevice3[2];
  float max_imp_WattageDevice3[2];
  float freq_WattageDevice3[2];
  float svx_WattageDevice3[2];
  bool clipping_WattageDevice3[2];
  float cvx_WattageDevice3;
  float hvx_WattageDevice3;
  float lvx_WattageDevice3;
  float temperature_WattageDevice3[4];
  float maxTemp_WattageDevice3[4];
  bool unitC_WattageDevice3;
  int noOfSensors_WattageDevice3;
  float volt_WattageDevice4[2];
  float curr_WattageDevice4[2];
  float real_p_WattageDevice4[2];
  float app_p_WattageDevice4[2];
  float max_volt_WattageDevice4[2];
  float max_curr_WattageDevice4[2];
  float max_real_WattageDevice4[2];
  float max_app_WattageDevice4[2];
  float pf_WattageDevice4[2];
  float imp_WattageDevice4[2];
  float min_imp_WattageDevice4[2];
  float max_imp_WattageDevice4[2];
  float freq_WattageDevice4[2];
  float svx_WattageDevice4[2];
  bool clipping_WattageDevice4[2];
  float cvx_WattageDevice4;
  float hvx_WattageDevice4;
  float lvx_WattageDevice4;
  float temperature_WattageDevice4[4];
  float maxTemp_WattageDevice4[4];
  bool unitC_WattageDevice4;
  int noOfSensors_WattageDevice4;
} uartSendingData;

uartSendingData uartData;

esp_now_peer_info_t peerInfo;

int calculateTemperatureDeviceHash(const tempDeviceData &data) {
  unsigned long hash = 0;

  // Process temperature and maxTemp arrays
  for (int i = 0; i < data.noOfSensors && i < 16; i++) {
    hash ^= (unsigned long)(data.temperature[i] * 1000);  // scale to avoid float loss
    hash ^= (unsigned long)(data.maxTemp[i] * 1000) << 1;
  }

  // Process voltage values
  hash ^= (unsigned long)(data.cvx * 1000) << 2;
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

int calculateWattageDeviceHash(const wattageDeviceData &data){
  const uint8_t* bytes = (const uint8_t*)&data;
  size_t len = offsetof(wattageDeviceData, hash); // exclude hash field

  uint32_t hash = 0;
  for (size_t i = 0; i < len; i++) {
    hash = (hash * 31) ^ bytes[i]; // simple multiplicative hash
  }

  return (int)(hash & 0x7FFFFFFF);
}

void onDataReceived(const uint8_t *src_addr, const uint8_t *incomingData, int len) {
  if (memcmp(temperatureTransmitterMAC, src_addr, 6) == 0) {
    tempDeviceData receivedTemperatureData;
    memcpy(&receivedTemperatureData, incomingData, sizeof(receivedTemperatureData));
    int temporaryHash = calculateTemperatureDeviceHash(receivedTemperatureData);
    Serial.printf("Calculated Hash: %d / Received Hash: %d\n", temporaryHash, receivedTemperatureData.hash);
    Serial.print("Data received: ");
    Serial.println(len);
    if (temporaryHash == receivedTemperatureData.hash) {
      Serial.println("Data Received Successfully");
      memcpy(&uartData.temperature_TemperatureDevice, receivedTemperatureData.temperature, sizeof(uartData.temperature_TemperatureDevice));
      memcpy(&uartData.maxTemp_TemperatureDevice, receivedTemperatureData.maxTemp, sizeof(uartData.maxTemp_TemperatureDevice));
      uartData.cvx_TemperatureDevice = receivedTemperatureData.cvx;
      uartData.hvx_TemperatureDevice = receivedTemperatureData.hvx;
      uartData.lvx_TemperatureDevice = receivedTemperatureData.lvx;
      uartData.svx_TemperatureDevice = receivedTemperatureData.svx;
      uartData.unitC_TemperatureDevice = receivedTemperatureData.unitC;
      uartData.noOfSensors_TemperatureDevice = receivedTemperatureData.noOfSensors;
      svx = receivedTemperatureData.svx;
      cvx = receivedTemperatureData.cvx;
      lvx = receivedTemperatureData.lvx;
      hvx = receivedTemperatureData.hvx;
      unitC = receivedTemperatureData.unitC;
      noOfSensors = receivedTemperatureData.noOfSensors;
      for (int i = 0; i < noOfSensors; i++) {
        temperature[i] = receivedTemperatureData.temperature[i];
        maxTemp[i] = receivedTemperatureData.maxTemp[i];
      }
      Serial.print("Temperature: ");
      for (int i = 0; i < 16; i++) {
        Serial.printf("%.2f / ", uartData.temperature_TemperatureDevice[i]);
      }
      Serial.println();
      Serial.print("Maximum Temperature: ");
      for (int i = 0; i < 16; i++) {
        Serial.printf("%.2f / ", uartData.maxTemp_TemperatureDevice[i]);
      }
      Serial.println();
      Serial.printf("Unit_C: %d / Used_Sensors:  %d / Start_volts: %.2f / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n", uartData.unitC_TemperatureDevice, uartData.noOfSensors_TemperatureDevice, uartData.svx_TemperatureDevice, uartData.cvx_TemperatureDevice, uartData.hvx_TemperatureDevice, uartData.lvx_TemperatureDevice);
      Serial.printf("\n\n");
      sendDataByUart();
    } else {
      Serial.println("Data is Corrupted");
    }
  } else if (memcmp(wattageTransmitter1MAC, src_addr, 6) == 0) {
    wattageDeviceData receivedWattageData;
    memcpy(&receivedWattageData, incomingData, sizeof(receivedWattageData));
    int temporaryHash = calculateWattageDeviceHash(receivedWattageData);
    Serial.printf("Calculated Hash: %d / Received Hash: %d\n", temporaryHash, receivedWattageData.hash);
    Serial.print("Data received: ");
    Serial.println(len);
    if (temporaryHash == receivedWattageData.hash) {
      Serial.println("Data Received Successfully");
      memcpy(&uartData.volt_WattageDevice1, &receivedWattageData.volt, sizeof(uartData.volt_WattageDevice1));
      memcpy(&uartData.curr_WattageDevice1, &receivedWattageData.curr, sizeof(uartData.curr_WattageDevice1));
      memcpy(&uartData.real_p_WattageDevice1, &receivedWattageData.real_p, sizeof(uartData.real_p_WattageDevice1));
      memcpy(&uartData.app_p_WattageDevice1, &receivedWattageData.app_p, sizeof(uartData.app_p_WattageDevice1));
      memcpy(&uartData.max_volt_WattageDevice1, &receivedWattageData.max_volt, sizeof(uartData.max_volt_WattageDevice1));
      memcpy(&uartData.max_curr_WattageDevice1, &receivedWattageData.max_curr, sizeof(uartData.max_curr_WattageDevice1));
      memcpy(&uartData.max_real_WattageDevice1, &receivedWattageData.max_real, sizeof(uartData.max_real_WattageDevice1));
      memcpy(&uartData.max_app_WattageDevice1, &receivedWattageData.max_app, sizeof(uartData.max_app_WattageDevice1));
      memcpy(&uartData.pf_WattageDevice1, &receivedWattageData.pf, sizeof(uartData.pf_WattageDevice1));
      memcpy(&uartData.imp_WattageDevice1, &receivedWattageData.imp, sizeof(uartData.imp_WattageDevice1));
      memcpy(&uartData.min_imp_WattageDevice1, &receivedWattageData.min_imp, sizeof(uartData.min_imp_WattageDevice1));
      memcpy(&uartData.max_imp_WattageDevice1, &receivedWattageData.max_imp, sizeof(uartData.max_imp_WattageDevice1));
      memcpy(&uartData.freq_WattageDevice1, &receivedWattageData.freq, sizeof(uartData.freq_WattageDevice1));
      memcpy(&uartData.svx_WattageDevice1, &receivedWattageData.svx, sizeof(uartData.svx_WattageDevice1));
      memcpy(&uartData.clipping_WattageDevice1, &receivedWattageData.clipping, sizeof(uartData.clipping_WattageDevice1));
      memcpy(&uartData.temperature_WattageDevice1, &receivedWattageData.temperature, sizeof(uartData.temperature_WattageDevice1));
      memcpy(&uartData.maxTemp_WattageDevice1, &receivedWattageData.maxTemp, sizeof(uartData.maxTemp_WattageDevice1));
      uartData.cvx_WattageDevice1 = receivedWattageData.cvx;
      uartData.hvx_WattageDevice1 = receivedWattageData.hvx;
      uartData.lvx_WattageDevice1 = receivedWattageData.lvx;
      uartData.unitC_WattageDevice1 = receivedWattageData.unitC;
      uartData.noOfSensors_WattageDevice1 = receivedWattageData.noOfSensors;
      Serial.printf("Volt: %.2f / %.2f\n", uartData.volt_WattageDevice1[0], uartData.volt_WattageDevice1[1]);
    Serial.printf("Current: %.2f / %.2f\n", uartData.curr_WattageDevice1[0], uartData.curr_WattageDevice1[1]);
    Serial.printf("Real Power: %.2f / %.2f\n", uartData.real_p_WattageDevice1[0], uartData.real_p_WattageDevice1[1]);
    Serial.printf("Apparant Power: %.2f / %.2f\n", uartData.app_p_WattageDevice1[0], uartData.app_p_WattageDevice1[1]);
    Serial.printf("Max Volt: %.2f / %.2f\n", uartData.max_volt_WattageDevice1[0], uartData.max_volt_WattageDevice1[1]);
    Serial.printf("MAx Current: %.2f / %.2f\n", uartData.max_curr_WattageDevice1[0], uartData.max_curr_WattageDevice1[1]);
    Serial.printf("Max Real Power: %.2f / %.2f\n", uartData.max_real_WattageDevice1[0], uartData.max_real_WattageDevice1[1]);
    Serial.printf("Max Apparant Power: %.2f / %.2f\n", uartData.max_app_WattageDevice1[0], uartData.max_app_WattageDevice1[1]);
    Serial.printf("Power Factor: %.2f / %.2f\n", uartData.pf_WattageDevice1[0], uartData.pf_WattageDevice1[1]);
    Serial.printf("Imp: %.2f / %.2f\n", uartData.imp_WattageDevice1[0], uartData.imp_WattageDevice1[1]);
    Serial.printf("Min Imp: %.2f / %.2f\n", uartData.min_imp_WattageDevice1[0], uartData.min_imp_WattageDevice1[1]);
    Serial.printf("Max Imp: %.2f / %.2f\n", uartData.max_imp_WattageDevice1[0], uartData.max_imp_WattageDevice1[1]);
    Serial.printf("Frequency: %.2f / %.2f\n", uartData.freq_WattageDevice1[0], uartData.freq_WattageDevice1[1]);
    Serial.printf("Starting Volt: %.2f / %.2f\n", uartData.svx_WattageDevice1[0], uartData.svx_WattageDevice1[1]);
    Serial.printf("Clipping: %.2f / %.2f\n", uartData.clipping_WattageDevice1[0], uartData.clipping_WattageDevice1[1]);
    Serial.print("Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.temperature_WattageDevice1[i]);
    }
    Serial.println();
    Serial.print("Maximum Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.maxTemp_WattageDevice1[i]);
    }
    Serial.println();
    Serial.printf("Unit_C: %d / Used_Sensors:  %d / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n", uartData.unitC_WattageDevice1, uartData.noOfSensors_WattageDevice1, uartData.cvx_WattageDevice1, uartData.hvx_WattageDevice1, uartData.lvx_WattageDevice1);
    sendDataByUart();
    } else {
      Serial.println("Data is Corrupted");
    }
  } else if (memcmp(wattageTransmitter2MAC, src_addr, 6) == 0) {
    wattageDeviceData receivedWattageData;
    memcpy(&receivedWattageData, incomingData, sizeof(receivedWattageData));
    int temporaryHash = calculateWattageDeviceHash(receivedWattageData);
    Serial.printf("Calculated Hash: %d / Received Hash: %d\n", temporaryHash, receivedWattageData.hash);
    Serial.print("Data received: ");
    Serial.println(len);
    if (temporaryHash == receivedWattageData.hash) {
      Serial.println("Data Received Successfully");
      memcpy(&uartData.volt_WattageDevice2, receivedWattageData.volt, sizeof(uartData.volt_WattageDevice2));
      memcpy(&uartData.curr_WattageDevice2, receivedWattageData.curr, sizeof(uartData.curr_WattageDevice2));
      memcpy(&uartData.real_p_WattageDevice2, receivedWattageData.real_p, sizeof(uartData.real_p_WattageDevice2));
      memcpy(&uartData.app_p_WattageDevice2, receivedWattageData.app_p, sizeof(uartData.app_p_WattageDevice2));
      memcpy(&uartData.max_volt_WattageDevice2, receivedWattageData.max_volt, sizeof(uartData.max_volt_WattageDevice2));
      memcpy(&uartData.max_curr_WattageDevice2, receivedWattageData.max_curr, sizeof(uartData.max_curr_WattageDevice2));
      memcpy(&uartData.max_real_WattageDevice2, receivedWattageData.max_real, sizeof(uartData.max_real_WattageDevice2));
      memcpy(&uartData.max_app_WattageDevice2, receivedWattageData.max_app, sizeof(uartData.max_app_WattageDevice2));
      memcpy(&uartData.pf_WattageDevice2, receivedWattageData.pf, sizeof(uartData.pf_WattageDevice2));
      memcpy(&uartData.imp_WattageDevice2, receivedWattageData.imp, sizeof(uartData.imp_WattageDevice2));
      memcpy(&uartData.min_imp_WattageDevice2, receivedWattageData.min_imp, sizeof(uartData.min_imp_WattageDevice2));
      memcpy(&uartData.max_imp_WattageDevice2, receivedWattageData.max_imp, sizeof(uartData.max_imp_WattageDevice2));
      memcpy(&uartData.freq_WattageDevice2, receivedWattageData.freq, sizeof(uartData.freq_WattageDevice2));
      memcpy(&uartData.svx_WattageDevice2, receivedWattageData.svx, sizeof(uartData.svx_WattageDevice2));
      memcpy(&uartData.clipping_WattageDevice2, receivedWattageData.clipping, sizeof(uartData.clipping_WattageDevice2));
      memcpy(&uartData.temperature_WattageDevice2, receivedWattageData.temperature, sizeof(uartData.temperature_WattageDevice2));
      memcpy(&uartData.maxTemp_WattageDevice2, receivedWattageData.maxTemp, sizeof(uartData.maxTemp_WattageDevice2));
      uartData.cvx_WattageDevice2 = receivedWattageData.cvx;
      uartData.hvx_WattageDevice2 = receivedWattageData.hvx;
      uartData.lvx_WattageDevice2 = receivedWattageData.lvx;
      uartData.unitC_WattageDevice2 = receivedWattageData.unitC;
      uartData.noOfSensors_WattageDevice2 = receivedWattageData.noOfSensors;
    Serial.printf("Volt: %.2f / %.2f\n", uartData.volt_WattageDevice2[0], uartData.volt_WattageDevice2[1]);
    Serial.printf("Current: %.2f / %.2f\n", uartData.curr_WattageDevice2[0], uartData.curr_WattageDevice2[1]);
    Serial.printf("Real Power: %.2f / %.2f\n", uartData.real_p_WattageDevice2[0], uartData.real_p_WattageDevice2[1]);
    Serial.printf("Apparant Power: %.2f / %.2f\n", uartData.app_p_WattageDevice2[0], uartData.app_p_WattageDevice2[1]);
    Serial.printf("Max Volt: %.2f / %.2f\n", uartData.max_volt_WattageDevice2[0], uartData.max_volt_WattageDevice2[1]);
    Serial.printf("MAx Current: %.2f / %.2f\n", uartData.max_curr_WattageDevice2[0], uartData.max_curr_WattageDevice2[1]);
    Serial.printf("Max Real Power: %.2f / %.2f\n", uartData.max_real_WattageDevice2[0], uartData.max_real_WattageDevice2[1]);
    Serial.printf("Max Apparant Power: %.2f / %.2f\n", uartData.max_app_WattageDevice2[0], uartData.max_app_WattageDevice2[1]);
    Serial.printf("Power Factor: %.2f / %.2f\n", uartData.pf_WattageDevice2[0], uartData.pf_WattageDevice2[1]);
    Serial.printf("Imp: %.2f / %.2f\n", uartData.imp_WattageDevice2[0], uartData.imp_WattageDevice2[1]);
    Serial.printf("Min Imp: %.2f / %.2f\n", uartData.min_imp_WattageDevice2[0], uartData.min_imp_WattageDevice2[1]);
    Serial.printf("Max Imp: %.2f / %.2f\n", uartData.max_imp_WattageDevice2[0], uartData.max_imp_WattageDevice2[1]);
    Serial.printf("Frequency: %.2f / %.2f\n", uartData.freq_WattageDevice2[0], uartData.freq_WattageDevice2[1]);
    Serial.printf("Starting Volt: %.2f / %.2f\n", uartData.svx_WattageDevice2[0], uartData.svx_WattageDevice2[1]);
    Serial.printf("Clipping: %.2f / %.2f\n", uartData.clipping_WattageDevice2[0], uartData.clipping_WattageDevice2[1]);
    Serial.print("Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.temperature_WattageDevice2[i]);
    }
    Serial.println();
    Serial.print("Maximum Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.maxTemp_WattageDevice2[i]);
    }
    Serial.println();
    Serial.printf("Unit_C: %d / Used_Sensors:  %d / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n", uartData.unitC_WattageDevice2, uartData.noOfSensors_WattageDevice2, uartData.cvx_WattageDevice2, uartData.hvx_WattageDevice2, uartData.lvx_WattageDevice2);
      sendDataByUart();
    } else {
      Serial.println("Data is Corrupted");
    }
  } else if (memcmp(wattageTransmitter3MAC, src_addr, 6) == 0) {
    wattageDeviceData receivedWattageData;
    memcpy(&receivedWattageData, incomingData, sizeof(receivedWattageData));
    int temporaryHash = calculateWattageDeviceHash(receivedWattageData);
    Serial.printf("Calculated Hash: %d / Received Hash: %d\n", temporaryHash, receivedWattageData.hash);
    Serial.print("Data received: ");
    Serial.println(len);
    if (temporaryHash == receivedWattageData.hash) {
      Serial.println("Data Received Successfully");
      memcpy(&uartData.volt_WattageDevice3, receivedWattageData.volt, sizeof(uartData.volt_WattageDevice3));
      memcpy(&uartData.curr_WattageDevice3, receivedWattageData.curr, sizeof(uartData.curr_WattageDevice3));
      memcpy(&uartData.real_p_WattageDevice3, receivedWattageData.real_p, sizeof(uartData.real_p_WattageDevice3));
      memcpy(&uartData.app_p_WattageDevice3, receivedWattageData.app_p, sizeof(uartData.app_p_WattageDevice3));
      memcpy(&uartData.max_volt_WattageDevice3, receivedWattageData.max_volt, sizeof(uartData.max_volt_WattageDevice3));
      memcpy(&uartData.max_curr_WattageDevice3, receivedWattageData.max_curr, sizeof(uartData.max_curr_WattageDevice3));
      memcpy(&uartData.max_real_WattageDevice3, receivedWattageData.max_real, sizeof(uartData.max_real_WattageDevice3));
      memcpy(&uartData.max_app_WattageDevice3, receivedWattageData.max_app, sizeof(uartData.max_app_WattageDevice3));
      memcpy(&uartData.pf_WattageDevice3, receivedWattageData.pf, sizeof(uartData.pf_WattageDevice3));
      memcpy(&uartData.imp_WattageDevice3, receivedWattageData.imp, sizeof(uartData.imp_WattageDevice3));
      memcpy(&uartData.min_imp_WattageDevice3, receivedWattageData.min_imp, sizeof(uartData.min_imp_WattageDevice3));
      memcpy(&uartData.max_imp_WattageDevice3, receivedWattageData.max_imp, sizeof(uartData.max_imp_WattageDevice3));
      memcpy(&uartData.freq_WattageDevice3, receivedWattageData.freq, sizeof(uartData.freq_WattageDevice3));
      memcpy(&uartData.svx_WattageDevice3, receivedWattageData.svx, sizeof(uartData.svx_WattageDevice3));
      memcpy(&uartData.clipping_WattageDevice3, receivedWattageData.clipping, sizeof(uartData.clipping_WattageDevice3));
      memcpy(&uartData.temperature_WattageDevice3, receivedWattageData.temperature, sizeof(uartData.temperature_WattageDevice3));
      memcpy(&uartData.maxTemp_WattageDevice3, receivedWattageData.maxTemp, sizeof(uartData.maxTemp_WattageDevice3));
      uartData.cvx_WattageDevice3 = receivedWattageData.cvx;
      uartData.hvx_WattageDevice3 = receivedWattageData.hvx;
      uartData.lvx_WattageDevice3 = receivedWattageData.lvx;
      uartData.unitC_WattageDevice3 = receivedWattageData.unitC;
      uartData.noOfSensors_WattageDevice3 = receivedWattageData.noOfSensors;
      Serial.printf("Volt: %.2f / %.2f\n", uartData.volt_WattageDevice3[0], uartData.volt_WattageDevice3[1]);
    Serial.printf("Current: %.2f / %.2f\n", uartData.curr_WattageDevice3[0], uartData.curr_WattageDevice3[1]);
    Serial.printf("Real Power: %.2f / %.2f\n", uartData.real_p_WattageDevice3[0], uartData.real_p_WattageDevice3[1]);
    Serial.printf("Apparant Power: %.2f / %.2f\n", uartData.app_p_WattageDevice3[0], uartData.app_p_WattageDevice3[1]);
    Serial.printf("Max Volt: %.2f / %.2f\n", uartData.max_volt_WattageDevice3[0], uartData.max_volt_WattageDevice3[1]);
    Serial.printf("MAx Current: %.2f / %.2f\n", uartData.max_curr_WattageDevice3[0], uartData.max_curr_WattageDevice3[1]);
    Serial.printf("Max Real Power: %.2f / %.2f\n", uartData.max_real_WattageDevice3[0], uartData.max_real_WattageDevice3[1]);
    Serial.printf("Max Apparant Power: %.2f / %.2f\n", uartData.max_app_WattageDevice3[0], uartData.max_app_WattageDevice3[1]);
    Serial.printf("Power Factor: %.2f / %.2f\n", uartData.pf_WattageDevice3[0], uartData.pf_WattageDevice3[1]);
    Serial.printf("Imp: %.2f / %.2f\n", uartData.imp_WattageDevice3[0], uartData.imp_WattageDevice3[1]);
    Serial.printf("Min Imp: %.2f / %.2f\n", uartData.min_imp_WattageDevice3[0], uartData.min_imp_WattageDevice3[1]);
    Serial.printf("Max Imp: %.2f / %.2f\n", uartData.max_imp_WattageDevice3[0], uartData.max_imp_WattageDevice3[1]);
    Serial.printf("Frequency: %.2f / %.2f\n", uartData.freq_WattageDevice3[0], uartData.freq_WattageDevice3[1]);
    Serial.printf("Starting Volt: %.2f / %.2f\n", uartData.svx_WattageDevice3[0], uartData.svx_WattageDevice3[1]);
    Serial.printf("Clipping: %.2f / %.2f\n", uartData.clipping_WattageDevice3[0], uartData.clipping_WattageDevice3[1]);
    Serial.print("Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.temperature_WattageDevice3[i]);
    }
    Serial.println();
    Serial.print("Maximum Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.maxTemp_WattageDevice3[i]);
    }
    Serial.println();
    Serial.printf("Unit_C: %d / Used_Sensors:  %d / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n", uartData.unitC_WattageDevice3, uartData.noOfSensors_WattageDevice3, uartData.cvx_WattageDevice3, uartData.hvx_WattageDevice3, uartData.lvx_WattageDevice3);
    sendDataByUart();
    } else {
      Serial.println("Data is Corrupted");
    }
  } else if (memcmp(wattageTransmitter4MAC, src_addr, 6) == 0) {
    wattageDeviceData receivedWattageData;
    memcpy(&receivedWattageData, incomingData, sizeof(receivedWattageData));
    int temporaryHash = calculateWattageDeviceHash(receivedWattageData);
    Serial.printf("Calculated Hash: %d / Received Hash: %d\n", temporaryHash, receivedWattageData.hash);
    Serial.print("Data received: ");
    Serial.println(len);
    if (temporaryHash == receivedWattageData.hash) {
      Serial.println("Data Received Successfully");
      memcpy(&uartData.volt_WattageDevice4, receivedWattageData.volt, sizeof(uartData.volt_WattageDevice4));
      memcpy(&uartData.curr_WattageDevice4, receivedWattageData.curr, sizeof(uartData.curr_WattageDevice4));
      memcpy(&uartData.real_p_WattageDevice4, receivedWattageData.real_p, sizeof(uartData.real_p_WattageDevice4));
      memcpy(&uartData.app_p_WattageDevice4, receivedWattageData.app_p, sizeof(uartData.app_p_WattageDevice4));
      memcpy(&uartData.max_volt_WattageDevice4, receivedWattageData.max_volt, sizeof(uartData.max_volt_WattageDevice4));
      memcpy(&uartData.max_curr_WattageDevice4, receivedWattageData.max_curr, sizeof(uartData.max_curr_WattageDevice4));
      memcpy(&uartData.max_real_WattageDevice4, receivedWattageData.max_real, sizeof(uartData.max_real_WattageDevice4));
      memcpy(&uartData.max_app_WattageDevice4, receivedWattageData.max_app, sizeof(uartData.max_app_WattageDevice4));
      memcpy(&uartData.pf_WattageDevice4, receivedWattageData.pf, sizeof(uartData.pf_WattageDevice4));
      memcpy(&uartData.imp_WattageDevice4, receivedWattageData.imp, sizeof(uartData.imp_WattageDevice4));
      memcpy(&uartData.min_imp_WattageDevice4, receivedWattageData.min_imp, sizeof(uartData.min_imp_WattageDevice4));
      memcpy(&uartData.max_imp_WattageDevice4, receivedWattageData.max_imp, sizeof(uartData.max_imp_WattageDevice4));
      memcpy(&uartData.freq_WattageDevice4, receivedWattageData.freq, sizeof(uartData.freq_WattageDevice4));
      memcpy(&uartData.svx_WattageDevice4, receivedWattageData.svx, sizeof(uartData.svx_WattageDevice4));
      memcpy(&uartData.clipping_WattageDevice4, receivedWattageData.clipping, sizeof(uartData.clipping_WattageDevice4));
      memcpy(&uartData.temperature_WattageDevice4, receivedWattageData.temperature, sizeof(uartData.temperature_WattageDevice4));
      memcpy(&uartData.maxTemp_WattageDevice4, receivedWattageData.maxTemp, sizeof(uartData.maxTemp_WattageDevice4));
      uartData.cvx_WattageDevice4 = receivedWattageData.cvx;
      uartData.hvx_WattageDevice4 = receivedWattageData.hvx;
      uartData.lvx_WattageDevice4 = receivedWattageData.lvx;
      uartData.unitC_WattageDevice4 = receivedWattageData.unitC;
      uartData.noOfSensors_WattageDevice4 = receivedWattageData.noOfSensors;
      Serial.printf("Volt: %.2f / %.2f\n", uartData.volt_WattageDevice4[0], uartData.volt_WattageDevice4[1]);
    Serial.printf("Current: %.2f / %.2f\n", uartData.curr_WattageDevice4[0], uartData.curr_WattageDevice4[1]);
    Serial.printf("Real Power: %.2f / %.2f\n", uartData.real_p_WattageDevice4[0], uartData.real_p_WattageDevice4[1]);
    Serial.printf("Apparant Power: %.2f / %.2f\n", uartData.app_p_WattageDevice4[0], uartData.app_p_WattageDevice4[1]);
    Serial.printf("Max Volt: %.2f / %.2f\n", uartData.max_volt_WattageDevice4[0], uartData.max_volt_WattageDevice4[1]);
    Serial.printf("MAx Current: %.2f / %.2f\n", uartData.max_curr_WattageDevice4[0], uartData.max_curr_WattageDevice4[1]);
    Serial.printf("Max Real Power: %.2f / %.2f\n", uartData.max_real_WattageDevice4[0], uartData.max_real_WattageDevice4[1]);
    Serial.printf("Max Apparant Power: %.2f / %.2f\n", uartData.max_app_WattageDevice4[0], uartData.max_app_WattageDevice4[1]);
    Serial.printf("Power Factor: %.2f / %.2f\n", uartData.pf_WattageDevice4[0], uartData.pf_WattageDevice4[1]);
    Serial.printf("Imp: %.2f / %.2f\n", uartData.imp_WattageDevice4[0], uartData.imp_WattageDevice4[1]);
    Serial.printf("Min Imp: %.2f / %.2f\n", uartData.min_imp_WattageDevice4[0], uartData.min_imp_WattageDevice4[1]);
    Serial.printf("Max Imp: %.2f / %.2f\n", uartData.max_imp_WattageDevice4[0], uartData.max_imp_WattageDevice4[1]);
    Serial.printf("Frequency: %.2f / %.2f\n", uartData.freq_WattageDevice4[0], uartData.freq_WattageDevice4[1]);
    Serial.printf("Starting Volt: %.2f / %.2f\n", uartData.svx_WattageDevice4[0], uartData.svx_WattageDevice4[1]);
    Serial.printf("Clipping: %.2f / %.2f\n", uartData.clipping_WattageDevice4[0], uartData.clipping_WattageDevice4[1]);
    Serial.print("Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.temperature_WattageDevice4[i]);
    }
    Serial.println();
    Serial.print("Maximum Temperature: ");
    for (int i = 0; i < 4; i++) {
      Serial.printf("%.2f / ", uartData.maxTemp_WattageDevice4[i]);
    }
    Serial.println();
    Serial.printf("Unit_C: %d / Used_Sensors:  %d / Instantaneous_volts: %.2f / Max_volts: %.2f / Min_volts: %.2f\n", uartData.unitC_WattageDevice4, uartData.noOfSensors_WattageDevice4, uartData.cvx_WattageDevice4, uartData.hvx_WattageDevice4, uartData.lvx_WattageDevice4);
    sendDataByUart();
    } else {
      Serial.println("Data is Corrupted");
    }
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

void sendDataByUart() {
  uint8_t buffer[2 + sizeof(uartSendingData)];
  Serial.println(sizeof(buffer));
  buffer[0] = 0xAA;
  buffer[1] = 0x15;
  memcpy(&buffer[2], &uartData, sizeof(uartSendingData));
  //Send over UART
  Serial.write(buffer, sizeof(buffer));
}

void setup() {
  Serial.begin(115200);
  esp_init();
}

void loop() {

}