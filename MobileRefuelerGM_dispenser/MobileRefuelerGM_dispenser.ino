#include "dispenser.h"
RunningAverage avgPT50(DISP_MOVING_AVG_SIZE);
RunningAverage avgPT350(DISP_MOVING_AVG_SIZE);
RunningAverage avgPT700(DISP_MOVING_AVG_SIZE);
SmallMatrix smallMatrix[3] = { SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = { LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

void (*resetFunc)(void) = 0;  // Allows hard reset of arduino through software

void preTransmission(){digitalWrite(31, 1);}
void postTransmission(){digitalWrite(31, 0);}

void setup() {
  TCCR5B = (TCCR5B & 0xF8) | 0x01;
  Wire.setWireTimeout(10000, true);
  pinModeSetup();
  Serial.begin(9600);
  Serial3.begin(9600, SERIAL_8N1);
  delay(500);
  matrixSetup("Mobile Refueler", "v9.0.2M GM");  //M master
  //################## Ethernet Setup ##################
  Ethernet.init(10);
  Ethernet.begin(mac, ip[0]);
  delay(5);
  if(Ethernet.linkStatus() == LinkOFF || Ethernet.hardwareStatus() == EthernetNoHardware) {
    bool r, s = false;
    if(!timer[0]) { timer[0] = millis(); }
    while (1) {
      if(Ethernet.linkStatus() != LinkOFF && Ethernet.hardwareStatus() != EthernetNoHardware) {
        Serial.println("Ethernet hardware connected.");
        break;
      }
      if(millis() - timer[0] > 10000 && timer[0]) {
        resetFunc();
        timer[0] = 0;
      }
      if(Ethernet.hardwareStatus() == EthernetNoHardware && !r) {
        Serial.println("Ethernet hardware was not found.");
        r = true;
      }
      if(Ethernet.linkStatus() == LinkOFF && !s) {
        Serial.println("Ethernet cable connection not detected.");
        s = true;
      }
    }
  }
  timer[0] = 0;
  Serial.println("Connecting TCP...");
  connectTCP();
  if (!checkTCP(true)) {
    Serial.println("All TCP devices connected.");
  }
  else{Serial.println("A TCP Device isnt connected,reseting");delay(500);resetFunc();}
  mbRTU.begin(3, Serial3);
  mbRTU.preTransmission(preTransmission);
  mbRTU.postTransmission(postTransmission);
  delay(500);  //wait for system to boot up

  for (int i = 0; i < DISP_MOVING_AVG_SIZE; i++) {
    avgPT50.addValue((analogRead(PT50) - 200.0) / 0.084);
    avgPT350.addValue((analogRead(PT350) - 200.0) / 0.084);
    avgPT700.addValue(2*(analogRead(PT700) - 200.0) / 0.084);
  }
  matrixSetup("MobileRefueler", "v9.0.2M GM");  //M master
  if(modbusTCPClient3.inputRegisterRead(4) < 2){
    resetFunc();
    delay(1000);
  }    

  
  Serial.println("OK");
  STATE = IDLE_OFF;
}

void loop() {
  SerialCLI();

  if(!timer[7]) { timer[7] = millis(); }
  if(millis() - timer[7] > 250) {
    double aa = (analogRead(PT50) - 200.0) / 0.084;
    double bb = (analogRead(PT350) - 200.0) / 0.084;
    double cc = 2*(analogRead(PT700) - 200.0) / 0.084;

    if(10 < aa < 15000) { avgPT50.addValue(aa); }
    if(10 < bb < 15000) { avgPT350.addValue(bb); }
    if(10 < cc < 20000) { avgPT700.addValue(cc); }

    timer[7] = millis();
  }

  pt50 = avgPT50.getAverage();
  pt350 = avgPT350.getAverage();
  pt700 = avgPT700.getAverage();

  

  //############### Communications #######################
  transceiveRTU();
  transceiveTCP();

  printOuts();

  if(pressureSetting == 700){
    maxCarr = max(bank1Pt, max(bank2Pt, bank3Pt));
  }
  else{
     maxCarr = max(bank1Pt, bank2Pt);
  }

//----------------Flash Amber Light------------------
  if(!timer[3] && flashAmber) { timer[3] = millis(); }
  if(millis() - timer[3] > 500 && flashAmber) {
    tog[2] = !tog[2];
    DO_Disp_LED02_LampAmber = tog[2];
    timer[3] = millis();
  }
  else if(!flashAmber && timer[3]) {
    DO_Disp_LED02_LampAmber = false;
    timer[3] = 0;
  }
//--------------------------------------------------
//--------------------Display-----------------------
  if(!timer[9]) { timer[9] = millis(); }
  if(millis() - timer[9] > 500 && timer[9]) {
    timer[9] = millis();
    bigMatrix[0].writeInt(currentPressure);
    smallMatrix[0].displayChar('M', (bitRead(sendByte1, 1) == 2) ? 1 : 0);

    switch (activeBank) {
      case 1:
        bigMatrix[1].writeInt(bank1Pt);
        smallMatrix[1].displayChar('1', 0);
        break;
      case 2:
        bigMatrix[1].writeInt(bank2Pt);
        smallMatrix[1].displayChar('2', 0);
        break;
      case 3:
        bigMatrix[1].writeInt(bank3Pt);
        smallMatrix[1].displayChar('3', 0);
        break;
      default:
        bigMatrix[1].writeInt(0);
        smallMatrix[1].squareDisplay(0);
        break;
    }
    bigMatrix[2].writeInt(maxCarr);
    smallMatrix[2].squareDisplay(0);
  }
//-----------------------------------------------------
//--------------------Button Debouncing----------------
  if(DI_Disp_SW01_ButtonGreen){if(!debouncer0){debouncer0 = millis();}}
  else{debouncer0 = 0; debouncedGreenButton = false;}
  if(millis() - debouncer0 > 100 && debouncer0){
    debouncedGreenButton = true;
    debouncer0 = 0;
  }

  if(DI_Disp_SW03_ButtonRed){if(!debouncer1){debouncer1 = millis();}}
  else{debouncer1 = 0; debouncedRedButton = false;}
  if(millis() - debouncer1 > 100 && debouncer1){
    debouncedRedButton = true;
    debouncer1 = 0;
  }

  if(virtualRedButton || debouncedRedButton) {
    Serial.println("RESET DETECTED, SHUTTING DOWN");
    virtualRedButton = false;
    activeBank = 0;
    displayState = 1;
    PREV_STATE = STATE;
    STATE = SHUTDOWN;
    flashAmber = false;
  }
  //----------------------------------------------------
  //----------------Pressure Setting--------------------
  if(pressureSetting == 50){
    currentPressure = pt50;
  }
  else{
    currentPressure = pt700;
  }
//-----------------------------------------------------
//-----------------------STATES------------------------
  switch (STATE) {
    case FAULT:
    break;

    case IDLE_OFF:
      if(TARGET_PRESSURE_350BAR && TARGET_PRESSURE_700BAR && (pressureSetting == 700 || pressureSetting == 350 || pressureSetting == 50)) {  // make sure we received a non zero target pressure from hmi and nonzero pressure setting that has been confirmed with carrier
        displayState = 1;
        DO_Disp_LED01_LampGreen = true;                                                                                                                                   // Show HMI green "ready to begin filling banner"
        if(DI_Disp_ASL_GSR && DI_Disp_ASL_LSR && debouncedGreenButton){
          displayState = 2;        // Get rid of banners and show filling screen
          if(pressureSetting > 50){ STATE = FILL100; }
          else if(pressureSetting == 50){ STATE = FLOW; }
          else{ displayState = 1; Serial.println("no pressure setting!");}
          bubbleSort(banks,3);
          PREV_STATE = IDLE_OFF;
        }
      }
      else {DO_Disp_LED01_LampGreen = false;}
    break;

    case INTEGRITY:
      if(!timer[0]) { timer[0] = millis(); }
      if(remoteActiveBank){activeBank = 0; break;}

      checkPressure(currentPressure, pressureSetting, (PREV_STATE == FILL85) ? FILL100 : FILL85);  // Check pressure by opening relay 1 and exiting to 3 or 4 if all is well

      if(millis() - timer[0] > 60000 * 2 && timer[0]) {
        errorOut(2000);
        timer[0] = 0;
      }
      break;

    case FILL85:
      if(!timer[0]) {
        timer[0] = millis();
        initPress = currentPressure;
        flashAmber = true;
        DO_Disp_LED01_LampGreen = false;
        if(pressureSetting == 700){
          DO_Disp_XV700_HighPressure = true;
          DO_Disp_XV350_MediumPressure = false;
          targetPressure = (maxCarr < TARGET_PRESSURE_700BAR)?maxCarr:TARGET_PRESSURE_700BAR;} //check if maximum available pressure is less than desired target pressure, if so change target pressure to max available pressure
        else{
          DO_Disp_XV350_MediumPressure = true;
          DO_Disp_XV700_HighPressure = false;
          targetPressure = (maxCarr < TARGET_PRESSURE_350BAR)?maxCarr:TARGET_PRESSURE_350BAR;}
      }


      if(!banks[count].onOff) {  // Check if the bank we are concerned with is already open
        banks[count].onOff = true;
        activeBank = banks[count].ID;
        timer[3] = millis();
      }
      else{                                                                     // If the bank in question is already on
        if(millis() - timer[3] > 1000 && timer[3] && stallCheck()){  // wait one second then check if we are stalling in pressure
          activeBank = 0;
          if(count < numBanks){banks[count].onOff = false; count++;}
          else{
            finalPressure = currentPressure;
            displayState = 3;
            STATE = SHUTDOWN;
            PREV_STATE = FILL85;
            timer[0] = 0;
          }
        }
      }

      if(currentPressure > (SECOND_CHECK/100) * targetPressure && millis() - timer[0] > 10000 && timer[0]) {
        DO_Disp_XV700_HighPressure = false;
        DO_Disp_XV350_MediumPressure = false;
        STATE = INTEGRITY;
        PREV_STATE = FILL85;
        timer[0] = 0;
        break;
      }

      if(millis() - timer[0] > FILL_TIMEOUT * 60000 && timer[0]) {
        errorOut(2000);
        timer[0] = 0;
      }
      break;

    case FILL100:
      if(!timer[0]){
        timer[0] = millis();
        flashAmber = true;
        DO_Disp_LED01_LampGreen = false;
      }
      if(pressureSetting == 700){
        if(activeBank == 1 || activeBank == 2){
          DO_Disp_XV700_HighPressure = false;
          DO_Disp_XV350_MediumPressure = true;
          targetPressure = (maxCarr < TARGET_PRESSURE_700BAR)?maxCarr:TARGET_PRESSURE_700BAR; //check if maximum available pressure is less than desired target pressure, if so change target pressure to max available pressur        }
        }
        else if(activeBank == 3){
          DO_Disp_XV700_HighPressure = true;
          DO_Disp_XV350_MediumPressure = false;
        }
        
      }
      else{
        DO_Disp_XV350_MediumPressure = true;
        DO_Disp_XV700_HighPressure = false;
        targetPressure = (maxCarr < TARGET_PRESSURE_350BAR)?maxCarr:TARGET_PRESSURE_350BAR;
      }
      if(!banks[count].onOff) {  // Check if the bank we are concerned with is already open
        banks[count].onOff = true;
        activeBank = banks[count].ID;
        timer[3] = millis();
      }
      else{                                                                     // If the bank in question is already on
        if(stallCheck()){  // wait one second then check if we are stalling in pressure
          if(pressureSetting == 350){
            if(!count){banks[count].onOff = false; count++;}
            else{
              finalPressure = currentPressure;
              displayState = 3;
              STATE = SHUTDOWN;
              PREV_STATE = FILL100;
              timer[0] = 0;
            }
          }
          if(pressureSetting == 700){
            if(count <= 1){banks[count].onOff = false; count++;}
            else{
              finalPressure = currentPressure;
              displayState = 3;
              STATE = SHUTDOWN;
              PREV_STATE = FILL100;
              timer[0] = 0;
            }
          }
        }
      }
      if(currentPressure >= targetPressure && timer[0] && millis() - timer[0] > 5000) {
        finalPressure = currentPressure;
        displayState = 3;
        STATE = SHUTDOWN;
        PREV_STATE = FILL100;
        timer[0] = 0;
        break;
      }

      if(millis() - timer[0] > FILL_TIMEOUT * 60000 && timer[0]) {
        errorOut(2000);
        timer[0] = 0;
      }
      break;

    case FLOW:
      if(!timer[0]) {
        initPress = currentPressure;
        timer[0] = millis();
        flashAmber = true;
        DO_Disp_XV350_MediumPressure = false;
        DO_Disp_XV700_HighPressure = false;
        DO_Disp_XV50_LowPressure = true;
      }
      if(!banks[count].onOff) {  // Check if the bank we are concerned with is already open
        banks[count].onOff = true;
        activeBank = banks[count].ID;
        timer[3] = millis();
      }
      if(millis() - timer[3] > 10000 && timer[3] && banks[count].pressure < MIN_ALLOWED_PRESSURE){
        if(banks[count+1].pressure > 7500){count = 100;} //prevent usage of high bank
        if(count < numBanks){
          banks[count].onOff = false;
          count++;
        }
        else{
          finalPressure = currentPressure;
          displayState = 3;
          count = 0;
          STATE = SHUTDOWN;
          PREV_STATE = FILL100;
          timer[0] = 0;
        }
      }

      if(millis() - timer[0] > 60000 && timer[0] && debouncedGreenButton) {
        finalPressure = currentPressure;
        displayState = 3;
        STATE = SHUTDOWN;
        PREV_STATE = FLOW;
        timer[0] = 0;
      }
    break;
      
    case SHUTDOWN:
      if(!timer[0]) {
        timer[0] = millis();
      }
      if(5000 < millis() - timer[0] < 15000 && timer[0]){
        DO_Disp_XV700_Vent = true;
      }
      banks[0].onOff = false;
      banks[1].onOff = false;
      banks[2].onOff = false;
      count = 0;
      activeBank = 0;
      DO_Disp_LED02_LampAmber = false;
      flashAmber = false;
      DO_Disp_XV50_LowPressure = false;
      DO_Disp_XV350_MediumPressure = false;
      DO_Disp_XV700_HighPressure = false;
      flashAmber = false;
      H2Leak = false;

      if(millis() - timer[0] > 15000){// || AI_Disp_psig_PT700_HighModePress < 500) {
        DO_Disp_XV700_Vent = false;
        targetPressure = 0;
        displayState = 
        STATE = IDLE_OFF;
        PREV_STATE = SHUTDOWN;
        memset(timer, 0, sizeof(timer));
      }
    break;
  }
}