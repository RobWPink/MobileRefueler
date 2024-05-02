#include "dispenser.h"

RunningAverage vehiclePT(MOVING_AVG_SIZE);
SmallMatrix smallMatrix[3] = { SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = { LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);


void (*resetFunc)(void) = 0;  // Allows hard reset of arduino through software

//Slave ID's are: 1-50 Carriers, 51-100 dispensers
void setup() {
  TCCR5B = (TCCR5B & 0xF8) | 0x01;
  Wire.setWireTimeout(10000, true);
  pinModeSetup();
  Serial.begin(9600);/*
 //################## Ethernet Setup ##################
  Ethernet.init(10);
  Ethernet.begin(mac, localIP);
  delay(5);
  if (Ethernet.linkStatus() == LinkOFF || Ethernet.hardwareStatus() == EthernetNoHardware) {
    bool r, s = false;
    if (!timer[0]) { timer[0] = millis(); }
    while (1) {
      if (Ethernet.linkStatus() != LinkOFF && Ethernet.hardwareStatus() != EthernetNoHardware) {
        Serial.println("Ethernet hardware connected.");
        break;
      }
      if (millis() - timer[0] > 10000 && timer[0]) {
        resetFunc();
        timer[0] = 0;
      }
      if (Ethernet.hardwareStatus() == EthernetNoHardware && !r) {
        Serial.println("Ethernet hardware was not found.");
        r = true;
      }
      if (Ethernet.linkStatus() == LinkOFF && !s) {
        Serial.println("Ethernet cable connection not detected.");
        s = true;
      }
    }
  }
  timer[0] = 0;
  Serial.println("Connecting TCP.");
  
  int j = 0;
  while(1){
    if(j < 7){
      if(!HMI.connected()){
        HMI.begin(HMIip, 502);
        Serial.print(".");
        j++;
        delay(1000);
      }
      else{break;}
    }
    else{Serial.println("Unable to connect to hmi!");delay(100);resetFunc();}
  }
*/
  matrixSetup("MobileRefueler", "Disp 4.1.1 noRTU");  //M master

  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    double t = (analogRead(PT) - 200.0) / 0.084;
    if(t < 14){t = 14.42;}
    vehiclePT.addValue(t);  //Read PT value
  }
  Serial.println("OK");
  digitalWrite(GL, HIGH); //green light
  
  STATE = IDLE_OFF;
}
// MOXA PINS
//DI0 = GB
//DI1 = RB
//DI2 = LSR
//DI3 = GSR
//DI4 = DMD
//DI5 = Carrier OK

//R0 = pump
//R1 = valve
//R2 = GL
//R3 = AL
//R4 = RL
//R5 = DMD

void loop() {
  /*
  if (!HMI.connected()) {
    HMI.begin(HMIip, 502);
    Serial.print("[ERROR] Unable to connect to slave");
    delay(100);
  }*/
  if (!timer[2]) { timer[2] = millis(); }
  if (millis() - timer[2] > 100) {
    int temp = analogRead(PT);
    Serial.println(temp);
    if (0 <= temp < 15000) {
      double t = (temp - 200.0) / 0.084;
      //if(t < 14){t = 14.42;}
      vehiclePT.addValue(t);  //Read PT value
    }
    timer[2] = millis();
  }

  currentPressure = vehiclePT.getAverage();

  //transceiveTCP();


  flashAmberFunc(500);
  flashRedFunc(250);
  daughterPrint(250);

// Button debounces
  if (digitalRead(GB)) {if(!debouncer0){ debouncer0 = millis(); }}
  else{debouncer0 = 0;}
  if (millis() - debouncer0 > 1000 && debouncer0){ greenButton = true; }
  else{greenButton = false;}

  if (digitalRead(RB)) {if(!debouncer1){ debouncer1 = millis(); }}
  else{debouncer1 = 0;}
  if (millis() - debouncer1 > 3000 && debouncer1){ redButton = true; }
  else{redButton = false;}

  if(!digitalRead(LSR)){
    PREV_STATE = STATE;
    STATE = FAULT;
  }

  switch(STATE){
    case FAULT:
      if(!timer[0]){
        timer[0] = millis();
        flashRed = true;
        flashAmber = false;
        digitalWrite(VALVE, LOW);
        digitalWrite(GL, LOW);
        digitalWrite(AL, LOW);
        digitalWrite(RL, HIGH);
      }
      if(digitalRead(LSR)){
        PREV_STATE = STATE;
        STATE = IDLE_OFF;
        flashRed = false;
        timer[0] = 0;
      }
    break;

    case IDLE_OFF:
      digitalWrite(GL, HIGH);
      digitalWrite(AL, LOW);
      digitalWrite(RL, LOW);
      digitalWrite(VALVE, LOW);
      displayState = 1;
      if(greenButton || virtualGreenButton){
        if(virtualGreenButton){Serial.print("VIRTUAL ");}
        Serial.println("START DETECTED, BEGIN FILLING");
        virtualGreenButton = false;
        digitalWrite(GL, LOW); //green light off 
        digitalWrite(AL, HIGH); //amber light on
        displayState = 2;
        PREV_STATE = STATE;
        STATE = SPIKE;
      }
    break;

    case SPIKE:
      if(!timer[0]){timer[0] = millis();digitalWrite(VALVE, HIGH);} //valve&dmd on
      if(millis() - timer[0] > 1500 && timer[0]){
        digitalWrite(VALVE, LOW); //valve off
        if(!timer[1]){timer[1] = millis();}
      }
      if(millis() - timer[1] > 5000 && timer[1]){
        PREV_STATE = STATE;
        STATE = FILL;
        timer[0] = 0;
        timer[1] = 0;
      }
    break;

    case FILL:
      if(!timer[0]){
        timer[0] = millis();
        timer[5] = millis();
        digitalWrite(VALVE, HIGH); //valve on
        flashAmber = true;
      }

      if(redButton){
        err_timeout = true;
        PREV_STATE = STATE;
        STATE = FINISHED;
        timer[0] = 0;
      }
    break;

    case FINISHED:
      flashAmber = false;
      flashRed = false;
      PREV_STATE = STATE;
      STATE = IDLE_OFF;
      err_timeout = false;
      timer[0] = 0;
    break;
    default:
      Serial.println("Unknown state!!!");
      digitalWrite(VALVE, LOW);
      digitalWrite(GL, HIGH);
      digitalWrite(AL, HIGH);
      digitalWrite(RL, HIGH);
    break;
  }

}
