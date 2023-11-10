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
      if(argStr.equalsIgnoreCase("target")){
        String argStrVal = argBuf[++n];
        argVal = argStrVal.toDouble();
        targetOverride = argVal;
      }
      else if(argStr.equalsIgnoreCase("green")){
        virtualGreenButton = !virtualGreenButton;
      }
      else if(argStr.equalsIgnoreCase("red")){
        virtualRedButton = !virtualRedButton;
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