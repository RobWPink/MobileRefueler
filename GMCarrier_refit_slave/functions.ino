void daughterPrint(unsigned long inter){
  if (!daughterTimer) { daughterTimer = millis(); }
  if (millis() - daughterTimer > inter && daughterTimer) {
    daughterTimer = millis();
    bigMatrix[0].writeInt(bank3Pt);  // Write local pressure values to 7-seg screen
    smallMatrix[0].displayChar('3', (activeBank == 3) ? 1 : 0);
    bigMatrix[1].writeInt(bank4Pt);
    smallMatrix[1].displayChar('4', (activeBank == 4) ? 1 : 0);
    bigMatrix[2].writeInt(man2Pt);

    if (activeBank == 1) {smallMatrix[2].displayChar('1', 1);} 
    else if (activeBank == 2) {smallMatrix[2].displayChar('2', 1);} 
    else {smallMatrix[2].displayChar('M', (dmd) ? 1 : 0);}

    jsonPrint();
    fancyPrint();

    lcd.setCursor(12, 1);
    lcd.print(vehPsi);
    lcd.display();
  }
}


void dataRead(unsigned long inter){
  if (!dataTimer) { dataTimer = millis(); }
  if (millis() - dataTimer > inter && dataTimer) {
    dataTimer = millis();
    long aa = (analogRead(ACT1_PRESS) - 200.0) / 0.084;
    long bb = (analogRead(ACT2_PRESS) - 200.0) / 0.084;
    long cc = (analogRead(ACT3_PRESS) - 200.0) / 0.084;
    if (aa > 10) { bank1RA.addValue(aa); }  // Grab pressure sensor values and convert to psi
    if (bb > 10) { bank2RA.addValue(bb); }  // Add pressure readings to moving average to smooth out data
    if (cc > 10) { man1RA.addValue(cc); }

    bank3Pt = bank1RA.getAverage();
    bank4Pt = bank2RA.getAverage();
    man2Pt = man1RA.getAverage();
    
  }
}


void matrixSetup(String s1, String s2) {
  //begin matrices
  for (int i = 0; i < 3; i++) {
    smallMatrix[i].begin();
    bigMatrix[i].begin();
  }

  //begin LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(s1);
  lcd.setCursor(0, 1);
  lcd.print(s2);
  lcd.setBacklight(150);
}

void closeEverything() { // Close all valves
  digitalWrite(RLY2, LOW);
  digitalWrite(RLY3, LOW);
}


void pinModeSetup() {
  // Inputs
  pinMode(DEMAND, INPUT_PULLUP);
  pinMode(OK, INPUT);

  // Power
  pinMode(ESTOP_BREAK, OUTPUT);
  digitalWrite(ESTOP_BREAK, HIGH);

  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  pinMode(TRACO_24VDC, OUTPUT);
  digitalWrite(TRACO_24VDC, HIGH);


/*// Ethernet
  pinMode(7, OUTPUT );
  digitalWrite(7, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SS, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(SS, LOW);
  digitalWrite(CS, HIGH);
*///-------
/*

  // Relays
  pinMode(RLY0,OUTPUT);
  digitalWrite(RLY0, LOW);
  
  pinMode(RLY1,OUTPUT);
  digitalWrite(RLY1, LOW);
*/
  pinMode(RLY2,OUTPUT);
  digitalWrite(RLY2, LOW);

  pinMode(RLY3,OUTPUT);
  digitalWrite(RLY3, LOW);
/*
  pinMode(RLY4,OUTPUT);
  digitalWrite(RLY4, LOW);

  pinMode(RLY5,OUTPUT);
  digitalWrite(RLY5, LOW);

  pinMode(RLY6,OUTPUT);
  digitalWrite(RLY6, LOW);

  pinMode(RLY7,OUTPUT);
  digitalWrite(RLY7, LOW);

  pinMode(RLY8,OUTPUT);
  digitalWrite(RLY8, LOW);

  pinMode(RLY9,OUTPUT);
  digitalWrite(RLY8, LOW);

  pinMode(RLY10,OUTPUT);
  digitalWrite(RLY8, LOW);

  pinMode(RLY11,OUTPUT);
  digitalWrite(RLY8, LOW);


  // Actuators
  pinMode(ACT1_CLOCK, OUTPUT);
  digitalWrite(ACT1_CLOCK, HIGH);

  pinMode(ACT1_CTRL, OUTPUT);
  digitalWrite(ACT1_CTRL, HIGH);

  pinMode(ACT1_CW, OUTPUT);
  digitalWrite(ACT1_CW, HIGH);

  pinMode(ACT1_ENABLE, OUTPUT);
  digitalWrite(ACT1_ENABLE, LOW);

  pinMode(ACT1_HALF, OUTPUT);
  digitalWrite(ACT1_HALF, LOW);

  pinMode(ACT1_HOME, OUTPUT);
  digitalWrite(ACT1_HOME, LOW);

  pinMode(ACT1_RESET, OUTPUT);
  digitalWrite(ACT1_RESET, HIGH);
  digitalWrite(ACT1_RESET, LOW);
  digitalWrite(ACT1_RESET, HIGH);
  digitalWrite(ACT1_RESET, LOW);

  pinMode(ACT1_VREF, OUTPUT);
  digitalWrite(ACT1_VREF, LOW);

  
  pinMode(ACT2_CLOCK, OUTPUT);
  digitalWrite(ACT2_CLOCK, HIGH);

  pinMode(ACT2_CTRL, OUTPUT);
  digitalWrite(ACT2_CTRL, HIGH);

  pinMode(ACT2_CW, OUTPUT);
  digitalWrite(ACT2_CW, HIGH);

  pinMode(ACT2_ENABLE, OUTPUT);
  digitalWrite(ACT2_ENABLE, LOW);

  pinMode(ACT2_HALF, OUTPUT);
  digitalWrite(ACT2_HALF, LOW);

  pinMode(ACT2_HOME, OUTPUT);
  digitalWrite(ACT2_HOME, LOW);

  pinMode(ACT2_RESET, OUTPUT);
  digitalWrite(ACT2_RESET, HIGH);
  digitalWrite(ACT2_RESET, LOW);
  digitalWrite(ACT2_RESET, HIGH);
  digitalWrite(ACT2_RESET, LOW);
  
  pinMode(ACT2_VREF, OUTPUT);
  digitalWrite(ACT2_VREF, LOW);


  pinMode(ACT3_CLOCK, OUTPUT);
  digitalWrite(ACT3_CLOCK, HIGH);

  pinMode(ACT3_CTRL, OUTPUT);
  digitalWrite(ACT3_CTRL, HIGH);

  pinMode(ACT3_CW, OUTPUT);
  digitalWrite(ACT3_CW, HIGH);

  pinMode(ACT3_ENABLE, OUTPUT);
  digitalWrite(ACT3_ENABLE, LOW);

  pinMode(ACT3_HALF, OUTPUT);
  digitalWrite(ACT3_HALF, LOW);

  pinMode(ACT3_HOME, OUTPUT);
  digitalWrite(ACT3_HOME, LOW);

  pinMode(ACT3_RESET, OUTPUT);
  digitalWrite(ACT3_RESET, HIGH);
  digitalWrite(ACT3_RESET, LOW);
  digitalWrite(ACT3_RESET, HIGH);
  digitalWrite(ACT3_RESET, LOW);
  
  pinMode(ACT3_VREF, OUTPUT);
  digitalWrite(ACT3_VREF, LOW);
  */
}
