#include <RunningAverage.h>
#include <Wire.h>  //For I2C comms
#include <SPI.h>
#include <LedHelper.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_LiquidCrystal.h"

#define STATUS "STATUS"
#define ESTOP "ESTOP"
#define ONOFF "ONOFF"
#define TRAILERKEY "TRAILER"
#define MANIFOLDKEY "MANIFOLD"

#define tel_adr 0x03

#define LED_PWR 22
#define TRACO_24VDC 23
#define ESTOP_BREAK 40
#define STATUS 43
#define ESTOP 41
#define ONOFF 49

SmallMatrix smallMatrix[3] = { SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = { LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

RunningAverage Trailer(10);
RunningAverage Manifold(10);

union floatToBytes {
  char asBytes[4] = { 0 };
  float asFloat;
};

unsigned long timer = 0;
unsigned long timer2 = 0;
void setup() {
  Wire.begin();
  Serial.begin(9600);  //debug
  pinMode(ESTOP_BREAK, OUTPUT);
  pinMode(LED_PWR, OUTPUT);
  pinMode(TRACO_24VDC, OUTPUT);
  digitalWrite(ESTOP_BREAK, HIGH);
  digitalWrite(LED_PWR, HIGH);
  digitalWrite(TRACO_24VDC, HIGH);
  pinMode(STATUS, INPUT);
  pinMode(ESTOP, INPUT);
  pinMode(ONOFF, INPUT);
  matrixSetup("LORAM Trailer", "v2.0.0");
}

void loop() {
  if(!timer){timer = millis();}
  if(millis() - timer > 250 && timer){
    Trailer.addValue(analogRead(A12));
    Manifold.addValue(analogRead(A11));
    bigMatrix[0].writeInt((Trailer.getAverage() - 200.0) / 0.042);
    bigMatrix[1].writeInt((Manifold.getAverage() - 200.0) / 0.042);
    smallMatrix[0].squareDisplay(digitalRead(ONOFF) ? 15 : 0);
    smallMatrix[1].squareDisplay(digitalRead(STATUS) ? 15 : 0);
    smallMatrix[2].squareDisplay(digitalRead(ESTOP) ? 15 : 0);
    timer = 0;
  }
  if(!timer2){timer2 = millis();}
  if(millis() - timer2 > 1000) {
    timer2 = millis();
    telSetValue(tel_adr, ESTOP, digitalRead(ESTOP));
    telSetValue(tel_adr, STATUS, digitalRead(STATUS));
    telSetValue(tel_adr, ONOFF, digitalRead(ONOFF));
    telSetValue(tel_adr, TRAILERKEY, (Trailer.getAverage() - 200.0) / 0.042);
    telSetValue(tel_adr, MANIFOLDKEY, (Manifold.getAverage() - 200.0) / 0.042);
  }
}
void matrixSetup(String s1, String s2) {
  //begin matrices
  for (int i = 0; i < 3; i++) {
    smallMatrix[i].begin();
    bigMatrix[i].begin();
  }
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(s1);
  lcd.setCursor(0, 1);
  lcd.print(s2);
  lcd.setBacklight(HIGH);
}

bool telSetValue(uint8_t telAddr, const char* dsKey, float dsVal) {
  char targetKey[6];
  strcpy(targetKey, dsKey);
  targetKey[5] = '\0';
  telWriteValue(telAddr, targetKey, dsVal);
  Wire.requestFrom((int)telAddr, 10);
  //-------------------------------------------//
  //-----------Verify Command Byte-------------//
  uint8_t cmdByte = Wire.read();
  if (cmdByte != 0xFE) {
    return false;
  }
  //-------------------------------------------//
  //--------Verify Correct Key was Set---------//
  char replyKey[6];
  for (int i = 0; i < 5; ++i) {
    replyKey[i] = Wire.read();
  }
  replyKey[5] = '\0';
  if (!strstr(replyKey, targetKey)) {
    return false;
  }
  //-------------------------------------------//
  //-------Verify Correct Value was Set--------//
  floatToBytes value;
  for (int i = 0; i < 5; ++i) {
    value.asBytes[i] = Wire.read();
  }
  return (dsVal == value.asFloat);
  //-------------------------------------------//
  //-------------------------------------------//
}

float telGetValue(uint8_t telAddr, const char* dsKey) {
  char targetKey[6];
  strcpy(targetKey, dsKey);
  targetKey[5] = '\0';
  Wire.beginTransmission(telAddr);        //Address of Uno used for telemetry
  Wire.write(0xFF);                             //Indicates a WRITE operation
  Wire.write(dsKey);                            //Coorelates to a Storage key
  Wire.endTransmission();  
  Wire.requestFrom((int)telAddr, 10);           //All data should be shared as four bytes
  //-------------------------------------------//
  //-----------Verify Command Byte-------------//
  uint8_t cmdByte = Wire.read();
  if (cmdByte != 0xFF) {
    return -254.0;
  }
  //-------------------------------------------//
  //--------Verify Correct Key was Got---------//
  char replyKey[6];
  for (int i = 0; i < 5; ++i) {
    replyKey[i] = Wire.read();
  }
  replyKey[5] = '\0';
  if (!strstr(replyKey, targetKey)) {
    return -254;
  }
  //-------------------------------------------//
  //-------------Retrieve Value----------------//
  floatToBytes value;
  for (int i = 0; i < 5; ++i) {
    value.asBytes[i] = Wire.read();
  }
  return value.asFloat;
}

int telWriteValue(uint8_t telAddr, char* dsKey, float dsVal) {
  floatToBytes value;
  value.asFloat = dsVal;

  Wire.beginTransmission(telAddr);              //Address of Uno used for telemetry
  Wire.write(0xFE);                             //Indicates a WRITE operation
  Wire.write(dsKey);                            //Coorelates to a Storage key
  Wire.write(value.asBytes[0]);                 //first byte of the value being sent
  Wire.write(value.asBytes[1]);                 //second byte of the value being sent
  Wire.write(value.asBytes[2]);                 //third byte of the value being sent
  Wire.write(value.asBytes[3]);                 //fourth byte of the value being sent
  return Wire.endTransmission();                       //Send message
}