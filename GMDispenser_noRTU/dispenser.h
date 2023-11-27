
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

#define GL 47
#define AL 45
#define RL 43
#define RLY3 41
#define VALVE 49 //DMD
#define RLY5 48

#define RB 13
#define LSR 42
#define NA 44
#define GB 46

int displayState = 0;
volatile int  MOVING_AVG_SIZE      = 10;     // Number of data points to average together to smooth pressure readings
volatile int TIME_OUT = 120;
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