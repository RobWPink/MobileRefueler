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
        tg[0] = !tg[0];
        digitalWrite(RLY2, tg[0]?HIGH:LOW);
      }
      else if(argStr.equalsIgnoreCase("bank2")){
        tg[1] = !tg[1];
        digitalWrite(RLY3, tg[1]?HIGH:LOW);
      }
      else if(argStr.equalsIgnoreCase("bank3")){
        tg[2] = !tg[2];
        digitalWrite(RLY4, tg[2]?HIGH:LOW);
      }
      else if(argStr.equalsIgnoreCase("manual")){
        manual = !manual;
      }

      else if(argStr.equalsIgnoreCase("data")){
        tog[0] = !tog[0];
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

void printOuts(){
  
  if(!timer[4]){timer[4] = millis();}
  if (millis() - timer[4] > 500 && timer[4]) { // display every 1/2 second
    if(tog[0]){
      Serial.print("Bank1: ");
      Serial.println(bank1Pt);
      Serial.print("Bank2: ");
      Serial.println(bank2Pt);
      Serial.print("Bank3: ");
      Serial.println(bank3Pt);
      Serial.print("Manifold: ");
      Serial.println(manPt);
      Serial.print("Vehicle Pressure: ");
      Serial.println(vehPsi);
      Serial.println();
      timer[4] = millis();
    }
  }
}
