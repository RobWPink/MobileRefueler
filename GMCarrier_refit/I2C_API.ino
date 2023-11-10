#include "I2C_API.h"
union floatToBytes {
  char asBytes[4] = { 0 };
  float asFloat;
};

bool telSetValue(uint8_t telAddr, const char* dsKey, float dsVal) {
  char targetKey[6];
  strcpy(targetKey, dsKey);
  targetKey[5] = '\0';
  telWriteValue(telAddr, targetKey, dsVal);
  Wire.requestFrom((int)telAddr, 10);
  //-------------------------------------------//
  //-----------Verify Command Byte-------------//
  uint8_t cmdByte = Wire.read();
  if (cmdByte != 0xFE) {
    return false;
  }
  //-------------------------------------------//
  //--------Verify Correct Key was Set---------//
  char replyKey[6];
  for (int i = 0; i < 5; ++i) {
    replyKey[i] = Wire.read();
  }
  replyKey[5] = '\0';
  if (!strstr(replyKey, targetKey)) {
    return false;
  }
  //-------------------------------------------//
  //-------Verify Correct Value was Set--------//
  floatToBytes value;
  for (int i = 0; i < 5; ++i) {
    value.asBytes[i] = Wire.read();
  }
  return (dsVal == value.asFloat);
  //-------------------------------------------//
  //-------------------------------------------//
}

float telGetValue(uint8_t telAddr, const char* dsKey) {
  char targetKey[6];
  strcpy(targetKey, dsKey);
  targetKey[5] = '\0';
  Wire.beginTransmission(telAddr);        //Address of Uno used for telemetry
  Wire.write(0xFF);                             //Indicates a WRITE operation
  Wire.write(dsKey);                            //Coorelates to a Storage key
  Wire.endTransmission();  
  Wire.requestFrom((int)telAddr, 10);           //All data should be shared as four bytes
  //-------------------------------------------//
  //-----------Verify Command Byte-------------//
  uint8_t cmdByte = Wire.read();
  if (cmdByte != 0xFF) {
    return -254.0;
  }
  //-------------------------------------------//
  //--------Verify Correct Key was Got---------//
  char replyKey[6];
  for (int i = 0; i < 5; ++i) {
    replyKey[i] = Wire.read();
  }
  replyKey[5] = '\0';
  if (!strstr(replyKey, targetKey)) {
    return -254;
  }
  //-------------------------------------------//
  //-------------Retrieve Value----------------//
  floatToBytes value;
  for (int i = 0; i < 5; ++i) {
    value.asBytes[i] = Wire.read();
  }
  return value.asFloat;
}



int telWriteValue(uint8_t telAddr, char* dsKey, float dsVal) {
  floatToBytes value;
  value.asFloat = dsVal;

  Wire.beginTransmission(telAddr);              //Address of Uno used for telemetry
  Wire.write(0xFE);                             //Indicates a WRITE operation
  Wire.write(dsKey);                            //Coorelates to a Storage key
  Wire.write(value.asBytes[0]);                 //first byte of the value being sent
  Wire.write(value.asBytes[1]);                 //second byte of the value being sent
  Wire.write(value.asBytes[2]);                 //third byte of the value being sent
  Wire.write(value.asBytes[3]);                 //fourth byte of the value being sent
  return Wire.endTransmission();                       //Send message
}
