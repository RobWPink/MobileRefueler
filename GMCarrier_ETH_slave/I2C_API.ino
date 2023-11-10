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


//---------------------------------------------------------------------//
//---------------------------------------------------------------------//
//---------------------------------------------------------------------//
/*
void TX_DB(){
  telSetValue(TEL_ADDR, FTSRF, AI_Ngas_gps_FT132_SRFuelFlowRate);
  telSetValue(TEL_ADDR, PTWTR, AI_Watr_psig_PT383_PumpOutPress);
  telSetValue(TEL_ADDR, PTSHX, AI_Stem_psig_PT318_SteamGenHX406Out);
  telSetValue(TEL_ADDR, PTRFM, AI_Ngas_psig_PT420_EjectOutPress);
  telSetValue(TEL_ADDR, PTSTM, AI_Stem_psig_PT304_SteamPress);
  
  telSetValue(TEL_ADDR, BLRSP, AO_Burn_pct_BL508_AirBlowerSpeed);
  telSetValue(TEL_ADDR, BRNPS, AO_Ngas_pct_FCV134_BurnFuelValvePosition);
  telSetValue(TEL_ADDR, SRFPS, AO_Ngas_pct_FCV141_SRFuelValvePosition);
  telSetValue(TEL_ADDR, WTRPS, AO_Watr_pct_FCV205_HX402WaterCtrl);

  //telSetValue(TEL_ADDR, DIBLR, DI_Burn_BL508_AirBlowerOn);
  telSetValue(TEL_ADDR, DIBAM, DI_Burn_BMM_BMMAlarm);
  //telSetValue(TEL_ADDR, DIBPF, DI_Burn_BMM_BMMProofOfFlame);
  //telSetValue(TEL_ADDR, DIH2D, DI_Comm_H2DemandRemote);
  telSetValue(TEL_ADDR, DIBNA, DI_Encl_SW01_ButtonAmber);
  telSetValue(TEL_ADDR, DIBNG, DI_Encl_SW02_ButtonGreen);
  //telSetValue(TEL_ADDR, DIVFD, DI_Encl_VFD_VFDFault);
  telSetValue(TEL_ADDR, DIPSH, DI_Ngas_PSH117_MainFuelPressHi);
  telSetValue(TEL_ADDR, DIPSL, DI_Ngas_PSL104_MainFuelPressLo);
  telSetValue(TEL_ADDR, DIZSL, DI_Ngas_ZSL111_MainFuelValvePOCb);
  //telSetValue(TEL_ADDR, DIMDB, DI_ModbusCriticalOK);

  telSetValue(TEL_ADDR, DOBLR, DO_Burn_BL508_AirBlowerEnable);
  telSetValue(TEL_ADDR, DOBMM, DO_Burn_BMM_BMMRun);
  telSetValue(TEL_ADDR, DOCLV, DO_Burn_XV501_SecondaryAirValve);
  telSetValue(TEL_ADDR, DOIDL, DO_Comm_IdleHold);
  telSetValue(TEL_ADDR, DOPLG, DO_Encl_LED02_LampGreen);
  telSetValue(TEL_ADDR, DOPLA, DO_Encl_LED01_LampAmber);
  telSetValue(TEL_ADDR, DOEXV, DO_Lref_XV474_ReformerExhaustValve);
  telSetValue(TEL_ADDR, DONGV, DO_N2_XV1100_NitrogenPurgeValve);
  telSetValue(TEL_ADDR, DOSRV, DO_Ngas_XV122_SRFuelFlowValve);
  telSetValue(TEL_ADDR, DOPMP, DO_Watr_PMP204_ROPump);
  telSetValue(TEL_ADDR, DOTWS, DO_Stem_TWV308_SteamControlValve);
  telSetValue(TEL_ADDR, DOTWR, DO_Stem_TWV310_SRControlValve);


  if(-1 < Calc_Lref_C_SRTubeTempMin < 10000){
    telSetValue(TEL_ADDR, CLCAV, Calc_Lref_C_SRTubeTempMean);
    telSetValue(TEL_ADDR, CLCMX, Calc_Lref_C_SRTubeTempMax);
    telSetValue(TEL_ADDR, CLCMN, Calc_Lref_C_SRTubeTempMin);
  }

  telSetValue(TEL_ADDR, TTFBB, round(Temp_Burn_C_TT511_SiCOut));
  telSetValue(TEL_ADDR, TTFBC, round(Temp_Burn_C_TT512_SiCOut));
  telSetValue(TEL_ADDR, TTFBD, round(Temp_Burn_C_TT513_BurnHX504In));
  telSetValue(TEL_ADDR, TTFBE, round(Temp_Burn_C_TT514_BurnHX504Out));
  telSetValue(TEL_ADDR, TTEAH, round(Temp_Lref_C_TT407_SROut));
  telSetValue(TEL_ADDR, TTEAI, round(Temp_Lref_C_TT408_HTSIn));
  telSetValue(TEL_ADDR, TTEBA, round(Temp_Lref_C_TT410_HTSOut));
  telSetValue(TEL_ADDR, TTEBB, round(Temp_Lref_C_TT411_HX406Out));
  telSetValue(TEL_ADDR, TTEBF, round(Temp_Lref_C_TT415_HX452Out));
  telSetValue(TEL_ADDR, TTEDA, round(Temp_Lref_C_TT430_SRIn));
  telSetValue(TEL_ADDR, TTEEB, round(Temp_Lref_C_TT441_SRTube1Exit));

  telSetValue(TEL_ADDR, TTEEC, round(Temp_Lref_C_TT442_SRTube2Exit));
  telSetValue(TEL_ADDR, TTEED, round(Temp_Lref_C_TT443_SRTube3Exit));
  telSetValue(TEL_ADDR, TTEEE, round(Temp_Lref_C_TT444_SRTube4Exit));
  telSetValue(TEL_ADDR, TTEEF, round(Temp_Lref_C_TT445_SRTube5Exit));
  telSetValue(TEL_ADDR, TTEEG, round(Temp_Lref_C_TT446_SRTube6Exit));
  telSetValue(TEL_ADDR, TTEEH, round(Temp_Lref_C_TT447_SRTube7Exit));
  telSetValue(TEL_ADDR, TTEEI, round(Temp_Lref_C_TT448_SRTube8Exit));
  telSetValue(TEL_ADDR, TTEEJ, round(Temp_Lref_C_TT449_SRTube9Exit));
  telSetValue(TEL_ADDR, TTBEC, round(Temp_Ngas_C_TT142_EjectorIn));
  telSetValue(TEL_ADDR, TTDAB, round(Temp_Stem_C_TT301_SteamGenHX406Out));
  telSetValue(TEL_ADDR, TTDAD, round(Temp_Stem_C_TT303_SteamGenHX504Out));
  telSetValue(TEL_ADDR, TTDAG, round(Temp_Stem_C_TT306_EjectorIn));
  telSetValue(TEL_ADDR, TTDBD, round(Temp_Stem_C_TT313_HX406Inlet));
  telSetValue(TEL_ADDR, TTDBJ, round(Temp_Stem_C_TT319_HX402SteamOut));

}
*/