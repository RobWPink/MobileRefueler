void errorOut(int errCode) {
  errors[errCnt++] = errCode + STATE;
  PREV_STATE = STATE;
  STATE = FAULT;  //error out
  timer[0] = 0;
  timer[1] = 0;
  tog[0] = false;  //stop printouts
  tog[1] = false;
  /*
  XXXS - XXX[STATE]
  1XXX - cannot connect to:
    11XX - TCP modbus
      (111X - MOXA)
      (112X - HMI)
    12XX - RTU modbus
      (121X - carrier)
      (122X - other dispenser)
  2XXX - timeout

  3XXX - invalid data
  */
}


void transceiveRTU() {
  mbRTU.clearTransmitBuffer();
  mbRTU.setTransmitBuffer(0, activeBank);
  mbRTU.setTransmitBuffer(1, currentPressure);
  uint8_t result = mbRTU.readWriteMultipleRegisters(0, 5, 10, 2);
  if (result == mbRTU.ku8MBSuccess) {
    for(int i = 0; i < numBanks; i++){
           if(banks[i].ID == 1){banks[i].pressure = mbRTU.getResponseBuffer(0); bank1Pt = banks[i].pressure;}
      else if(banks[i].ID == 2){banks[i].pressure = mbRTU.getResponseBuffer(1); bank2Pt = banks[i].pressure;}
      else if(banks[i].ID == 3){banks[i].pressure = mbRTU.getResponseBuffer(2); bank3Pt = banks[i].pressure;}
    }
    manPt = mbRTU.getResponseBuffer(3);
    remoteActiveBank = mbRTU.getResponseBuffer(4);
    mbRTU.clearResponseBuffer();
    rtuFailCnt[0] = 0;
  } else {
    rtuFailCnt[0]++;
  }
  if (rtuFailCnt[0] > 10) {
    Serial.println("Failure to read/write data from slave carrier!");
    Serial.print("Modbus Error Code: ");
    Serial.println(result, HEX);
    rtuFailCnt[0] = 0;
  }
}

