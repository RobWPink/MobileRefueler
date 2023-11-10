#include "carrierSlave.h"
SmallMatrix smallMatrix[3] = {SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = {LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

RunningAverage bank1RA(MOVING_AVG_SIZE);
RunningAverage bank2RA(MOVING_AVG_SIZE);
RunningAverage man1RA(MOVING_AVG_SIZE);

void (*resetFunc)(void) = 0;  // Allows hard reset of arduino through software

void setup() {
  TCCR5B = (TCCR5B & 0xF8) | 0x01;
  Wire.begin();
  Wire.setWireTimeout(10000, true);
  pinModeSetup();
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1);  //local
  Serial3.begin(9600, SERIAL_8N1);  //remote
  matrixSetup("SmartCarrier", "NAV V4.1.1S");
  mb.begin(&Serial3,RE_DE3);
  mb.setBaudrate(9600);
  mb.slave(3);
  mb.addHreg(0,0,15);
  
  for (int i = 0; i < MOVING_AVG_SIZE; i++){
    bank1RA.addValue((analogRead(ACT1_PRESS) - 200.0) / 0.084); // Grab pressure sensor values and convert to psi
    bank2RA.addValue((analogRead(ACT2_PRESS) - 200.0) / 0.084); // Add pressure readings to moving average to smooth out data
    man1RA.addValue((analogRead(ACT3_PRESS) - 200.0) / 0.084);
  }
  Serial.println("OK");
}

void loop() {

  dataRead(1000);
  daughterPrint(250);
  SerialCLI();

  mb.Hreg(0,bank3Pt);
  mb.Hreg(1,bank4Pt);
  mb.Hreg(2,man2Pt);
  mb.Hreg(3,localActiveBank);
  mb.Hreg(4,overrideBank);

  mb.task();
  yield();
  
  activeBank = mb.Hreg(10);
  bank1Pt = mb.Hreg(11);
  bank2Pt = mb.Hreg(12);
  man1Pt = mb.Hreg(13);
  vehPsi = mb.Hreg(14);
  int temp = mb.Hreg(15);
  if(temp){MOVING_AVG_SIZE = temp;}

  if(activeBank){Serial.println("Master Carrier has overriden Bank control. Please wait until Master is finished"); overrideBank = 0;}

  localActiveBank = activeBank;
  
  if(localActiveBank == 3 || overrideBank == 3){digitalWrite(RLY2, HIGH); digitalWrite(RLY3, LOW);}
  else if(localActiveBank == 4 || overrideBank == 4){digitalWrite(RLY2, LOW); digitalWrite(RLY3, HIGH);}
  else{digitalWrite(RLY2, LOW); digitalWrite(RLY3, LOW);}
}
