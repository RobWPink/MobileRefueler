#define TEL_ADDR 0x03

#define REGRX "REGRX"  //Indicates telemetry module has new values

#define BALOC "BALOC"    //Analog Input, Local Bank 1 Pressure
#define BBLOC "BBLOC"    //Analog Input, Local Bank 2 Pressure
#define MNLOC "MNLOC"    //Analog Input, Local Manifold Pressure

#define BCEXT "BCEXT"    //Received from Modbus, External Bank 1 Pressure
#define BDEXT "BDEXT"    //Received from Modbus, External Bank 2 Pressure
#define MNEXT "MNEXT"    //Received from Modbus, External Manifold Pressure

#define ACTBK "ACTBK"
#define DMAND "DMAND"    //Digital Input, Demand


//---------------------------------------------------------------------------------------------------------------------//
//-----------------------------------------Values received from dashboard----------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------//
#define HLKTH_CI "HLKTH"    //Ambient hydrogen threshold for leak detection
#define MXPRD_CI "MXPRD"    //Maximum allowable PSI jump, exceeding this indicates a fault
#define MXHOP_CI "MXHOP"    //Maximum discharge pressure allowed in hose
#define MNALP_CI "MNALP"    //Minimum Allowable pressure within tank
#define SOLHZ_CI "SOLHZ"    //Frequency of solenoid pwm, used to prevent overheating
#define MVAVG_CI "MVAVG"    //Number of samples used by moving average filters
#define BNKTH_CI "BNKTH"    //The point (measured in grams per second) where we shift to another bank
