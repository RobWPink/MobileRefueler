#include <Wire.h>  //For I2C comm
#include <Adafruit_GFX.h>
#include <RunningAverage.h>
#include <LedHelper.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

byte mac[] = {0xDE, 0xBD, 0x3E, 0xEA, 0x1E, 0xED};
IPAddress localIP(192, 168, 1, 11);

EthernetServer ethServer(502);

ModbusTCPServer slave;
volatile int MOVING_AVG_SIZE = 10;

unsigned long timer[10] = {0};
unsigned long daughterTimer = 0;
unsigned long dataTimer = 0;
bool tog[10] = {0};
bool dmd = false;

bool q = false;

uint16_t bank1Pt = 0;
uint16_t bank2Pt = 0;
uint16_t bank3Pt = 0;
uint16_t bank4Pt = 0;
uint16_t man1Pt = 0;
uint16_t man2Pt = 0;

int overrideBank = 0;
int activeBank = 0;
int localActiveBank = 0;
double vehPsi = 0;

#define ACT1_PRESS A10
#define ACT1_POS A9
#define ACT1_CLOCK 38
#define ACT1_CTRL 29
#define ACT1_CW 25
#define ACT1_ENABLE 34
#define ACT1_HALF 32
#define ACT1_HOME 36
#define ACT1_RESET 27
#define ACT1_VREF 44

#define ACT2_PRESS A12
#define ACT2_POS A11
#define ACT2_CLOCK 39
#define ACT2_CTRL 28
#define ACT2_CW 24
#define ACT2_ENABLE 35
#define ACT2_HALF 33
#define ACT2_HOME 37
#define ACT2_RESET 26
#define ACT2_VREF 46

#define ACT3_PRESS A13
#define ACT3_POS 
#define ACT3_CLOCK 5
#define ACT3_CTRL 4
#define ACT3_CW 2
#define ACT3_ENABLE 9
#define ACT3_HALF 8
#define ACT3_HOME 11
#define ACT3_RESET 3
#define ACT3_VREF 6

//#define RLY0 47
//#define RLY1 45
#define RLY2 43
#define RLY3 41
#define RLY4 49
#define RLY5 48
#define RLY6
#define RLY7
#define RLY8
#define RLY9
#define RLY10
#define RLY11

#define SS 10  //W5500 CS
#define RST 7  //W5500 RST

#define DEMAND 45
#define OK 47
#define ESTOP_BREAK 40
#define LED_PWR 22
#define TRACO_24VDC 23

#define RE_DE1 12
#define RE_DE3 31

#define I2C_ADC1 0x49
#define I2C_ADC2 0x4B
//#define I2C_DAC 0x60
#define I2C_IOEXP 0x60
#define I2C_IoT 0x48
#define I2C_INERT 0x68
#define I2C_RTC 0x51
#define I2C_ESP32 0x69  // configurable