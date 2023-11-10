
#include <RunningAverage.h>
#include <LedHelper.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h> //For I2C comms
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_LiquidCrystal.h"
#include <ArduinoModbus.h>  //for modbus tcp/ip
#include <ModbusRTU.h>

ModbusRTU mb;

enum state{
  FAULT,
  OUT_OF_ORDER,
  IDLE_OFF,
  SPIKE,
  FILL,
  FINISHED
}STATE, PREV_STATE;

#define ESTOP_BREAK 40
#define LED_PWR 22
#define TRACO_24VDC 23

#define SS 10    //W5500 CS
#define RST 7    //W5500 RST
#define CS 4     //SD CS pin

#define PT A10
#define H2SNIFFER 0

#define SS 10    //W5500 CS
#define RST 7    //W5500 RST
#define CS 4     //SD CS pin

#define GB 0
#define RB 0
#define LSR 0
#define GSR 0
#define DMD 0
#define OK 0

#define VALVE 0
#define GL 0
#define AL 0
#define RL 0


volatile int  MOVING_AVG_SIZE      = 10;     // Number of data points to average together to smooth pressure readings
volatile int TIME_OUT = 5;
unsigned long flashTimer1 = 0;
unsigned long flashTimer2 = 0;
unsigned long daughterTimer = 0;

byte mac[] = {0xDE, 0xA1, 0xBE, 0xEF, 0xFA, 0xED};

IPAddress HMIip(192, 168, 127, 163);
IPAddress localIP(192, 168, 127, 20);

EthernetClient hmi;
ModbusTCPClient HMI(hmi);


unsigned long debouncer0 = 0;
unsigned long debouncer1 = 0;
unsigned long timer[13] = {0};
double HMI_targetPressure = 5000;
bool          tog[10] = {false};
bool          flashAmber     = false;
bool          flashRed     = false;
bool          virtualGreenButton = false;
bool          virtualRedButton = false;
bool greenButton = false;
bool redButton = false;
bool err_timeout = false;
double      initVehPress   = 0;
double      finalPressure  = 0;
double      targetPressure = 0;
double      currentPressure = 0;
bool oooOverride = false;
bool override = false;
uint16_t      readByte = 0;
uint16_t      sendByte = 0;
double      displayState = 0;
double      targetOverride = 0;
int modID = 0;
#define RE_DE1 12
#define RE_DE3 31
