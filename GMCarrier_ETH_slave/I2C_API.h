#define TEL_ADDR 0x03

#define REGRX "REGRX"  //Indicates telemetry module has new values

#define B1SOL_AI "B1SOL"    //Analog Input, B1 Solenoid
#define B2SOL_AI "B2SOL"    //Analog Input, B2 Solenoid

#define LOCP1_AI "LOCP1"    //Analog Input, Local Bank 1 Pressure
#define LOCP2_AI "LOCP2"    //Analog Input, Local Bank 2 Pressure
#define LOCMN_AI "LOCMN"    //Analog Input, Local Manifold Pressure

#define EXTP1_AI "EXTP1"    //Received from Modbus, External Bank 1 Pressure
#define EXTP2_AI "EXTP2"    //Received from Modbus, External Bank 2 Pressure
#define EXTMN_AI "EXTMN"    //Received from Modbus, External Manifold Pressure

#define DMAND_DI "DMAND"    //Digital Input, Demand
#define RESET_DI "RESET"    //Digital Input, Reset
#define PWLED_DO "PWLED"    //Digital Output, Power Good
#define TCACT_DO "TCACT"    //Digital Output, Traco Power Good Status
#define A1ACT_DO "A1ACT"    //Digital Output, Actuator 1 Status
#define A2ACT_DO "A2ACT"    //Digital Output, Actuator 2 Status
#define RTOFL_DO "RTOFL"    //Digital Output, Ready to fill Indicator
#define ESTOP_DO "ESTOP"    //Digital Output, ESTOP Status
#define MBACT_DO "MBACT"    //Digital Output, Modbus Active

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
