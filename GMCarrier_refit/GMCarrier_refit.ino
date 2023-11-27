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
//Serial3 is middle
void (*resetFunc)(void) = 0;  // Allows hard reset of arduino through software

void setup() {
  TCCR5B = (TCCR5B & 0xF8) | 0x01;
  Wire.begin();
  Wire.setWireTimeout(10000, true);
  pinModeSetup();
  Serial.begin(9600);
 //Serial1.begin(9600, SERIAL_8N1);  //remote
  Serial3.begin(9600, SERIAL_8N1);  //local
  Serial.println("OK");
  matrixSetup("Refueler", "GM V4.1.1CM");

  mbLocal.begin(mbSlaveCarrierAddr, Serial3);
  mbLocal.preTransmission(preTransmission);
  mbLocal.postTransmission(postTransmission);

  //mbRemote.begin(&Serial1, RE_DE1);
  //mbRemote.setBaudrate(9600);
  //mbRemote.slave(mbSlaveAddr);
  //mbRemote.addHreg(0, 0, 40);
  delay(500);  //wait for system to boot up

  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    bank1RA.addValue((analogRead(ACT1_PRESS) - 200.0) / 0.084);  // Grab pressure sensor values and convert to psi
    bank2RA.addValue((analogRead(ACT2_PRESS) - 200.0) / 0.084);  // Add pressure readings to moving average to smooth out data
    man1RA.addValue((analogRead(ACT3_PRESS) - 200.0) / 0.084);
    delay(100);
  }

  Serial.println("OK");
}

void loop() {
  if (digitalRead(DEMAND)) {if(!debouncer0){ debouncer0 = millis(); }}
  else{debouncer0 = 0;}
  if (millis() - debouncer0 > 150 && debouncer0){ dmd = true; }
  else{dmd = false;}

  transceiveRTU();
  //SerialCLI();
  dataRead(1000);
  daughterPrint(250);
  dataSend(2000);
/*
  double sendBuf[20] = {-1};
  sendBuf[0] = activeBank;
  sendBuf[1] = count;
  sendBuf[2] = man1Pt;
  sendBuf[3] = man2Pt;
  for(int i = 0; i < 4; i++){
    sendBuf[4+i] = banks[i].pressure;
    sendBuf[8+i] = banks[i].initPsi;
    sendBuf[12+i] = banks[i].finalPsi;
    sendBuf[16+i] = banks[i].bankTime;
  }

  for (int i = 0; i < 20; i++) {
    mbRemote.Hreg(i, sendBuf[i]);
  }

  mbRemote.task();
  yield();
  vehPsi = mbRemote.Hreg(30);

  if(!timer[3]){vehPsi = rtuCheck; timer[3] = millis();}
  if(millis() - timer[3] < 30000 && timer[3]){
    if(vehPsi != rtuCheck){timer[3] = 0; rtuComms = true;}
  }
  else{rtuComms = false;}

  if (mbRemote.Hreg(31)) { MOVING_AVG_SIZE = mbRemote.Hreg(11); }
  if (mbRemote.Hreg(32)) { STALL_THRESHOLD = mbRemote.Hreg(12); }
  if (mbRemote.Hreg(33)) { MIN_ALLOWED_PRESSURE = mbRemote.Hreg(13); }
  if (mbRemote.Hreg(34)) { DELTA_PSI_TIME = mbRemote.Hreg(14); }
  if (mbRemote.Hreg(35)) { STALL_CONFIRMATION_COUNT = mbRemote.Hreg(15); }
  if (mbRemote.Hreg(36)) { STALL_TIMEOUT = mbRemote.Hreg(16); }
*/
  if(activeBank == 1){digitalWrite(RLY2, HIGH); digitalWrite(RLY3, LOW);}
  else if(activeBank == 2){digitalWrite(RLY2, LOW); digitalWrite(RLY3, HIGH);}
  else{digitalWrite(RLY2, LOW); digitalWrite(RLY3, LOW);}

  switch (STATE) {
    case ORDER_BANKS:
      for (int i = 0; i < 4; i++) {  // check if any of our pressures are zero, if so we need to read them again
        if (banks[i].pressure && MIN_ALLOWED_PRESSURE <= banks[i].pressure <= 10000){;}
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

      if(dmdOverride || dmd){
        STATE = CASCADE;
        timer[4] = 0;
      }
      break;

    case CASCADE:

      if (dmd || dmdOverride) {
        if (!activeBank) {  // Check if the bank we are concerned with is already open
          activeBank = banks[count].ID;
          banks[count].initPsi = banks[count].pressure;
          timer[0] = millis();
        }
        else {                                                                     // If the bank in question is already on
          if (millis() - timer[0] > 3000 && timer[0] && banks[count].pressure < MIN_ALLOWED_PRESSURE) {  // wait one second then check if we are stalling in pressure
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
        stallCnt = 0;
        memset(prevPsi,0,sizeof(prevPsi));
        
        Serial.println("finished");
      }

      break;

    case IDLE_ON:
      if (!dmd && !dmdOverride) {digitalWrite(ESTOP_BREAK,HIGH); STATE = ORDER_BANKS; }
      else{digitalWrite(ESTOP_BREAK,LOW);}
      count = 0;
      break;

    default:
      break;
  }
}