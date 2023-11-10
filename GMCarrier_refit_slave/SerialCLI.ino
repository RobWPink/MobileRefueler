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
        overrideBank = (overrideBank == 1)?0:1;
      }
      
      else if(argStr.equalsIgnoreCase("bank2")){
        overrideBank = (overrideBank == 2)?0:2;
      }

      else if(argStr.equalsIgnoreCase("bank3")){
        overrideBank = (overrideBank == 3)?0:3;
      }
  
      else if(argStr.equalsIgnoreCase("bank4")){
        overrideBank = (overrideBank == 4)?0:4;
      }

      else if(argStr.equalsIgnoreCase("json")){
        tog[0] = !tog[0];
      }

      else if(argStr.equalsIgnoreCase("fancy")){
        tog[1] = !tog[1];
      }

      else if(argStr.equalsIgnoreCase("reset")){
        resetFunc();
      }

      else{
        if(!q){ Serial.println("Invalid entry"); }
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
  Serial.println("################## HELP MENU ##################");
  Serial.println("json               -> Print data as json string");
  Serial.println("pretty             -> Print data as human readable format");
  Serial.println("dmd                -> Artifically induce DEMAND signal");
  Serial.println("shutdown           -> Print all NON-tempurature data");
  Serial.println("bank1              -> Toggle Bank 1 solenoid valve");
  Serial.println("bank2              -> Toggle Bank 2 solenoid valve");
  Serial.println("bank3              -> Toggle Bank 3 solenoid valve");
  Serial.println("bank4              -> Toggle Bank 4 solenoid valve");
  Serial.println("reset              -> Arduino soft reset");
  Serial.println("help/h             -> This help menu");
  Serial.println("###############################################");

}

void fancyPrint(){
  if(tog[1]){
    Serial.print("Bank 1: ");
    Serial.print(bank1Pt);
    Serial.println("psi");
    Serial.print("Bank 2: ");
    Serial.print(bank2Pt);
    Serial.println("psi");
    Serial.print("Bank 3: ");
    Serial.print(bank3Pt);
    Serial.println("psi");
    Serial.print("Bank 4: ");
    Serial.print(bank4Pt);
    Serial.println("psi");
    Serial.print("Local Manifold: ");
    Serial.print(man1Pt);
    Serial.println("psi");
    Serial.print("Remote Manifold: ");
    Serial.print(man2Pt);
    Serial.println("psi");
    Serial.print("Local active bank: ");
    Serial.println(localActiveBank);
    Serial.print("Remote active bank: ");
    Serial.println(activeBank);
  }
}
void jsonPrint(){
  if(tog[0]){
    Serial.print("{");
    Serial.print(bank1Pt);
    Serial.print(",");
    Serial.print(bank2Pt);
    Serial.print(",");
    Serial.print(bank3Pt);
    Serial.print(",");
    Serial.print(bank4Pt);
    Serial.print(",");
    Serial.print(man1Pt);
    Serial.print(",");
    Serial.print(man2Pt);
    Serial.print(",");
    Serial.print(localActiveBank);
    Serial.print(",");
    Serial.print(activeBank);
    Serial.print("}");
    Serial.println();
  }
}