void transceiveTCP() {
  checkTCP(false);
  uint16_t CURRENT_DI_STATUS_WORD = modbusTCPClient1.inputRegisterRead(48);
  int16_t res = CURRENT_DI_STATUS_WORD;
  if(res == -1) {
    Serial.print("Failed to read moxa1! ");
    Serial.println(modbusTCPClient1.lastError());
    modbusTCPClient1.end();
  }
  else{
    DI_Disp_SW01_ButtonGreen = bitRead(CURRENT_DI_STATUS_WORD, 0);
    DI_Disp_SW03_ButtonRed = bitRead(CURRENT_DI_STATUS_WORD, 1);
    DI_Disp_ASL_LSR = bitRead(CURRENT_DI_STATUS_WORD, 2);
    DI_Disp_ASL_GSR = bitRead(CURRENT_DI_STATUS_WORD, 3);
    DI_Comm_H2DemandRemote = bitRead(CURRENT_DI_STATUS_WORD, 4);
    DI_Disp_ASL_CarrierOK = bitRead(CURRENT_DI_STATUS_WORD, 5);
  }

  uint16_t CURRENT_DO1_STATUS_WORD = 0;
  bitWrite(CURRENT_DO1_STATUS_WORD,1,DO_Disp_XV50_LowPressure);
  bitWrite(CURRENT_DO1_STATUS_WORD,2,DO_Disp_LED01_LampGreen);
  bitWrite(CURRENT_DO1_STATUS_WORD,3,DO_Disp_LED02_LampAmber);
  bitWrite(CURRENT_DO1_STATUS_WORD,4,DO_Disp_LED03_LampRed);
  bitWrite(CURRENT_DO1_STATUS_WORD,5,DO_Comm_H2DemandLocal);
  if(!modbusTCPClient1.holdingRegisterWrite(32, CURRENT_DO1_STATUS_WORD)){
    Serial.print("Failed to write moxa1! ");
    Serial.println(modbusTCPClient1.lastError());
  }

  uint16_t CURRENT_DO2_STATUS_WORD = 0;
  bitWrite(CURRENT_DO2_STATUS_WORD,0,DO_Disp_XV700_HighPressure);
  bitWrite(CURRENT_DO2_STATUS_WORD,1,DO_Disp_XV350_MediumPressure);
  bitWrite(CURRENT_DO2_STATUS_WORD,2,DO_Disp_XV700_Vent);
  if(!modbusTCPClient2.holdingRegisterWrite(32, CURRENT_DO2_STATUS_WORD)){
    Serial.print("Failed to write moxa2! ");
    Serial.println(modbusTCPClient2.lastError());
  }


  double readBuf[17] = {0};
  if(!modbusTCPClient3.requestFrom(INPUT_REGISTERS,0,16)){
    Serial.print("Failed to read hmi! ");
    Serial.println(modbusTCPClient3.lastError());
  }
  else{
    for(int i = 0; i < 17;i++){
      readBuf[i] = modbusTCPClient3.read();
    }
  }

  if(!modbusTCPClient3.beginTransmission(HOLDING_REGISTERS,0,17)){
    Serial.print("Failed to write hmi! ");
    Serial.println(modbusTCPClient3.lastError());
  }
  else{
    modbusTCPClient3.write(!DI_Disp_ASL_LSR);
    modbusTCPClient3.write(!DI_Disp_ASL_GSR);
    modbusTCPClient3.write(H2Leak);
    modbusTCPClient3.write(0);
    modbusTCPClient3.write(0);
    modbusTCPClient3.write(0);
    modbusTCPClient3.write(STATE);
    modbusTCPClient3.write(pt50);
    modbusTCPClient3.write(pt350);
    modbusTCPClient3.write(pt700);
    modbusTCPClient3.write(initPress);
    modbusTCPClient3.write(finalPressure);
    modbusTCPClient3.write(currentPressure);
    modbusTCPClient3.write(bank1Pt);
    modbusTCPClient3.write(bank2Pt);
    modbusTCPClient3.write(bank3Pt);
    modbusTCPClient3.write(manPt);
  }
  modbusTCPClient3.endTransmission();

  if(STATE == IDLE_OFF){
    DO_Disp_XV50_LowPressure = readBuf[0];
    DO_Disp_XV350_MediumPressure = readBuf[1];
    DO_Disp_XV700_HighPressure = readBuf[2];
    DO_Disp_XV700_Vent     = readBuf[3];
    CARR_MOVING_AVG_SIZE   = readBuf[4];
    DELTA_PSI_TIME         = readBuf[5];
    MIN_ALLOWED_PRESSURE   = readBuf[6];
    STALL_THRESHOLD        = readBuf[7];
    SECOND_CHECK           = readBuf[8];
    FILL_TIMEOUT           = readBuf[9];
    H2_SENSOR_LIMIT        = readBuf[10];
    DISP_MOVING_AVG_SIZE   = readBuf[11];
    PRESSURE_LOSS_TOL      = readBuf[12];
    TARGET_PRESSURE_700BAR = readBuf[13];
    TARGET_PRESSURE_350BAR = readBuf[14];
    activeBank = modbusTCPClient3.holdingRegisterRead(17);
  }
  else{modbusTCPClient3.holdingRegisterWrite(17,activeBank);}
  pressureSetting = readBuf[15];
}

bool checkPressure(int curPsi, int psiSetting, enum state exitState) {
  activeBank = 1;
  if(psiSetting == 50){DO_Disp_XV50_LowPressure = true;}
  else if(psiSetting == 350){DO_Disp_XV350_MediumPressure = true;}
  else if(psiSetting == 700){DO_Disp_XV700_HighPressure = true;}
  else{errorOut(3000);activeBank = 0;}
  if (!checkTimer[1]) { checkTimer[1] = millis(); }  // Start timer to wait for gas to settle before taking intial pressure value

  if (millis() - checkTimer[1] > 5000 && checkTimer[1]) {  //settling time
    if (!initPressure && curPsi) {
      initPressure = curPsi;
      checkTimer[0] = millis();  //comparison interval
    }

    if (millis() - checkTimer[0] > 3000 && checkTimer[0]) {  //measure every 3 seconds for leaks
      if (H2cnt > 20) {                            // Grab 20 values to make sure we are leaking
        H2Leak = true;
        Serial.println("H2LEAK!!!");
        errorOut(8000);
        DO_Disp_XV50_LowPressure = false;
        DO_Disp_XV350_MediumPressure = false;
        DO_Disp_XV700_HighPressure = false;
        activeBank = 0;
        return true;
      }
      else if (initPressure - curPsi > PRESSURE_LOSS_TOL) {H2cnt++;}
      else {
        timer[0] = 0;
        checkTimer[0] = 0;
        checkTimer[1] = 0;
        PREV_STATE = STATE;
        STATE = exitState;
        DO_Disp_XV50_LowPressure = false;
        DO_Disp_XV350_MediumPressure = false;
        DO_Disp_XV700_HighPressure = false;
        activeBank = 0;
      }
      initPressure = 0;
    }
  }
  return false;
}

