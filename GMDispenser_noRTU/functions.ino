void flashAmberFunc(unsigned long inter){
  if (!flashTimer1 && flashAmber) { flashTimer1 = millis(); }
  if (millis() - flashTimer1 > inter && flashAmber) {
    tog[2] = !tog[2];
    bitWrite(sendByte, 3, tog[2]);
    flashTimer1 = millis();
  } else if (!flashAmber && flashTimer1) {
    bitClear(sendByte, 3);
    flashTimer1 = 0;
  }
}
void flashRedFunc(unsigned long inter){
  if (!flashTimer2 && flashRed) { flashTimer2 = millis(); }
  if (millis() - flashTimer2 > inter && flashRed) {
    tog[3] = !tog[3];
    bitWrite(sendByte, 4, tog[3]);
    flashTimer2 = millis();
  } else if (!flashRed && flashTimer2) {
    bitClear(sendByte, 4);
    flashTimer2 = 0;
  }
}
void daughterPrint(unsigned long inter){
  if (!daughterTimer) { daughterTimer = millis(); }
  if (millis() - daughterTimer > inter && daughterTimer) {
    daughterTimer = millis();
    checkTCP(false);
    bigMatrix[0].writeInt(currentPressure);  // Write local pressure values to 7-seg screen
    smallMatrix[0].squareDisplay(0);
    bigMatrix[1].writeInt(HMI_targetPressure);
    smallMatrix[1].squareDisplay(0);
    smallMatrix[2].squareDisplay(0);
  }
}
void transceiveTCP() {
  bitWrite(readByte,0,digitalRead(GB));
  bitWrite(readByte,1,digitalRead(RB));
  bitWrite(readByte,2,digitalRead(LSR));
  bitWrite(readByte,3,digitalRead(GSR));
  bitWrite(readByte,4,digitalRead(DMD));
  bitWrite(readByte,5,digitalRead(OK));

  digitalWrite(VALVE,bitRead(sendByte,1));
  digitalWrite(GL,bitRead(sendByte,2));
  digitalWrite(AL,bitRead(sendByte,3));
  digitalWrite(RL,bitRead(sendByte,4));
  //HMI.coilWrite(3,0);
  //HMI.coilWrite(7,bitRead(sendByte,4)); 
  HMI.coilWrite(1,!bitRead(readByte,2)); //lsr
  HMI.coilWrite(2,0);//!bitRead(readByte,3)); //gsr
  if((HMI.coilRead(4) || mb.Hreg(10)) && STATE == IDLE_OFF){
    STATE = OUT_OF_ORDER;
  }

  if(STATE == IDLE_OFF){
    bitWrite(sendByte,0,HMI.coilRead(3)?1:0);
    double timeout = HMI.holdingRegisterRead(11);
    if(timeout && timeout != TIME_OUT){
      TIME_OUT = timeout;
    }
    if(HMI.holdingRegisterRead(13) != modID){resetFunc();}
    double target = HMI.holdingRegisterRead(1);
    if (target && !targetOverride) { HMI_targetPressure = target; }
    if (targetOverride) { HMI_targetPressure = targetOverride; }
  }
  
  //if(HMI.coilRead(13) || HMI.coilRead(14)){Serial.println("received hmi errors 1 and 2"); virtualAmberButton = true;}

  HMI.holdingRegisterWrite(0,displayState);
  HMI.holdingRegisterWrite(2,currentPressure);

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
  pinMode(12, OUTPUT);
  digitalWrite(12, 0);
  pinMode(ESTOP_BREAK, OUTPUT);
  digitalWrite(ESTOP_BREAK, HIGH);

  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  pinMode(TRACO_24VDC, OUTPUT);
  digitalWrite(TRACO_24VDC, HIGH);
}
