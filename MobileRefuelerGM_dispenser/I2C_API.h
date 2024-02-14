#define TEL_ADDR 0x03
#define DMAND_DO "DMAND"    //Digital Output, Demand
#define RESET_DI "RESET"    //Digital Input, Reset
#define VLPSH_AI "VLPSH"    //Analog Input, vehicle Psi High 700bar
#define VLPSL_AI "VLPSL"    //Analog Input, vehicle Psi Low 50bar
#define INITP_AI "INITP"    //Analog Input, initial vehicle Psi

#define XVHIH_DO "XVHIH"    //Digital Output, dispenser 700bar valve status
#define XVVNT_DO "XVVNT"    //Digital Output, dispenser 700bar VENT valve status
#define XVMED_DO "XVMED"    //Digital Output, dispenser 350bar valve status
#define XVLOW_DO "XVLOW"    //Digital Output, dispenser 50bar valve status

#define PLGRN_DO "PLGRN"    //Digital Output, pilot light green
#define PLAMB_DO "PLAMB"    //Digital Output, pilot light amber
#define PLRED_DO "PLRED"    //Digital Output, pilot light red
#define FNPSI_AI "FNPSI"    //Analog Input, final pressure of vehicle

#define H2LEK_DI "H2LEK"    //Digital Input, H2Leak detected
#define GSRER_DI "GSRER"    //Digital Input, gsr loop error detected
#define LSRER_DI "LSRER"    //Digital Input, lsr look error detected
#define ESTOP_DO "ESTOP"    //Digital Output, ESTOP Status
#define MAXCR_AI "MAXCR"    //Analog  Input, maximum available carrier psi
#define GNBTN_DI "GNBTN"    //Digital Input, green button press
#define RDBTN_DI "RDBTN"    //Digital Input, red button press
#define DMDIN_DI "DMDIN"    //Digital Input, dmd input
#define CAROK_DI "CAROK"    //Digital Input, carrier ok
#define FNSHD_AL "FNSHD"    //Alarm, Finished signal
#define ERRCD_AL "ERRCD"    //Alarm, error code
#define FNSTM_AL "FNSTM"    //finish time

//MOXA #1

//1DI0 = GB
//1DI1 = RB
//1DI2 = LSR
//1DI3 = GSR
//1DI4 = DMD
//1DI5 = Carrier OK

//1R0 = SPARE used to be pump
//1R1 = 50bar Valve
//1R2 = GL
//1R3 = AL
//1R4 = RL
//1R5 = DMD

//2R0 = 350bar Valve (R6)
//2R1 = 700bar Valve (R7)
//2R2 = 700bar vent Valve (R8)

//                  8 7 6        5 4  3 2 1 0  Relay#
//  0  0  0  0   0  0 0 0    0 0 0 0  0 0 0 0  read/write Word
// 15 14 13 12  11 10 9 8    7 6 5 4  3 2 1 0  bit#

