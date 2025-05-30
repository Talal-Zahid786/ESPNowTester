#define debug

#define RECEIVER_MAC {0xC8, 0xF0, 0x9E, 0xB6, 0xAB, 0xC8}

#define SENSORS_USED 4

byte temperatureI2cAddress[16] = { 0x5A, 0x5B, 0x5C, 0x5D};

//#define USE_UNITS_C
#define USE_UNITS_F

#define DELAY 50