void flashAmberFunc(unsigned long inter){
  if (!flashTimer1 && flashAmber) { flashTimer1 = millis(); }
  if (millis() - flashTimer1 > inter && flashAmber) {
    tog[2] = !tog[2];
    digitalWrite(AL, tog[2]?HIGH:LOW);
    flashTimer1 = millis();
  } else if (!flashAmber && flashTimer1) {
    digitalWrite(AL, LOW);
    flashTimer1 = 0;
  }
}
void flashRedFunc(unsigned long inter){
  if (!flashTimer2 && flashRed) { flashTimer2 = millis(); }
  if (millis() - flashTimer2 > inter && flashRed) {
    tog[3] = !tog[3];
    digitalWrite(RL, tog[3]?HIGH:LOW);
    flashTimer2 = millis();
  } else if (!flashRed && flashTimer2) {
    digitalWrite(RL, LOW);
    flashTimer2 = 0;
  }
}
void daughterPrint(unsigned long inter){
  if (!daughterTimer) { daughterTimer = millis(); }
  if (millis() - daughterTimer > inter && daughterTimer) {
    daughterTimer = millis();
    //checkTCP(false);
    bigMatrix[0].writeInt(currentPressure);  // Write local pressure values to 7-seg screen
    smallMatrix[0].squareDisplay(0);
   // bigMatrix[1].writeInt(HMI_targetPressure);
    smallMatrix[1].squareDisplay(0);
    smallMatrix[2].squareDisplay(0);
  }
}
void transceiveTCP() {
  
  HMI.coilWrite(0,!digitalRead(LSR));//lsr
  HMI.holdingRegisterWrite(0,displayState);
  HMI.holdingRegisterWrite(1,currentPressure);

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

  pinMode(GB, INPUT_PULLUP);
  pinMode(RB, INPUT_PULLUP);
  pinMode(LSR, INPUT_PULLUP);
//  pinMode(ESTOP, INPUT_PULLUP);

  pinMode(VALVE, OUTPUT);
  digitalWrite(VALVE,LOW);
  
  pinMode(GL, OUTPUT);
  digitalWrite(GL,LOW);
  
  pinMode(AL, OUTPUT);
  digitalWrite(AL,LOW);

  pinMode(RL, OUTPUT);
  digitalWrite(RL,LOW);
}