void bubbleSort(struct bank array[], int size) { //sort an array of structs by their pressure data
  for (int step = 0; step < (size - 1); ++step) {
    int swapped = 0;
    for (int i = 0; i < (size - step - 1); ++i) {
      if (array[i].pressure > array[i + 1].pressure) {
        struct bank temp = array[i];
        array[i] = array[i + 1];
        array[i + 1] = temp;
        swapped = 1;
      }
    }
    if (swapped == 0)
      break;
  }
}

bool stallCheck() {

  if(banks[count].ID != 3){
    if(banks[count].pressure > 0 && banks[count].pressure > manPt){

      deltaPsi = banks[count].pressure - manPt;
      if(deltaPsi <= STALL_THRESHOLD){if(!stallTimer[0]){stallTimer[0] = millis();}}
      else{stallTimer[0] = 0;}
      if(millis() - stallTimer[0] > DELTA_PSI_TIME && stallTimer[0]){
        stallTimer[0] = 0;
        deltaPsi = 0;
        return true;
      }
    }
  }
  else{
    if(banks[count].pressure > 0 && banks[count].pressure > pt700){
      deltaPsi = banks[count].pressure - pt700;
      if(deltaPsi <= 150){if(!stallTimer[0]){stallTimer[0] = millis();}}
      else{stallTimer[0] = 0;}
      if(millis() - stallTimer[0] > 60000 && stallTimer[0]){
        stallTimer[0] = 0;
        deltaPsi = 0;
        return true;
      }
    }
  }
  return false;
}


uint16_t checkTCP(bool prnt) {
  uint16_t failed = 0;
  int replayCnt = 0;
  int replay = 0;

  while (1) {
    switch (replay) {
      case 0:
        if (!modbusTCPClient1.connected()) {
          modbusTCPClient1.begin(ip[1], 502);

          if (replayCnt >= 10) {
            bitSet(failed, replay);
            if (prnt) {
              Serial.print("Unable to connect to IO expander ");
              Serial.println(replay + 1);
              resetFunc();
            }
            replay++;
          } else {
            replayCnt++;
          }
        } else {
          replay++;
          replayCnt = 0;
        }
        break;

      case 1:
        if (!modbusTCPClient2.connected()) {
          modbusTCPClient2.begin(ip[2], 502);

          if (replayCnt >= 10) {
            bitSet(failed, replay);
            if (prnt) {
              Serial.print("Unable to connect to IO expander ");
              Serial.println(replay + 1);
              resetFunc();
            }
            replay++;
          } else {
            replayCnt++;
          }
        } else {
          replay++;
          replayCnt = 0;
        }
        break;

      case 2:
        if (!modbusTCPClient3.connected()) {
          modbusTCPClient3.begin(ip[3], 502);

          if (replayCnt >= 10) {
            bitSet(failed, replay);
            if (prnt) {
              Serial.print("Unable to connect to IO expander ");
              Serial.println(replay + 1);
              resetFunc();
            }
            replay++;
          } else {
            replayCnt++;
          }
        } else {
          replay++;
          replayCnt = 0;
        }
        break;
      default:
        return failed;
        break;
    }
  }
  return failed;
}


void disconnectTCP() {
  modbusTCPClient1.end();
  modbusTCPClient2.end();
  modbusTCPClient3.end();
}
void connectTCP() {
  if (!modbusTCPClient1.connected()) {//moxa1
    modbusTCPClient1.begin(ip[1], 502);
  }
  if (!modbusTCPClient2.connected()) {//moxa2
    modbusTCPClient2.begin(ip[2], 502);
  }
  if (!modbusTCPClient3.connected()) {//hmi
    modbusTCPClient3.begin(ip[3], 502);
  }
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
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SS, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(SS, LOW);
  digitalWrite(CS, HIGH);
  pinMode(31, OUTPUT);
  digitalWrite(31, 0);
  pinMode(ESTOP_BREAK, OUTPUT);
  digitalWrite(ESTOP_BREAK, HIGH);

  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  pinMode(TRACO_24VDC, OUTPUT);
  digitalWrite(TRACO_24VDC, HIGH);
}

