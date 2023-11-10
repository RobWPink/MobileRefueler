#include "carrierSlave.h"
#include "I2C_API.h"
SmallMatrix smallMatrix[3] = {SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = {LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

RunningAverage bank1RA(MOVING_AVG_SIZE);
RunningAverage bank2RA(MOVING_AVG_SIZE);
RunningAverage man1RA(MOVING_AVG_SIZE);

void (*resetFunc)(void) = 0;  // Allows hard reset of arduino through software

void setup() {
  TCCR5B = (TCCR5B & 0xF8) | 0x01;
  Wire.begin();
  Wire.setWireTimeout(10000, true);
  pinModeSetup();
  Serial.begin(9600);
 
  matrixSetup("MobileRefueler", "GM V2.1.1S ETH");
   //################## Ethernet Setup ##################
  Ethernet.init(10);
  Ethernet.begin(mac, localIP);
  delay(5);
  if (Ethernet.linkStatus() == LinkOFF || Ethernet.hardwareStatus() == EthernetNoHardware) {
    bool r, s = false;
    if (!timer[0]) { timer[0] = millis(); }
    while (1) {
      if (Ethernet.linkStatus() != LinkOFF && Ethernet.hardwareStatus() != EthernetNoHardware) {
        Serial.println("Ethernet hardware connected.");
        break;
      }
      if (millis() - timer[0] > 10000 && timer[0]) {
        resetFunc();
        timer[0] = 0;
      }
      if (Ethernet.hardwareStatus() == EthernetNoHardware && !r) {
        Serial.println("Ethernet hardware was not found.");
        r = true;
      }
      if (Ethernet.linkStatus() == LinkOFF && !s) {
        Serial.println("Ethernet cable connection not detected.");
        s = true;
      }
    }
  }
  timer[0] = 0;
  ethServer.begin();
  
  // start the Modbus TCP server
  if (!slave.begin()) {
    Serial.println("Failed to start Modbus TCP slave!");
    delay(3000);
    resetFunc();
  }
  slave.configureHoldingRegisters(0,20);

  for (int i = 0; i < MOVING_AVG_SIZE; i++){
    bank1RA.addValue((analogRead(ACT1_PRESS) - 200.0) / 0.084); // Grab pressure sensor values and convert to psi
    bank2RA.addValue((analogRead(ACT2_PRESS) - 200.0) / 0.084); // Add pressure readings to moving average to smooth out data
    man1RA.addValue((analogRead(ACT3_PRESS) - 200.0) / 0.084);
  }
  Serial.println("OK");
}

void loop() {

  dataRead(1000);
  daughterPrint(250);
  SerialCLI();
  EthernetClient client = ethServer.available();
  
  if (client) {
    slave.accept(client);
    while (client.connected()) {
      slave.poll();

      slave.holdingRegisterWrite(0,bank3Pt);
      slave.holdingRegisterWrite(1,bank4Pt);
      slave.holdingRegisterWrite(2,man2Pt);
      slave.holdingRegisterWrite(3,localActiveBank);
      slave.holdingRegisterWrite(4,overrideBank);
      
      activeBank = slave.holdingRegisterRead(10);
      bank1Pt = slave.holdingRegisterRead(11);
      bank2Pt = slave.holdingRegisterRead(12);
      man1Pt = slave.holdingRegisterRead(13);
      int temp = slave.holdingRegisterRead(14);
      if(temp){MOVING_AVG_SIZE = temp;}

      if(activeBank){Serial.println("Master Carrier has overriden Bank control. Please wait until Master is finished"); overrideBank = 0;}

      localActiveBank = activeBank;
      
      if(localActiveBank == 3 || overrideBank == 3){digitalWrite(RLY2, HIGH); digitalWrite(RLY3, LOW);}
      else if(localActiveBank == 4 || overrideBank == 4){digitalWrite(RLY2, LOW); digitalWrite(RLY3, HIGH);}
      else{digitalWrite(RLY2, LOW); digitalWrite(RLY3, LOW);}
    }
    Serial.println();
    Serial.print("Disconnected");
  }
  Serial.print(".");
}
