#include "carrier.h"
SmallMatrix smallMatrix[3] = {SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = {LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

RunningAverage bank1RA(MOVING_AVG_SIZE);
RunningAverage bank2RA(MOVING_AVG_SIZE);
RunningAverage bank3RA(MOVING_AVG_SIZE);
RunningAverage manRA(MOVING_AVG_SIZE);

void (*resetFunc)(void) = 0;  // Allows hard reset of arduino through software

void setup() {
  TCCR5B = (TCCR5B & 0xF8) | 0x01;
  Wire.begin();
  Wire.setWireTimeout(10000, true);
  pinModeSetup();
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1);  //local
  Serial3.begin(9600, SERIAL_8N1);  //remote
  matrixSetup("Mobile Refueler", "SlaveCarrier v11");
  mb.begin(&Serial3,RE_DE2);
  mb.setBaudrate(9600);
  mb.slave(3);
  mb.addHreg(0,0,20);
  
  for (int i = 0; i < MOVING_AVG_SIZE; i++){
    bank1RA.addValue((analogRead(SOLA_PRESS) - 200.0) / 0.084); // Grab pressure sensor values and convert to psi
    bank2RA.addValue((analogRead(SOLB_PRESS) - 200.0) / 0.084); // Add pressure readings to moving average to smooth out data
    bank3RA.addValue(2*(analogRead(SOLC_PRESS) - 200.0) / 0.084);
    manRA.addValue((analogRead(MAN_PRESS) - 200.0) / 0.084);
  }
  Serial.println("OK");
}

void loop() {
  SerialCLI();

  if (digitalRead(DEMAND)) {
    if (!timer[0]) { timer[0] = millis(); }
    if (millis() - timer[0] >= 500 && timer[0]) {
      dmd = true;
    } else {
      dmd = false;
    }
  } else {
    timer[0] = 0;
    dmd = false;
  }

  if(!timer[4]){ timer[4] = millis(); }
  if(millis() - timer[4] > 250){
    long aa = (analogRead(SOLA_PRESS) - 200.0) / 0.084;
    long bb = (analogRead(SOLB_PRESS) - 200.0) / 0.084;
    long cc = 2*(analogRead(SOLC_PRESS) - 200.0) / 0.084; //930bar
    long dd = (analogRead(MAN_PRESS) - 200.0) / 0.084; 
    if (10 < aa < 15000) { bank1RA.addValue(aa);} // Grab pressure sensor values and convert to psi
    if (10 < bb < 15000) { bank2RA.addValue(bb);} // Add pressure readings to moving average to smooth out data
    if (10 < cc < 15000) { bank3RA.addValue(cc);} //930bar
    if (10 < dd < 15000) { manRA.addValue(dd);}
    bank1Pt = bank1RA.getAverage();
    bank2Pt = bank2RA.getAverage();
    bank3Pt = bank3RA.getAverage();
    manPt = manRA.getAverage();
  }

  if(!timer[2]){timer[2] = millis();}
  if(millis() - timer[2] >= 500 && timer[2]){
    bigMatrix[0].writeInt(bank1Pt); // Write local pressure values to 7-seg screen
    bigMatrix[1].writeInt(bank2Pt);

    smallMatrix[0].displayChar('1',(activeBank == 1)?1:0);
    smallMatrix[1].displayChar('2',(activeBank == 2)?1:0);
    if(!timer[5]){timer[5] = millis();}
    if(millis() - timer[5] > 3000 && timer[5]){tog[2] = !tog[2]; timer[5] = 0;}
    if(!tog[2]){
      smallMatrix[2].displayChar('3',(activeBank == 3)?1:0);
      bigMatrix[2].writeInt(bank3Pt);
    }
    else{
      smallMatrix[2].displayChar('M', (dmd) ? 1 : 0);
      bigMatrix[2].writeInt(manPt);
    }
    lcd.setCursor(12, 1);
    lcd.print(vehPsi);
    lcd.display();
    timer[2] = 0;
  }
  printOuts();
  mb.Hreg(0,bank1Pt); // Store moving average into registers
  mb.Hreg(1,bank2Pt);
  mb.Hreg(2,bank3Pt);
  mb.Hreg(3,manPt);
  mb.Hreg(4,activeBank);

  mb.task();
  yield();

  activeBank = mb.Hreg(10);
  vehPsi = mb.Hreg(11);
  if(!manual){
  if(activeBank == 1){
    digitalWrite(RLY2, HIGH);
    digitalWrite(RLY3, LOW);
    digitalWrite(RLY4, LOW);
  }
  else if(activeBank == 2){
    digitalWrite(RLY2, LOW);
    digitalWrite(RLY3, HIGH);
    digitalWrite(RLY4, LOW);
  }
  else if(activeBank == 3){
    digitalWrite(RLY2, LOW);
    digitalWrite(RLY3, LOW);
    digitalWrite(RLY4, HIGH);
  }
  else{
    digitalWrite(RLY2, LOW);
    digitalWrite(RLY3, LOW);
    digitalWrite(RLY4, LOW);
  }}
}

void matrixSetup(String s1, String s2) {
  //begin matrix
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
  lcd.display();
}

void pinModeSetup() {
  // Inputs
  pinMode(DEMAND, INPUT);

  pinMode(ESTOP_BREAK, OUTPUT);
  digitalWrite(ESTOP_BREAK, HIGH);

  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  pinMode(TRACO_24VDC, OUTPUT);
  digitalWrite(TRACO_24VDC, HIGH);

  pinMode(RLY2,OUTPUT);
  digitalWrite(RLY2, LOW);

  pinMode(RLY3,OUTPUT);
  digitalWrite(RLY3, LOW);

  pinMode(RLY4,OUTPUT);
  digitalWrite(RLY4, LOW);
}
