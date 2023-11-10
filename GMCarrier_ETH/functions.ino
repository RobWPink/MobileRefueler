void daughterPrint(unsigned long inter){
  if (!daughterTimer) { daughterTimer = millis(); }
  if (millis() - daughterTimer > inter && daughterTimer) {
    daughterTimer = millis();
    bigMatrix[0].writeInt(bank1Pt);  // Write local pressure values to 7-seg screen
    smallMatrix[0].displayChar('1', (activeBank == 1) ? 1 : 0);
    bigMatrix[1].writeInt(bank2Pt);
    smallMatrix[1].displayChar('2', (activeBank == 2) ? 1 : 0);
    bigMatrix[2].writeInt(man1Pt);

    if (activeBank == 3) {smallMatrix[2].displayChar('3', 1);} 
    else if (activeBank == 4) {smallMatrix[2].displayChar('4', 1);} 
    else {smallMatrix[2].displayChar('M', (dmd || dmdOverride) ? 1 : 0);}

    //jsonPrint();
    //fancyPrint();

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

    bank1Pt = bank1RA.getAverage();
    bank2Pt = bank2RA.getAverage();
    man1Pt = man1RA.getAverage();
    
    for (int i = 0; i < 4; i++) {// Store moving average into struct
      if (banks[i].ID == 1) { banks[i].pressure = bank1Pt; }
      else if (banks[i].ID == 2) { banks[i].pressure = bank2Pt; }
      else if (banks[i].ID == 3) { banks[i].pressure = bank3Pt; }
      else if (banks[i].ID == 4) { banks[i].pressure = bank4Pt; }
      else{Serial.println("Error!! Unknown Bank ID detected!!");}
    }
  }
}


void dataSend(unsigned long inter){
  if (!cellTimer) { cellTimer = millis(); }
  if (millis() - cellTimer > inter && cellTimer) {
    cellTimer = millis();
    telWriteValue(TEL_ADDR, BALOC, bank1Pt);
    telWriteValue(TEL_ADDR, BBLOC, bank2Pt);
    telWriteValue(TEL_ADDR, BCEXT, bank3Pt);
    telWriteValue(TEL_ADDR, BDEXT, bank4Pt);
    telWriteValue(TEL_ADDR, MNLOC, man1Pt);
    telWriteValue(TEL_ADDR, MNEXT, man2Pt);
    telWriteValue(TEL_ADDR, ACTBK, (!overrideBank)?activeBank:overrideBank);
  }
}


void transceiveTCP() {
  if(tcpTog){
    master.beginTransmission(HOLDING_REGISTERS,10,5);
    master.write(activeBank);
    master.write(bank1Pt);
    master.write(bank2Pt);
    master.write(man1Pt);
    master.write(MOVING_AVG_SIZE);
    master.endTransmission();
    tcpTog = !tcpTog;
  }
  else{
    master.requestFrom(HOLDING_REGISTERS,0,5);
    bank3Pt = master.read();
    bank4Pt = master.read();
    man2Pt = master.read();
    extActiveBank = master.read();
    overrideBank = master.read();
    tcpTog = !tcpTog;
  }
}


void bubbleSort(struct bank array[], int size) {
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

  // Power
  pinMode(ESTOP_BREAK, OUTPUT);
  digitalWrite(ESTOP_BREAK, HIGH);
// Ethernet
  pinMode(7, OUTPUT );
  digitalWrite(7, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SS, OUTPUT);
  pinMode(RST, OUTPUT);
  digitalWrite(SS, LOW);

  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);
  pinMode(31, OUTPUT);
  digitalWrite(31, 0);
  pinMode(12, OUTPUT);
  digitalWrite(12, 0);
  pinMode(TRACO_24VDC, OUTPUT);
  digitalWrite(TRACO_24VDC, HIGH);

  pinMode(RLY2, OUTPUT);
  digitalWrite(RLY2, LOW);

  pinMode(RLY3, OUTPUT);
  digitalWrite(RLY3, LOW);
}
