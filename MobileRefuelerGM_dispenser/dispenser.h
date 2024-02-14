#include <RunningAverage.h>
#include <LedHelper.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h> //For I2C comms
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_LiquidCrystal.h"
#include <ArduinoRS485.h>   //ArduinoModbus depends on the ArduinoRS485 lib
#include <ArduinoModbus.h>  //for modbus tcp/ip
#include <ModbusMaster_oneH2.h>

#include "I2C_API.h"

#define ESTOP_BREAK 40
#define LED_PWR 22
#define TRACO_24VDC 23

#define SS 10    //W5500 CS
#define RST 7    //W5500 RST
#define CS 4     //SD CS pin

#define PT700 A13
#define PT350 A12
#define PT50 A10

#define H2SNIFFER 0
#define RE_DE1 12
#define RE_DE2 31
#define SS 10    //W5500 CS
#define RST 7    //W5500 RST
#define CS 4     //SD CS pin
#define NUM_EXP 4

volatile int CARR_MOVING_AVG_SIZE  = 0;
volatile int DISP_MOVING_AVG_SIZE  = 10;     // Number of data points to average together to smooth pressure readings
volatile int DELTA_PSI_TIME           = 20000;
volatile int MIN_ALLOWED_PRESSURE     = 1000;
volatile int STALL_THRESHOLD          = 55;
volatile int FILL_TIMEOUT            = 60;

volatile int  H2_SENSOR_LIMIT      = 285;    // Maximum ambient h2 sensor readings before detecting H2 leak
volatile int  SECOND_CHECK = 75;
volatile int  PRESSURE_LOSS_TOL    = 500;    // The allowed amount of pressure drop over 3 seconds while checking for leaks before declaring a leak
volatile int  TARGET_PRESSURE_350BAR      = 5000;
volatile int  TARGET_PRESSURE_700BAR      = 10000;


ModbusMaster mbRTU;
EthernetClient dead;
EthernetClient ethClient1;
EthernetClient ethClient2;
EthernetClient ethClient3;
ModbusTCPClient modbusTCPClient1(ethClient1);
ModbusTCPClient modbusTCPClient2(ethClient2);
ModbusTCPClient modbusTCPClient3(ethClient3);

byte mac[] = { 0x3D, 0x61, 0x1C, 0xC5, 0xB1, 0x68 };

IPAddress ip[] = {
  { 192, 168, 127, 100 },
  { 192, 168, 127, 101 },
  { 192, 168, 127, 102 },
  { 192, 168, 127, 163 } //HMI
};

enum state{
  FAULT,
  IDLE_OFF,
  INTEGRITY,
  FILL85,
  FILL100,
  FLOW,
  SHUTDOWN
}STATE, PREV_STATE;

struct bank {
  uint16_t pressure;
  int ID;
  bool onOff;
  bool allowed;
};
bank banks[] = {
  { 0, 1, false, true },
  { 0, 2, false, true },
  { 0, 3, false, true }
};

int prevBank = 0;
int numBanks = 3;

int printCnt = 0;
//uint16_t prevPsi[4] = {0};
int rtuFailCnt[3] = {0};
//int stallCnt = 0;
unsigned long timer[15] = {0};
unsigned long stallTimer[5] = {0};
unsigned long checkTimer[10] = {0};
unsigned long debouncer0 = 0;
unsigned long debouncer1 = 0;
uint16_t maxCarr = 0;
int remoteActiveBank = 0;
double initPress = 0;
int count = 0;
double deltaPsi = 0;
int psiCnt = 0;
int tcpFailCnt[9] = {0};
uint16_t TCPfail = 0;
bool DO_Disp_XV50_LowPressure = false;
bool DO_Disp_XV350_MediumPressure = false;
bool DO_Disp_XV700_HighPressure = false;
bool DO_Disp_XV700_Vent = false;
bool DO_Disp_LED01_LampGreen = false;
bool DO_Disp_LED02_LampAmber = false;
bool DO_Disp_LED03_LampRed = false;
bool DO_Comm_H2DemandLocal = false;

bool DI_Disp_SW01_ButtonGreen = false;
bool DI_Disp_SW03_ButtonRed = false;
bool DI_Comm_H2DemandRemote = false;
bool DI_Disp_ASL_GSR = false;
bool DI_Disp_ASL_LSR = false;
bool DI_Disp_ASL_CarrierOK = false;

double AI_Disp_psig_PT50_GeneratorModePress = 0;
double AI_Disp_psig_PT350_LowModePress = 0;
double AI_Disp_psig_PT700_HighModePress = 0;
double AI_Carr_psig_PT100_350Bar_Bank1Press = 0;
double AI_Carr_psig_PT200_350Bar_Bank2Press = 0;
double AI_Carr_psig_PT300_700Bar_Bank3Press = 0;
double AI_Carr_psig_PT400_700Bar_ManifoldPress = 0;


bool carrierFinished = false;
bool virtualGreenButton = false;
bool virtualRedButton = false;
bool debouncedGreenButton = false;
bool debouncedRedButton = false;


double displayPage = 0;
double bank1Pt = 0;
double bank2Pt = 0;
double bank3Pt = 0;


uint16_t pressureSetting = 0;

uint16_t errors[20] = {0};
int errCnt = 0;
bool tog[10] = {false};
bool flashAmber = false;
int activeBank = 0;

int           initPressure   = 0;
uint16_t      finalPressure  = 0;

uint16_t      pt50 = 0;
uint16_t      pt350 = 0;
uint16_t      pt700 = 0;
unsigned long fillTime = 0;
uint16_t      readByte = 0;
uint16_t      sendByte1 = 0;
uint16_t      sendByte2 = 0;

uint16_t      displayState = 0;
uint16_t      targetOverride = 0;
uint16_t      targetPressure = 0;
uint16_t      prevPressure = 0;
uint16_t      currentPressure = 0;

uint16_t manPt = 0;

int manCnt = 0;
int H2cnt = 0;
int hmiCnt = 0;
bool H2Leak  = false;