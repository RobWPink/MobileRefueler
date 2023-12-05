#include <Wire.h>  //For I2C comm
#include <Adafruit_GFX.h>
#include <RunningAverage.h>
#include <LedHelper.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_LiquidCrystal.h>
#include <ModbusRTU.h>

ModbusRTU mb;
int MOVING_AVG_SIZE = 10;
struct bank {
  uint16_t pressure;
  int ID;
  bool onOff;
};

bank banks[] = {
  { 0, 1, false },
  { 0, 2, false },
  { 0, 3, false }
};
bool manual = false;
unsigned long timer[10] = {0};
bool tog[10] = {0};
bool dmd = false;

int vehPsi = 0;
uint16_t bank1Pt = 0;
uint16_t bank2Pt = 0;
uint16_t bank3Pt = 0;
uint16_t manPt = 0;

int activeBank = 0;
bool tg[3] = {false};

#define SOLA_PRESS A10
#define SOLB_PRESS A12
#define SOLC_PRESS A7
#define MAN_PRESS A13

#define RLY2 43 //520BAR SOL A
#define RLY3 41 //520BAR SOL B
#define RLY4 49 //930BAR SOL C
#define SS 10  //W5500 CS
#define RST 7  //W5500 RST

#define DEMAND 45
#define OK 47
#define ESTOP_BREAK 40
#define LED_PWR 22
#define TRACO_24VDC 23

#define RE_DE1 12
#define RE_DE2 31