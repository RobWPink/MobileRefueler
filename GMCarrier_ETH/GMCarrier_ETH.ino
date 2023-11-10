#include "carrier.h"
#include "I2C_API.h"

Adafruit_LiquidCrystal lcd(0);

SmallMatrix smallMatrix[3] = { SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };

LargeMatrix bigMatrix[3] = { LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };

RunningAverage bank1RA(MOVING_AVG_SIZE);
RunningAverage bank2RA(MOVING_AVG_SIZE);
RunningAverage bank3RA(MOVING_AVG_SIZE);
RunningAverage bank4RA(MOVING_AVG_SIZE);
RunningAverage man1RA(MOVING_AVG_SIZE);
RunningAverage man2RA(MOVING_AVG_SIZE);

void (*resetFunc)(void) = 0;  // Allows hard reset of arduino through software

void setup() {
  TCCR5B = (TCCR5B & 0xF8) | 0x01;
  Wire.begin();
  Wire.setWireTimeout(10000, true);
  pinModeSetup();
  Serial.begin(9600);
  Serial.println("OK");
  matrixSetup("MobileRefueler", "GM V2.1.1M ETH");
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
      if(!master.connected()){
        master.begin(slaveIP, 502);
        Serial.print(".");
        j++;
        delay(1000);
      }
      else{break;}
    }
    else{Serial.println("Unable to connect to slave carrier!");delay(100);resetFunc();}
  }

  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    bank1RA.addValue((analogRead(ACT1_PRESS) - 200.0) / 0.084);  // Grab pressure sensor values and convert to psi
    bank2RA.addValue((analogRead(ACT2_PRESS) - 200.0) / 0.084);  // Add pressure readings to moving average to smooth out data
    man1RA.addValue((analogRead(ACT3_PRESS) - 200.0) / 0.084);
    delay(100);
  }

  Serial.println("OK");
}

void loop() {
  if (!master.connected()) {
    master.begin(slaveIP, 502);
    Serial.print("[ERROR] Unable to connect to slave");
    delay(100);
  }

  if (digitalRead(DEMAND)) {if(!debouncer0){ debouncer0 = millis(); }}
  else{debouncer0 = 0;}
  if (millis() - debouncer0 > 150 && debouncer0){ dmd = true; }
  else{dmd = false;}

  //transceiveRTU();
  //SerialCLI();
  dataRead(1000);
  daughterPrint(250);
  dataSend(2000);

  if(activeBank == 1){digitalWrite(RLY2, HIGH); digitalWrite(RLY3, LOW);}
  else if(activeBank == 2){digitalWrite(RLY2, LOW); digitalWrite(RLY3, HIGH);}
  else{digitalWrite(RLY2, LOW); digitalWrite(RLY3, LOW);}

  switch (STATE) {
    case ORDER_BANKS:
      for (int i = 0; i < 4; i++) {  // check if any of our pressures are zero, if so we need to read them again
        if (banks[i].pressure && MIN_ALLOWED_PRESSURE <= banks[i].pressure <= 10000 && banks[i].allowed){;}
        else {
          if (errcnt > 100) {
            banks[i].allowed = false;
            errcnt = 0;
          }
          else {
            errcnt++;
            return;
          }  //if we fail to read properly within 50 reads, exclude bank from cascade
        }
      }
      bubbleSort(banks, 4);  // orderes the array of structures based on pressure
      STATE = IDLE_OFF;
      break;

    case IDLE_OFF:
      if(!activeBank){
        if(overrideBank == 1){digitalWrite(RLY2, HIGH); digitalWrite(RLY3, LOW);}
        else if(overrideBank == 2){digitalWrite(RLY2, LOW); digitalWrite(RLY3, HIGH);}
        else{digitalWrite(RLY2, LOW); digitalWrite(RLY3, LOW);}
      }

      if(!timer[4]){timer[4] = millis();}
      if(millis() - timer[4] > 20000 && timer[4]){ //run this continually after 20 seconds of inactivity
        timer[4] = 0;
        count = 0;

        if(fillTime){ //run this once after the 20 seconds
          fillTime = 0;
          STATE = ORDER_BANKS;
        }
      }

      if(dmdOverride || dmd){
        STATE = CASCADE;
        timer[4] = 0;
      }
      break;

    case CASCADE:
      if(!fillTime){fillTime = millis();}//fill timer

      if (dmd || dmdOverride) {
        if (!activeBank) {  // Check if the bank we are concerned with is already open
          activeBank = banks[count].ID;
          banks[count].initPsi = banks[count].pressure;
          timer[0] = millis();
        }
        else {                                                                     // If the bank in question is already on
          if (millis() - timer[0] > 1000 && timer[0] && banks[count].pressure <= MIN_ALLOWED_PRESSURE) {  // wait one second then check if we are stalling in pressure
            banks[count].finalPsi = banks[count].pressure;
            banks[count].bankTime = millis() - timer[0];
            activeBank = 0;
            if(count >= numBanks - 1){STATE = IDLE_ON;}
            else{count++;}
          }
        }
      }
      else {  // Demand is turned off, make sure all valves are off too
        activeBank = 0;
        count = 0;
        timer[0] = 0;
        
        Serial.println("finished");
      }

      break;

    case IDLE_ON:
      if (!dmd && !dmdOverride) { STATE = IDLE_OFF; }
      count = 99;
      break;

    default:
      break;
  }
}