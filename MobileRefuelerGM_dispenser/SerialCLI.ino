#define SPTR_SIZE   20

void SerialCLI(){
  String str;
  char *argBuf[20] = {0};
  double argVal = 0;

  while(Serial.available()) { str = Serial.readStringUntil('\n'); }
  if (str.length() > 0) {
    int numArgs = parseString(str, argBuf, (int)sizeof(argBuf)); // arguments are stored in argBuf
    for(int n = 0; n < numArgs; n++){
      String argStr = argBuf[n];
      if(argStr.equalsIgnoreCase("bank1")){
        if(STATE == IDLE_OFF){
          if(activeBank == 1){activeBank = 0;}
          else{activeBank = 1;}
        }
        else{Serial.println("ACCESS DENIED!! Cannot manually control system while already filling!");}
      }
      else if(argStr.equalsIgnoreCase("bank2")){
        if(STATE == IDLE_OFF){
          if(activeBank == 2){activeBank = 0;}
          else{activeBank = 2;}
        }
        else{Serial.println("ACCESS DENIED!! Cannot manually control system while already filling!");}
      }
      else if(argStr.equalsIgnoreCase("bank3")){
        if(STATE == IDLE_OFF){
          if(activeBank == 3){activeBank = 0;}
          else{activeBank = 3;}
        }
        else{Serial.println("ACCESS DENIED!! Cannot manually control system while already filling!");}
      }
      else if(argStr.equalsIgnoreCase("xv50")){
        if(STATE == IDLE_OFF){DO_Disp_XV50_LowPressure = !DO_Disp_XV50_LowPressure;}
        else{Serial.println("ACCESS DENIED!! Cannot manually control system while already filling!");}
      }
      else if(argStr.equalsIgnoreCase("xv350")){
        if(STATE == IDLE_OFF){DO_Disp_XV350_MediumPressure = !DO_Disp_XV350_MediumPressure;}
        else{Serial.println("ACCESS DENIED!! Cannot manually control system while already filling!");}
      }
      else if(argStr.equalsIgnoreCase("xv700")){
        if(STATE == IDLE_OFF){DO_Disp_XV700_HighPressure = !DO_Disp_XV700_HighPressure;}
        else{Serial.println("ACCESS DENIED!! Cannot manually control system while already filling!");}
      }
      else if(argStr.equalsIgnoreCase("vent")){
        if(STATE == IDLE_OFF){DO_Disp_XV700_Vent = !DO_Disp_XV700_Vent;}
        else{Serial.println("ACCESS DENIED!! Cannot manually control system while already filling!");}
      }
      else if(argStr.equalsIgnoreCase("close")){
        activeBank = 0;
        DO_Disp_XV50_LowPressure = false;
        DO_Disp_XV350_MediumPressure = false;
        DO_Disp_XV700_HighPressure = false;
        DO_Disp_XV700_Vent = false;
      }

      else if(argStr.equalsIgnoreCase("green")){
        virtualGreenButton = !virtualGreenButton;
      }
      else if(argStr.equalsIgnoreCase("red")){
        virtualRedButton = !virtualRedButton;
      }
      else if(argStr.equalsIgnoreCase("data")){
        tog[0] = !tog[0];
      }
      else if(argStr.equalsIgnoreCase("json")){
        tog[1] = !tog[1];
      }
      else if(argStr.equalsIgnoreCase("h") || argStr.equalsIgnoreCase("help")){
        printHelp();
      }

      else if(argStr.equalsIgnoreCase("reset")){
        resetFunc();  
      }

      else{
        Serial.println("Invalid entry");
      }
    }
  }
}

int parseString(String str, char **p, int size){
  char s[100] = {0};
  int n = 0;
  strcpy(s, str.c_str());
  *p++ = strtok(s, " ");
  for(n = 1; NULL != (*p++ = strtok(NULL, " ")); n++){
    if(size == n){ break; }
  }
  return n;
}

void printHelp(){
  Serial.println("################## GM Mobile Refueler Help Menu ##################");
  Serial.println("bank1   <toggle>    -> Open Bank1's 350bar solenoid");
  Serial.println("bank2   <toggle>    -> Open Bank2's 350bar solenoid");
  Serial.println("bank3   <toggle>    -> Open Bank3's 700bar solenoid");
  Serial.println("xv50    <toggle>    -> Open 50bar solenoid");
  Serial.println("xv350   <toggle>    -> Open 350bar solenoid");
  Serial.println("xv700   <toggle>    -> Open 700bar solenoid");
  Serial.println("vent    <toggle>    -> Open 700bar VENT solenoid");
  Serial.println("close               -> Close all bank and dispenser solenoids");
  Serial.println("green               -> Artificially press green button");
  Serial.println("red                 -> Artificially press red button");
  Serial.println("data    <toggle>    -> Print all labeled data");
  Serial.println("json    <toggle>    -> Print important values in json-like format");
  Serial.println("help/h              -> This help menu");
  Serial.println("##################################################################");

}
void printOuts(){
  
  if(!timer[4]){timer[4] = millis();}
  if (millis() - timer[4] > 500 && timer[4]) { // display every 1/2 second
    if(tog[0]){
      Serial.println(" ");
      Serial.print("State:-----");
      Serial.println(STATE);
      Serial.print("Bank Pressures:");
      Serial.print(bank1Pt);Serial.print(", ");Serial.print(bank2Pt);Serial.print(", ");Serial.print(bank3Pt);Serial.print(", ");Serial.println(manPt);
      Serial.print("50bar VehiclePSI:");
      Serial.println(pt50);
      Serial.print("350bar VehiclePSI:");
      Serial.println(pt350);
      Serial.print("700bar VehiclePSI:");
      Serial.println(pt700);
      Serial.print("TargetPressure:- ");
      Serial.println((pressureSetting == 700)?TARGET_PRESSURE_700BAR:TARGET_PRESSURE_350BAR);
      Serial.println(" ");
      timer[4] = millis();
    }
    if(tog[1]){
      Serial.print(STATE);
      Serial.print(",");
      Serial.print(pt50);
      Serial.print(",");
      Serial.print(pt350);
      Serial.print(",");
      Serial.print(pt700);
      Serial.print(",");
      Serial.print(bank1Pt);
      Serial.print(",");
      Serial.print(bank2Pt);
      Serial.print(",");
      Serial.print(bank3Pt);
      Serial.print(",");
      Serial.print(manPt);
      Serial.print(",");
      Serial.print(activeBank);
      Serial.print(",");
      Serial.print(targetPressure);
      Serial.print(",");
      Serial.print(CARR_MOVING_AVG_SIZE);
      Serial.print(",");
      Serial.print(DELTA_PSI_TIME);
      Serial.print(",");
      Serial.print(MIN_ALLOWED_PRESSURE);
      Serial.print(",");
      Serial.print(STALL_THRESHOLD);
      Serial.print(",");
      Serial.print(H2_SENSOR_LIMIT);
      Serial.print(",");
      Serial.print(DISP_MOVING_AVG_SIZE);
      Serial.print(",");
      Serial.print(PRESSURE_LOSS_TOL);
      Serial.print(",");
      Serial.print(TARGET_PRESSURE_350BAR);
      Serial.print(",");
      Serial.print(TARGET_PRESSURE_700BAR);
      Serial.print(",");
      Serial.print(currentPressure);
      Serial.print(",");
      Serial.print(pressureSetting);
      Serial.print(",");
      Serial.print(targetPressure);
      Serial.print(",");
      Serial.print(DO_Disp_XV50_LowPressure?1:0);
      Serial.print(",");
      Serial.print(DO_Disp_XV350_MediumPressure?1:0);
      Serial.print(",");
      Serial.print(DO_Disp_XV700_HighPressure?1:0);
      Serial.print(",");
      Serial.print(DO_Disp_XV700_Vent?1:0);
      Serial.print(",");
      Serial.print(DI_Disp_ASL_GSR?1:0);
      Serial.print(",");
      Serial.print(DI_Disp_ASL_LSR?1:0);
      Serial.print(",");
      Serial.print(DI_Disp_ASL_CarrierOK?1:0);
      Serial.print(",");
      Serial.print(DI_Disp_SW01_ButtonGreen?1:0);
      Serial.print(",");
      Serial.print(debouncedGreenButton?1:0);
      Serial.print(",");
      Serial.print(deltaPsi);
      Serial.print(",");
      Serial.print(displayState);
      Serial.print(",");
      Serial.print(displayPage);
      Serial.println();
      timer[4] = millis();
    }
  }
}
