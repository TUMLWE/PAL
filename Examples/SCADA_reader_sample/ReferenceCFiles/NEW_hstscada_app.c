/**
********************************************************************************
* @file     hstscada_app.c
* @author   Bachmann electronic GmbH
* @version  $Revision: 3.90 $ $LastChangedBy: BE $
* @date     $LastChangeDate: 2013-06-10 11:00:00 $
*
* @brief    This file contains the application algorithms
*           and the application specific SVI interface.
*
********************************************************************************
* COPYRIGHT BY BACHMANN ELECTRONIC GmbH 2013
*******************************************************************************/

/* VxWorks includes */
#include <vxWorks.h>
#include <taskLib.h>
#include <tickLib.h>
#include <intLib.h>
#include <semLib.h>
#include <sysLib.h>
#include <inetLib.h>
#include <string.h>
#include <stdio.h>
#include <symLib.h>
#include <sysSymTbl.h>

/* MSys includes */
#include <mtypes.h>
#include <msys_e.h>
#include <mio.h>
#include <mio_e.h>
#include <res_e.h>
#include <svi_e.h>
#include <log_e.h>
#include <prof_e.h>
#include <lst_e.h>

/* Project includes */
#include "hstscada.h"
#include "hstscada_e.h"
#include "hstscada_int.h"

/* Functions: administration, to be called from outside this file */
SINT32  hstscada_AppEOI(VOID);
VOID    hstscada_AppDeinit(VOID);
SINT32  hstscada_CfgRead(VOID);
SINT32  hstscada_SviSrvInit(VOID);
VOID    hstscada_SviSrvDeinit(VOID);

/* Functions: administration, to be called only from within this file */
MLOCAL VOID hstscada_CfgInit(VOID);

/* Functions: task administration, being called only within this file */
MLOCAL SINT32 Task_CreateAll(VOID);
MLOCAL VOID Task_DeleteAll(VOID);
MLOCAL SINT32 Task_CfgRead(VOID);
MLOCAL SINT32 Task_InitTiming(TASK_PROPERTIES * pTaskData);
MLOCAL SINT32 Task_InitTiming_Tick(TASK_PROPERTIES * pTaskData);
MLOCAL SINT32 Task_InitTiming_Sync(TASK_PROPERTIES * pTaskData);
MLOCAL VOID Task_WaitCycle(TASK_PROPERTIES * pTaskData);

/* Functions: worker task "Control" */
MLOCAL VOID Control_Main(TASK_PROPERTIES * pTaskData);
MLOCAL VOID Control_CycleInit(VOID);
MLOCAL VOID Control_CycleStart(VOID);
MLOCAL VOID Control_Cycle(VOID);
MLOCAL VOID Control_CycleEnd(TASK_PROPERTIES * pTaskData);

/* Functions: SVI client */
MLOCAL SINT32 SviClt_Example(UINT32 * pTime_us);
MLOCAL SINT32 SviClt_Init(VOID);
MLOCAL VOID SviClt_Deinit(VOID);


MLOCAL SINT32 SviClt_Read();
MLOCAL SINT32 SviClt_Write(VOID);

/* Global variables: data structure for mconfig parameters */
HSTSCADA_BASE_PARMS hstscada_BaseParams;

/* Global variables: SVI client */
MLOCAL  SINT32(**pSviLib) () = NULL;    /* Information about external SVI server */
MLOCAL SVI_ADDR TimeSviAddr;            /* SVI address of server's variable */
MLOCAL BOOL8 PrintTime = TRUE;



// MATLABCODEGEN: OpenField SVI Server Definition

MLOCAL SINT32  (**pSviLib_ifcloads)  ()= NULL;
MLOCAL SINT32  (**pSviLib_ifcmm)  ()= NULL;
MLOCAL SINT32  (**pSviLib_ifcscada)  ()= NULL;

// MATLABCODEGEN: CloseField SVI Server Definition


// MATLABCODEGEN: OpenField SA Address and Variables to be read from interface

MLOCAL SVI_ADDR SA_WT1_Timestamp_UTC;
MLOCAL SVI_ADDR SA_WT1_GeneratorSpeed;
MLOCAL SVI_ADDR SA_WT1_ConverterTorque;
MLOCAL SVI_ADDR SA_WT1_NacelleDirection;
MLOCAL SVI_ADDR SA_WT1_PitchAngle;
MLOCAL SVI_ADDR SA_WT1_PitchAngle1;
MLOCAL SVI_ADDR SA_WT1_PitchAngle2;
MLOCAL SVI_ADDR SA_WT1_PitchAngle3;
MLOCAL SVI_ADDR SA_WT1_Power;
MLOCAL SVI_ADDR SA_WT1_RotorRpm;
MLOCAL SVI_ADDR SA_WT1_TurbineState;
MLOCAL SVI_ADDR SA_WT1_WindSpeed;
MLOCAL SVI_ADDR SA_WT1_YawError;
MLOCAL SVI_ADDR SA_WT1_ref_signal;
MLOCAL SVI_ADDR SA_WT1_isConnected;
MLOCAL SVI_ADDR SA_WT2_Timestamp_UTC;
MLOCAL SVI_ADDR SA_WT2_GeneratorSpeed;
MLOCAL SVI_ADDR SA_WT2_ConverterTorque;
MLOCAL SVI_ADDR SA_WT2_NacelleDirection;
MLOCAL SVI_ADDR SA_WT2_PitchAngle;
MLOCAL SVI_ADDR SA_WT2_PitchAngle1;
MLOCAL SVI_ADDR SA_WT2_PitchAngle2;
MLOCAL SVI_ADDR SA_WT2_PitchAngle3;
MLOCAL SVI_ADDR SA_WT2_Power;
MLOCAL SVI_ADDR SA_WT2_RotorRpm;
MLOCAL SVI_ADDR SA_WT2_TurbineState;
MLOCAL SVI_ADDR SA_WT2_WindSpeed;
MLOCAL SVI_ADDR SA_WT2_YawError;
MLOCAL SVI_ADDR SA_WT2_ref_signal;
MLOCAL SVI_ADDR SA_WT2_isConnected;
MLOCAL SVI_ADDR SA_WT3_Timestamp_UTC;
MLOCAL SVI_ADDR SA_WT3_GeneratorSpeed;
MLOCAL SVI_ADDR SA_WT3_ConverterTorque;
MLOCAL SVI_ADDR SA_WT3_NacelleDirection;
MLOCAL SVI_ADDR SA_WT3_PitchAngle;
MLOCAL SVI_ADDR SA_WT3_PitchAngle1;
MLOCAL SVI_ADDR SA_WT3_PitchAngle2;
MLOCAL SVI_ADDR SA_WT3_PitchAngle3;
MLOCAL SVI_ADDR SA_WT3_Power;
MLOCAL SVI_ADDR SA_WT3_RotorRpm;
MLOCAL SVI_ADDR SA_WT3_TurbineState;
MLOCAL SVI_ADDR SA_WT3_WindSpeed;
MLOCAL SVI_ADDR SA_WT3_YawError;
MLOCAL SVI_ADDR SA_WT3_ref_signal;
MLOCAL SVI_ADDR SA_WT3_isConnected;
REAL64 itfc_scada_WT1XYZvibrations[3];
MLOCAL SVI_ADDR SA_itfc_scada_WT1XYZvibrations;
REAL64 itfc_scada_WT2XYZvibrations[3];
MLOCAL SVI_ADDR SA_itfc_scada_WT2XYZvibrations;
REAL64 itfc_scada_WT3XYZvibrations[3];
MLOCAL SVI_ADDR SA_itfc_scada_WT3XYZvibrations;
MLOCAL SVI_ADDR SA_mm_Timestamp_UTC;
MLOCAL SVI_ADDR SA_mm_ws_110m;
MLOCAL SVI_ADDR SA_mm_ws_60m;
MLOCAL SVI_ADDR SA_mm_wd_110m;
MLOCAL SVI_ADDR SA_mm_wd_60m;
MLOCAL SVI_ADDR SA_mm_temperature;
MLOCAL SVI_ADDR SA_mm_humidity;
MLOCAL SVI_ADDR SA_mm_pressure;
MLOCAL SVI_ADDR SA_loads_Timestamp_UTC;
MLOCAL SVI_ADDR SA_wt1_blade1_root_edge;
MLOCAL SVI_ADDR SA_wt1_blade1_root_flap;
MLOCAL SVI_ADDR SA_wt1_blade2_root_edge;
MLOCAL SVI_ADDR SA_wt1_blade2_root_flap;
MLOCAL SVI_ADDR SA_wt1_blade3_root_edge;
MLOCAL SVI_ADDR SA_wt1_blade3_root_flap;
MLOCAL SVI_ADDR SA_wt2_blade1_root_edge;
MLOCAL SVI_ADDR SA_wt2_blade1_root_flap;
MLOCAL SVI_ADDR SA_wt2_blade2_root_edge;
MLOCAL SVI_ADDR SA_wt2_blade2_root_flap;
MLOCAL SVI_ADDR SA_wt2_blade3_root_edge;
MLOCAL SVI_ADDR SA_wt2_blade3_root_flap;
MLOCAL SVI_ADDR SA_wt3_blade1_root_edge;
MLOCAL SVI_ADDR SA_wt3_blade1_root_flap;
MLOCAL SVI_ADDR SA_wt3_blade2_root_edge;
MLOCAL SVI_ADDR SA_wt3_blade2_root_flap;
MLOCAL SVI_ADDR SA_wt3_blade3_root_edge;
MLOCAL SVI_ADDR SA_wt3_blade3_root_flap;
REAL64 itfc_scada_exchangedatactrl[10];
MLOCAL SVI_ADDR SA_itfc_scada_exchangedatactrl;
MLOCAL SVI_ADDR SA_wt1_blade1_IP;
MLOCAL SVI_ADDR SA_wt1_blade2_IP;
MLOCAL SVI_ADDR SA_wt1_blade3_IP;
MLOCAL SVI_ADDR SA_wt1_blade1_OOP;
MLOCAL SVI_ADDR SA_wt1_blade2_OOP;
MLOCAL SVI_ADDR SA_wt1_blade3_OOP;

// MATLABCODEGEN: CloseField SA Address and Variables to be read from interface

/* Global variables: miscellaneous */
char output_FileName_main[1024] = "host_scada_";
char output_path[1024] = "/usbBulk0/SCADA/";
MLOCAL UINT32 newfile_time_s =60;




// Buffer and Writer Task Settings
MLOCAL UINT32 FAST_FREQ_RATIO = 1;       // Ratio between AppCklRate and Fast Buffer
MLOCAL UINT32 SLOW_FREQ_RATIO = 10;      // Ratio between AppCklRate and Slow Buffer
MLOCAL UINT32 CTRL_FREQ_RATIO = 10;      // Ratio between AppCklRate and Control Buffer



MLOCAL UINT16 Recording_Start = TRUE;      // Needs to be UINT16 in order to be modbus compatible
MLOCAL UINT16 Recording_Start_prev = TRUE;      // Needs to be UINT16 in order to be modbus compatible
MLOCAL UINT32 CycleCount_new_file = 0;
MLOCAL SVI_ADDR Time_ms1970_SviAddr;
UINT64  Time_ms1970;
#define TRUE 1
#define FALSE 0
#define MAXSTRLENGTH 1024
MLOCAL FILE *pFileFast  = NULL;                 // File for fast data recording
MLOCAL FILE *pFileSlow  = NULL;                 // File for slow data recording
MLOCAL FILE *pFileCtrl  = NULL;                 // File for recording controller data
MLOCAL SINT32 write_header_fast();
MLOCAL SINT32 write_output_fast();
MLOCAL SINT32 write_header_slow();
MLOCAL SINT32 write_output_slow();
MLOCAL SINT32 write_header_ctrl();
MLOCAL SINT32 write_output_ctrl();
char output_FileName_fast[MAXSTRLENGTH];
char output_FileName_slow[MAXSTRLENGTH];
char output_FileName_ctrl[MAXSTRLENGTH];
char dateYMD_HMS[MAXSTRLENGTH];
MLOCAL time_t rawtime;
struct tm * timeinfo;
MLOCAL REAL32 CycleTime_ms;
MLOCAL UINT32 offset_newfile;
MLOCAL BOOL8 create_new_file_fast = FALSE;
MLOCAL BOOL8 create_new_file_slow = FALSE;
MLOCAL BOOL8 create_new_file_ctrl = FALSE;
MLOCAL CHAR Date_and_Time[24] = "2023.04.24_19:41:00:000";




MLOCAL UINT32 CycleCount = 0;


// MATLABCODEGEN: OpenField SVI Variables Definition

MLOCAL CHAR WT1_Timestamp_UTC[32];
MLOCAL REAL64 WT1_GeneratorSpeed=0;
MLOCAL REAL64 WT1_ConverterTorque=0;
MLOCAL REAL64 WT1_NacelleDirection=0;
MLOCAL REAL64 WT1_PitchAngle=0;
MLOCAL REAL64 WT1_PitchAngle1=0;
MLOCAL REAL64 WT1_PitchAngle2=0;
MLOCAL REAL64 WT1_PitchAngle3=0;
MLOCAL REAL64 WT1_Power=0;
MLOCAL REAL64 WT1_RotorRpm=0;
MLOCAL UINT16 WT1_TurbineState=0;
MLOCAL REAL64 WT1_WindSpeed=0;
MLOCAL REAL64 WT1_YawError=0;
MLOCAL REAL32 WT1_ref_signal=0;
MLOCAL BOOL8 WT1_isConnected=0;
MLOCAL CHAR WT2_Timestamp_UTC[32];
MLOCAL REAL64 WT2_GeneratorSpeed=0;
MLOCAL REAL64 WT2_ConverterTorque=0;
MLOCAL REAL64 WT2_NacelleDirection=0;
MLOCAL REAL64 WT2_PitchAngle=0;
MLOCAL REAL64 WT2_PitchAngle1=0;
MLOCAL REAL64 WT2_PitchAngle2=0;
MLOCAL REAL64 WT2_PitchAngle3=0;
MLOCAL REAL64 WT2_Power=0;
MLOCAL REAL64 WT2_RotorRpm=0;
MLOCAL UINT16 WT2_TurbineState=0;
MLOCAL REAL64 WT2_WindSpeed=0;
MLOCAL REAL64 WT2_YawError=0;
MLOCAL REAL32 WT2_ref_signal=0;
MLOCAL BOOL8 WT2_isConnected=0;
MLOCAL CHAR WT3_Timestamp_UTC[32];
MLOCAL REAL64 WT3_GeneratorSpeed=0;
MLOCAL REAL64 WT3_ConverterTorque=0;
MLOCAL REAL64 WT3_NacelleDirection=0;
MLOCAL REAL64 WT3_PitchAngle=0;
MLOCAL REAL64 WT3_PitchAngle1=0;
MLOCAL REAL64 WT3_PitchAngle2=0;
MLOCAL REAL64 WT3_PitchAngle3=0;
MLOCAL REAL64 WT3_Power=0;
MLOCAL REAL64 WT3_RotorRpm=0;
MLOCAL UINT16 WT3_TurbineState=0;
MLOCAL REAL64 WT3_WindSpeed=0;
MLOCAL REAL64 WT3_YawError=0;
MLOCAL REAL32 WT3_ref_signal=0;
MLOCAL BOOL8 WT3_isConnected=0;
MLOCAL REAL64 WT1_X_vibration=0;
MLOCAL REAL64 WT1_Y_vibration=0;
MLOCAL REAL64 WT1_Z_vibration=0;
MLOCAL REAL64 WT2_X_vibration=0;
MLOCAL REAL64 WT2_Y_vibration=0;
MLOCAL REAL64 WT2_Z_vibration=0;
MLOCAL REAL64 WT3_X_vibration=0;
MLOCAL REAL64 WT3_Y_vibration=0;
MLOCAL REAL64 WT3_Z_vibration=0;
MLOCAL CHAR mm_Timestamp_UTC[32];
MLOCAL REAL64 mm_ws_110m=0;
MLOCAL REAL64 mm_ws_60m=0;
MLOCAL REAL64 mm_wd_110m=0;
MLOCAL REAL64 mm_wd_60m=0;
MLOCAL REAL64 mm_temperature=0;
MLOCAL REAL64 mm_humidity=0;
MLOCAL REAL64 mm_pressure=0;
MLOCAL CHAR loads_Timestamp_UTC[32];
MLOCAL REAL64 wt1_blade1_root_edge=0;
MLOCAL REAL64 wt1_blade1_root_flap=0;
MLOCAL REAL64 wt1_blade2_root_edge=0;
MLOCAL REAL64 wt1_blade2_root_flap=0;
MLOCAL REAL64 wt1_blade3_root_edge=0;
MLOCAL REAL64 wt1_blade3_root_flap=0;
MLOCAL REAL64 wt2_blade1_root_edge=0;
MLOCAL REAL64 wt2_blade1_root_flap=0;
MLOCAL REAL64 wt2_blade2_root_edge=0;
MLOCAL REAL64 wt2_blade2_root_flap=0;
MLOCAL REAL64 wt2_blade3_root_edge=0;
MLOCAL REAL64 wt2_blade3_root_flap=0;
MLOCAL REAL64 wt3_blade1_root_edge=0;
MLOCAL REAL64 wt3_blade1_root_flap=0;
MLOCAL REAL64 wt3_blade2_root_edge=0;
MLOCAL REAL64 wt3_blade2_root_flap=0;
MLOCAL REAL64 wt3_blade3_root_edge=0;
MLOCAL REAL64 wt3_blade3_root_flap=0;
MLOCAL REAL64 check_isWFon=1;
MLOCAL REAL64 check_WindInSector=1;
MLOCAL REAL64 check_all=1;
MLOCAL REAL64 wt1_blade1_IP=0;
MLOCAL REAL64 wt1_blade2_IP=0;
MLOCAL REAL64 wt1_blade3_IP=0;
MLOCAL REAL64 wt1_blade1_OOP=0;
MLOCAL REAL64 wt1_blade2_OOP=0;
MLOCAL REAL64 wt1_blade3_OOP=0;
MLOCAL REAL64 avg_TI=0;
MLOCAL REAL64 avg_ws_110m=0;
MLOCAL REAL64 avg_ws_60m=0;
MLOCAL REAL64 avg_shearExp=0;
MLOCAL REAL64 avg_wd_110m=0;
MLOCAL REAL64 avg_inflowState=0;
MLOCAL UINT16 avg_inflow_AppStatus=0;
MLOCAL UINT16 loads_process_AppStatus=0;
MLOCAL UINT16 WF_status_AppStatus=0;

// MATLABCODEGEN: CloseField SVI Variables Definition

/*
 * Global variables: Settings for application task
 * A reference to these settings must be registered in TaskList[], see below.
 * If no configuration group is being specified, all values must be set properly
 * in this initialization.
 */
MLOCAL TASK_PROPERTIES TaskProperties_aControl = {
    "aHSTSCADA_Ctrl",                 /* unique task name, maximum length 14 */
    "ControlTask",                      /* configuration group name */
    Control_Main,                       /* task entry function (function pointer) */
    0,                                  /* default task priority (->Task_CfgRead) */
    100,                               /* default task cycle time in ms (->Task_CfgRead) */
    0,                                  /* default task time base (->Task_CfgRead, 0=tick, 1=sync) */
    5,                                  /* default ratio of watchdog time / cycle time
                                         * (->Task_CfgRead) */
    10000,                              /* task stack size in bytes, standard size is 10000 */
    TRUE                                /* task uses floating point operations */
};

/*
 * Global variables: List of all application tasks
 * TaskList[] is being used for all task administration functions.
 */
MLOCAL TASK_PROPERTIES *TaskList[] = {
    &TaskProperties_aControl
};

/*
 * Global variables: SVI server variables list
 * The following variables will be exported to the SVI of the module.
 * Variables in SviGlobVarList will be directly accessible.
 * Those in SviVirtVarList via read/write function pointers.
 * The settings in the SviGlobVarList are:
 * Visible name of SVI variable, max. length = SVI_ADDRLEN
 * - Visible format and access rights (see defines SVI_F_xxx from svi.h)
 * - Size of variable in bytes
 * - Pointer to variable in the software project
 */
MLOCAL SVI_GLOBVAR SviGlobVarList[] = {
    {"CycleCounter", SVI_F_INOUT | SVI_F_UINT32, sizeof(UINT32), (UINT32 *) & CycleCount, 0, NULL,
     NULL}
    ,
    {"ModuleVersion", SVI_F_OUT | SVI_F_STRING, sizeof(hstscada_Version),
     (UINT32 *) hstscada_Version, 0, NULL, NULL}

,
    {"TimeStamp_UTC", SVI_F_OUT   | SVI_F_STRING, sizeof(Date_and_Time),  (UINT32 *) Date_and_Time},
     {"Recording/Flag_Record", SVI_F_INOUT   | SVI_F_UINT16, sizeof(UINT16), (UINT32*) &Recording_Start},




// MATLABCODEGEN: OpenField SVI Variables Coupling

{"WT1_Timestamp_UTC", SVI_F_INOUT | SVI_F_STRING, sizeof(CHAR[32]), (UINT32 *) &WT1_Timestamp_UTC, 0, NULL, NULL},
{"WT1_GeneratorSpeed", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_GeneratorSpeed, 0, NULL, NULL},
{"WT1_ConverterTorque", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_ConverterTorque, 0, NULL, NULL},
{"WT1_NacelleDirection", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_NacelleDirection, 0, NULL, NULL},
{"WT1_PitchAngle", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_PitchAngle, 0, NULL, NULL},
{"WT1_PitchAngle1", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_PitchAngle1, 0, NULL, NULL},
{"WT1_PitchAngle2", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_PitchAngle2, 0, NULL, NULL},
{"WT1_PitchAngle3", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_PitchAngle3, 0, NULL, NULL},
{"WT1_Power", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_Power, 0, NULL, NULL},
{"WT1_RotorRpm", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_RotorRpm, 0, NULL, NULL},
{"WT1_TurbineState", SVI_F_INOUT | SVI_F_UINT16, sizeof(UINT16[1]), (UINT32 *) &WT1_TurbineState, 0, NULL, NULL},
{"WT1_WindSpeed", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_WindSpeed, 0, NULL, NULL},
{"WT1_YawError", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_YawError, 0, NULL, NULL},
{"WT1_ref_signal", SVI_F_INOUT | SVI_F_REAL32, sizeof(REAL32[1]), (UINT32 *) &WT1_ref_signal, 0, NULL, NULL},
{"WT1_isConnected", SVI_F_INOUT | SVI_F_BOOL8, sizeof(BOOL8[1]), (UINT32 *) &WT1_isConnected, 0, NULL, NULL},
{"WT2_Timestamp_UTC", SVI_F_INOUT | SVI_F_STRING, sizeof(CHAR[32]), (UINT32 *) &WT2_Timestamp_UTC, 0, NULL, NULL},
{"WT2_GeneratorSpeed", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_GeneratorSpeed, 0, NULL, NULL},
{"WT2_ConverterTorque", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_ConverterTorque, 0, NULL, NULL},
{"WT2_NacelleDirection", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_NacelleDirection, 0, NULL, NULL},
{"WT2_PitchAngle", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_PitchAngle, 0, NULL, NULL},
{"WT2_PitchAngle1", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_PitchAngle1, 0, NULL, NULL},
{"WT2_PitchAngle2", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_PitchAngle2, 0, NULL, NULL},
{"WT2_PitchAngle3", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_PitchAngle3, 0, NULL, NULL},
{"WT2_Power", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_Power, 0, NULL, NULL},
{"WT2_RotorRpm", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_RotorRpm, 0, NULL, NULL},
{"WT2_TurbineState", SVI_F_INOUT | SVI_F_UINT16, sizeof(UINT16[1]), (UINT32 *) &WT2_TurbineState, 0, NULL, NULL},
{"WT2_WindSpeed", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_WindSpeed, 0, NULL, NULL},
{"WT2_YawError", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_YawError, 0, NULL, NULL},
{"WT2_ref_signal", SVI_F_INOUT | SVI_F_REAL32, sizeof(REAL32[1]), (UINT32 *) &WT2_ref_signal, 0, NULL, NULL},
{"WT2_isConnected", SVI_F_INOUT | SVI_F_BOOL8, sizeof(BOOL8[1]), (UINT32 *) &WT2_isConnected, 0, NULL, NULL},
{"WT3_Timestamp_UTC", SVI_F_INOUT | SVI_F_STRING, sizeof(CHAR[32]), (UINT32 *) &WT3_Timestamp_UTC, 0, NULL, NULL},
{"WT3_GeneratorSpeed", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_GeneratorSpeed, 0, NULL, NULL},
{"WT3_ConverterTorque", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_ConverterTorque, 0, NULL, NULL},
{"WT3_NacelleDirection", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_NacelleDirection, 0, NULL, NULL},
{"WT3_PitchAngle", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_PitchAngle, 0, NULL, NULL},
{"WT3_PitchAngle1", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_PitchAngle1, 0, NULL, NULL},
{"WT3_PitchAngle2", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_PitchAngle2, 0, NULL, NULL},
{"WT3_PitchAngle3", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_PitchAngle3, 0, NULL, NULL},
{"WT3_Power", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_Power, 0, NULL, NULL},
{"WT3_RotorRpm", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_RotorRpm, 0, NULL, NULL},
{"WT3_TurbineState", SVI_F_INOUT | SVI_F_UINT16, sizeof(UINT16[1]), (UINT32 *) &WT3_TurbineState, 0, NULL, NULL},
{"WT3_WindSpeed", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_WindSpeed, 0, NULL, NULL},
{"WT3_YawError", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_YawError, 0, NULL, NULL},
{"WT3_ref_signal", SVI_F_INOUT | SVI_F_REAL32, sizeof(REAL32[1]), (UINT32 *) &WT3_ref_signal, 0, NULL, NULL},
{"WT3_isConnected", SVI_F_INOUT | SVI_F_BOOL8, sizeof(BOOL8[1]), (UINT32 *) &WT3_isConnected, 0, NULL, NULL},
{"WT1_X_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_X_vibration, 0, NULL, NULL},
{"WT1_Y_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_Y_vibration, 0, NULL, NULL},
{"WT1_Z_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT1_Z_vibration, 0, NULL, NULL},
{"WT2_X_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_X_vibration, 0, NULL, NULL},
{"WT2_Y_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_Y_vibration, 0, NULL, NULL},
{"WT2_Z_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT2_Z_vibration, 0, NULL, NULL},
{"WT3_X_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_X_vibration, 0, NULL, NULL},
{"WT3_Y_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_Y_vibration, 0, NULL, NULL},
{"WT3_Z_vibration", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &WT3_Z_vibration, 0, NULL, NULL},
{"mm_Timestamp_UTC", SVI_F_INOUT | SVI_F_STRING, sizeof(CHAR[32]), (UINT32 *) &mm_Timestamp_UTC, 0, NULL, NULL},
{"mm_ws_110m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &mm_ws_110m, 0, NULL, NULL},
{"mm_ws_60m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &mm_ws_60m, 0, NULL, NULL},
{"mm_wd_110m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &mm_wd_110m, 0, NULL, NULL},
{"mm_wd_60m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &mm_wd_60m, 0, NULL, NULL},
{"mm_temperature", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &mm_temperature, 0, NULL, NULL},
{"mm_humidity", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &mm_humidity, 0, NULL, NULL},
{"mm_pressure", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &mm_pressure, 0, NULL, NULL},
{"loads_Timestamp_UTC", SVI_F_INOUT | SVI_F_STRING, sizeof(CHAR[32]), (UINT32 *) &loads_Timestamp_UTC, 0, NULL, NULL},
{"wt1_blade1_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade1_root_edge, 0, NULL, NULL},
{"wt1_blade1_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade1_root_flap, 0, NULL, NULL},
{"wt1_blade2_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade2_root_edge, 0, NULL, NULL},
{"wt1_blade2_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade2_root_flap, 0, NULL, NULL},
{"wt1_blade3_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade3_root_edge, 0, NULL, NULL},
{"wt1_blade3_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade3_root_flap, 0, NULL, NULL},
{"wt2_blade1_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt2_blade1_root_edge, 0, NULL, NULL},
{"wt2_blade1_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt2_blade1_root_flap, 0, NULL, NULL},
{"wt2_blade2_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt2_blade2_root_edge, 0, NULL, NULL},
{"wt2_blade2_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt2_blade2_root_flap, 0, NULL, NULL},
{"wt2_blade3_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt2_blade3_root_edge, 0, NULL, NULL},
{"wt2_blade3_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt2_blade3_root_flap, 0, NULL, NULL},
{"wt3_blade1_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt3_blade1_root_edge, 0, NULL, NULL},
{"wt3_blade1_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt3_blade1_root_flap, 0, NULL, NULL},
{"wt3_blade2_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt3_blade2_root_edge, 0, NULL, NULL},
{"wt3_blade2_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt3_blade2_root_flap, 0, NULL, NULL},
{"wt3_blade3_root_edge", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt3_blade3_root_edge, 0, NULL, NULL},
{"wt3_blade3_root_flap", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt3_blade3_root_flap, 0, NULL, NULL},
{"check_isWFon", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &check_isWFon, 0, NULL, NULL},
{"check_WindInSector", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &check_WindInSector, 0, NULL, NULL},
{"check_all", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &check_all, 0, NULL, NULL},
{"wt1_blade1_IP", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade1_IP, 0, NULL, NULL},
{"wt1_blade2_IP", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade2_IP, 0, NULL, NULL},
{"wt1_blade3_IP", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade3_IP, 0, NULL, NULL},
{"wt1_blade1_OOP", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade1_OOP, 0, NULL, NULL},
{"wt1_blade2_OOP", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade2_OOP, 0, NULL, NULL},
{"wt1_blade3_OOP", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &wt1_blade3_OOP, 0, NULL, NULL},
{"avg_TI", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &avg_TI, 0, NULL, NULL},
{"avg_ws_110m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &avg_ws_110m, 0, NULL, NULL},
{"avg_ws_60m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &avg_ws_60m, 0, NULL, NULL},
{"avg_shearExp", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &avg_shearExp, 0, NULL, NULL},
{"avg_wd_110m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &avg_wd_110m, 0, NULL, NULL},
{"avg_inflowState", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64, sizeof(REAL64[1]), (UINT32 *) &avg_inflowState, 0, NULL, NULL},
{"avg_inflow_AppStatus", SVI_F_INOUT | SVI_F_UINT16, sizeof(UINT16[1]), (UINT32 *) &avg_inflow_AppStatus, 0, NULL, NULL},
{"loads_process_AppStatus", SVI_F_INOUT | SVI_F_UINT16, sizeof(UINT16[1]), (UINT32 *) &loads_process_AppStatus, 0, NULL, NULL},
{"WF_status_AppStatus", SVI_F_INOUT | SVI_F_UINT16, sizeof(UINT16[1]), (UINT32 *) &WF_status_AppStatus, 0, NULL, NULL},

// MATLABCODEGEN: CloseField SVI Variables Coupling

};

/**
********************************************************************************
* @brief Main entry function of the aTask of the application.
*        The input parameter is being passed by the task spawn call.
*
* @param[in]  pointer to task properties data structure
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID Control_Main(TASK_PROPERTIES * pTaskData)
{
    /* Initialization upon task entry */
    Control_CycleInit();

    /*
     * This loop is executed endlessly
     * as long as there is no request to quit the task
     */
    while (!pTaskData->Quit)
    {
        /* cycle start administration */
        Control_CycleStart();

        /* operational code */
        Control_Cycle();

        /* cycle end administration */
        Control_CycleEnd(pTaskData);
    }
}

/**
********************************************************************************
* @brief Administration code to be called once before first cycle start.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID Control_CycleInit(VOID)
{
    CycleTime_ms = TaskProperties_aControl.CycleTime_ms;
    offset_newfile = newfile_time_s / CycleTime_ms * 1000;




    /* TODO: add what is necessary before cyclic operation starts */

}

/**
********************************************************************************
* @brief Administration code to be called once at each task cycle start.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID Control_CycleStart(VOID)
{
SINT32 ret;

if (SviClt_Read() < 0)
    LOG_W(0, "Control_CycleStart", "Could not read all SVI variables!");


// MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI

WT1_X_vibration = itfc_scada_WT1XYZvibrations[0];
WT1_Y_vibration = itfc_scada_WT1XYZvibrations[1];
WT1_Z_vibration = itfc_scada_WT1XYZvibrations[2];
WT2_X_vibration = itfc_scada_WT2XYZvibrations[0];
WT2_Y_vibration = itfc_scada_WT2XYZvibrations[1];
WT2_Z_vibration = itfc_scada_WT2XYZvibrations[2];
WT3_X_vibration = itfc_scada_WT3XYZvibrations[0];
WT3_Y_vibration = itfc_scada_WT3XYZvibrations[1];
WT3_Z_vibration = itfc_scada_WT3XYZvibrations[2];

// MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI

// MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI

itfc_scada_exchangedatactrl[0] = check_isWFon;
itfc_scada_exchangedatactrl[1] = check_WindInSector;
itfc_scada_exchangedatactrl[2] = check_all;

// MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI



time ( &rawtime );
timeinfo = gmtime( &rawtime );
sprintf(Date_and_Time, "%d.%02d.%02d_%02d:%02d:%02d:%03lld", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, Time_ms1970 % 1000);

//    filename_hostdata
sprintf(dateYMD_HMS,"%d%02d%02d_%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
sprintf(output_FileName_fast,"%s%s_%s_FAST.dat", output_path, dateYMD_HMS, output_FileName_main);
sprintf(output_FileName_slow,"%s%s_%s_SLOW.dat", output_path, dateYMD_HMS, output_FileName_main);
sprintf(output_FileName_ctrl,"%s%s_%s_CTRL.dat", output_path, dateYMD_HMS, output_FileName_main);


// Check if Recording_Start has changed, and if it went off i should close all open files
if (Recording_Start == FALSE && Recording_Start_prev == TRUE ) {
    if (pFileFast != NULL) {fclose(pFileFast);}
    if (pFileSlow != NULL) {fclose(pFileSlow);}
    if (pFileCtrl != NULL) {fclose(pFileCtrl);}
} else if (Recording_Start == TRUE && Recording_Start_prev == FALSE ) { // if i just turned on the recording, create new files
    CycleCount_new_file = CycleCount;
}

// Shall we generate new files? (has time elapsed?)
if (CycleCount >= CycleCount_new_file && Recording_Start)
{
    create_new_file_fast = TRUE;
    create_new_file_slow = TRUE;
    create_new_file_ctrl = TRUE;

    CycleCount_new_file = CycleCount + offset_newfile;
}


// manage fast file
if ((CycleCount % FAST_FREQ_RATIO) == 0 && Recording_Start )  {

    if (create_new_file_fast) {
        if (pFileFast != NULL) {fclose(pFileFast);}
        pFileFast = fopen( output_FileName_fast, "w");
        if (pFileFast == NULL)
        {
            LOG_W(0, "Control_CycleStart", "Cannot open fast file!");
        } else {
        ret = write_header_fast();
        }
        create_new_file_fast = FALSE;
    };

    if (pFileFast != NULL) ret = write_output_fast();
    else LOG_W(0, "Control_CycleStart", "Cannot write into fast file!");

};

if ((CycleCount % SLOW_FREQ_RATIO) == 0) {

    if (create_new_file_slow) {
        if (pFileSlow != NULL) {fclose(pFileSlow);}
        pFileSlow = fopen( output_FileName_slow, "w");
        if (pFileSlow == NULL)
        {
            LOG_W(0, "Control_CycleStart", "Cannot open slow file!");
        } else {
        ret = write_header_slow();
        }
        create_new_file_slow = FALSE;
    };

    if (pFileSlow != NULL) ret = write_output_slow();
    else LOG_W(0, "Control_CycleStart", "Cannot write into slow file!");

};

if ((CycleCount % CTRL_FREQ_RATIO) == 0) {

    if (create_new_file_ctrl) {
        if (pFileCtrl != NULL) {fclose(pFileCtrl);}
        pFileCtrl = fopen( output_FileName_ctrl, "w");
        if (pFileCtrl == NULL)
        {
            LOG_W(0, "Control_CycleStart", "Cannot open ctrl file!");
        } else {
        ret = write_header_ctrl();
        }
        create_new_file_ctrl = FALSE;
    };

    if (pFileCtrl != NULL) ret = write_output_ctrl();
    else LOG_W(0, "Control_CycleStart", "Cannot write into ctrl file!");

};




    /* TODO: add what is necessary at each cycle start */

}

/**
********************************************************************************
* @brief Cyclic application code.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID Control_Cycle(VOID)
{
    UINT32  Time_us;

    /* TODO: add operational code to be called in this task */

    /* Increase cycle counter */
    CycleCount++;

    /*
     * In this example, all values are read in a separated function.
     * It is also possible to read values on demand only.
     */
    if (SviClt_Example(&Time_us) < 0)
        LOG_W(0, "Control_Cycle", "Could not read SVI variable");
    else if (PrintTime)
    {
        LOG_I(0, "Control_Cycle", "%u microseconds since CPU boot ...", Time_us);
        PrintTime = FALSE;
    }

}

/**
********************************************************************************
* @brief Administration code to be called at each task cycle end
*
* @param[in]  pointer to task properties data structure
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID Control_CycleEnd(TASK_PROPERTIES * pTaskData)
{
    SviClt_Write();



    Recording_Start_prev = Recording_Start;




    /* TODO: add what is to be called at each cycle end */

    /*
     * This is the very end of the cycle
     * Delay task in order to match desired cycle time
     */
    Task_WaitCycle(pTaskData);
}

/**
********************************************************************************
* @brief Performs the second phase of the module initialization.
* 		  Called at "End Of Init" by the bTask.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
SINT32 hstscada_AppEOI(VOID)
{

    /* do while(0), to be left as soon as there is an error */
    do
    {
        /* TODO: set module info string, maximum length is SMI_DESCLEN_A */
        snprintf(hstscada_ModuleInfoDesc, sizeof(hstscada_ModuleInfoDesc), "TODO: set application specific module info string");

        /* TODO: add all initializations required by your application */

        /* Initialize SVI access to variables of other software modules */
        if (SviClt_Init() < 0)
            break;

        /* Start all application tasks listed in TaskList */
        if (Task_CreateAll() < 0)
            break;

        /* At this point, all init actions are done successfully */
        return (OK);
    }
    while (0);

    /*
     * At this point, an init action returned an error.
     * The application code is being de-initialized.
     */
    hstscada_AppDeinit();
    return (ERROR);
}

/**
********************************************************************************
* @brief Frees all resources of the application
*        Called at De-Init of the module by the bTask.
*        The function does not quit on an error.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
VOID hstscada_AppDeinit(VOID)
{

    /* TODO: Free all resources which have been allocated by the application */

    /* Delete all application tasks listed in TaskList */
    Task_DeleteAll();

    /* Delete all SVI client access data */
    SviClt_Deinit();

}

/**
********************************************************************************
* @brief Reads the settings from configuration file mconfig
*        for all tasks registered in TaskList[].
*        The task name in TaskList[] is being used as configuration group name.
*        The initialization values in TaskList[] are being used as default values.
*        For general configuration data, hstscada_CfgParams is being used.
*        Being called by hstscada_CfgRead.
*        All parameters are stored in the task properties data structure.
*        All parameters are being treated as optional.
*        There is no limitation checking of the parameters, the limits are being
*        specified in the cru and checked by the configurator.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_CfgRead(VOID)
{
    UINT32  idx;
    UINT32  NbOfTasks = sizeof(TaskList) / sizeof(TASK_PROPERTIES *);
    SINT32  ret;
    CHAR    section[PF_KEYLEN_A];
    CHAR    group[PF_KEYLEN_A];
    CHAR    key[PF_KEYLEN_A];
    SINT32  TmpVal;
    CHAR    TmpStrg[32];
    UINT32  Error = FALSE;
    CHAR    Func[] = "Task_CfgRead";

    /* section name is the application name, for all tasks */
    snprintf(section, sizeof(section), hstscada_BaseParams.AppName);

    /* For all application tasks listed in TaskList */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (!TaskList[idx])
        {
            LOG_E(0, Func, "Invalid task properties pointer in task list entry #%d!", idx);
            Error = TRUE;
            continue;
        }

        /* group name is specified in the task properties */
        snprintf(group, sizeof(group), TaskList[idx]->CfgGroup);

        /* if no group name has been specified: skip configuration reading for this task */
        if (strlen(group) < 1)
        {
            LOG_I(0, Func, "Could not find task configuration for task '%s' in mconfig ",
                  TaskList[idx]->Name);
            continue;
        }

        /*
         * Read the desired value for the task cycle time.
         * If the keyword has not been found, the initialization value remains
         * in the task properties.
         */
        snprintf(key, sizeof(key), "CycleTime");
        snprintf(TmpStrg, sizeof(TmpStrg), "%f", TaskList[idx]->CycleTime_ms);
        ret = pf_GetStrg(section, group, key, "", (CHAR *) & TmpStrg, sizeof(TmpStrg),
                         hstscada_BaseParams.CfgLine, hstscada_BaseParams.CfgFileName);
        /* keyword has been found */
        if (ret >= 0)
        {
            *((REAL32 *) & TaskList[idx]->CycleTime_ms) = atof(TmpStrg);
        }
        /* keyword has not been found */
        else
        {
            LOG_W(0, Func, "Missing configuration parameter '[%s](%s)%s'", section, group, key);
            LOG_W(0, Func, " -> using initialization value of %f ms", TaskList[idx]->CycleTime_ms);
        }

        /*
         * Read the desired value for the task priority.
         * If the keyword has not been found, the initialization value remains
         * in the task properties.
         * As an additional fall back, the priority in the base parms will be used.
         */
        snprintf(key, sizeof(key), "Priority");
        ret = pf_GetInt(section, group, key, TaskList[idx]->Priority, &TmpVal,
                        hstscada_BaseParams.CfgLine, hstscada_BaseParams.CfgFileName);
        /* keyword has been found */
        if (ret >= 0)
        {
            TaskList[idx]->Priority = TmpVal;
        }
        /* keyword has not been found */
        else
        {
            LOG_W(0, Func, "Missing configuration parameter '[%s](%s)%s'", section, group, key);
            if (TaskList[idx]->Priority == 0)
            {
                TaskList[idx]->Priority = hstscada_BaseParams.DefaultPriority;
                LOG_W(0, Func, " -> using base parms value of %d", TaskList[idx]->Priority);
            }
            else
                LOG_W(0, Func, " -> using initialization value of %d", TaskList[idx]->Priority);
        }

        /*
         * Read the software watchdog time ratio.
         * If the keyword has not been found, the initialization value remains
         * in the task properties.
         */
        snprintf(key, sizeof(key), "WatchdogRatio");
        ret = pf_GetInt(section, group, key, TaskList[idx]->WDogRatio, &TmpVal,
                        hstscada_BaseParams.CfgLine, hstscada_BaseParams.CfgFileName);
        /* keyword has been found */
        if (ret >= 0)
        {
            TaskList[idx]->WDogRatio = TmpVal;
        }
        /* keyword has not been found */
        else
        {
            LOG_W(0, Func, "Missing configuration parameter '[%s](%s)%s'", section, group, key);
            LOG_W(0, Func, " -> using initialization value of %d", TaskList[idx]->WDogRatio);
        }

        /*
         * Read the time base for the cycle time.
         * If the keyword has not been found, the initialization value remains
         * in the task properties.
         */
        snprintf(key, sizeof(key), "TimeBase");
        ret = pf_GetInt(section, group, key, TaskList[idx]->TimeBase, &TmpVal,
                        hstscada_BaseParams.CfgLine, hstscada_BaseParams.CfgFileName);
        /* keyword has been found */
        if (ret >= 0)
        {
            TaskList[idx]->TimeBase = TmpVal;
        }
        /* keyword has not been found */
        else
        {
            LOG_W(0, Func, "Missing configuration parameter '[%s](%s)%s'", section, group, key);
            LOG_W(0, Func, " -> using initialization value of %d", TaskList[idx]->TimeBase);
        }
    }

    /* Evaluate overall error flag */
    if (Error)
        return (ERROR);
    else
        return (OK);
}

/**
********************************************************************************
* @brief Starts all tasks which are registered in the global task list
*        - task watchdog is being created if specified
*        - priority is being checked and corrected if necessary
*        - semaphore for cycle timing is being created
*        - sync session is being started if necessary
*        - sync ISR is being attached if necessary
*        If there is an error creating a task, no further tasks will be started.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_CreateAll(VOID)
{
    UINT32  idx;
    UINT8   TaskName[M_TSKNAMELEN_A];
    UINT32  NbOfTasks = sizeof(TaskList) / sizeof(TASK_PROPERTIES *);
    UINT32  TaskOptions;
    UINT32  wdogtime_us;
    CHAR    Func[] = "Task_CreateAll";

    /* For all application tasks listed in TaskList */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (!TaskList[idx])
        {
            LOG_E(0, Func, "Invalid task properties pointer!");
            return (ERROR);
        }

        /* Initialize what is necessary */
        TaskList[idx]->SyncSessionId = ERROR;
        TaskList[idx]->TaskId = ERROR;
        TaskList[idx]->WdogId = 0;
        TaskList[idx]->Quit = FALSE;

        /* Create software watchdog if required */
        if (TaskList[idx]->WDogRatio > 0)
        {
            /* check watchdog ratio, minimum useful value is 2 */
            if (TaskList[idx]->WDogRatio < 3)
            {
                TaskList[idx]->WDogRatio = 3;
                LOG_W(0, Func, "Watchdog ratio increased to 3!");
            }

            wdogtime_us = (TaskList[idx]->CycleTime_ms * 1000) * TaskList[idx]->WDogRatio;
            TaskList[idx]->WdogId = sys_WdogCreate(hstscada_AppName, wdogtime_us);
            if (TaskList[idx]->WdogId == 0)
            {
                LOG_E(0, Func, "Could not create watchdog!");
                return (ERROR);
            }
        }

        /* Create binary semaphore for cycle timing */
        TaskList[idx]->CycleSema = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
        if (!TaskList[idx]->CycleSema)
        {
            LOG_E(0, Func, "Could not create cycle timing semaphore for task '%s'!",
                  TaskList[idx]->Name);
            return (ERROR);
        }

        /* Initialize task cycle timing infrastructure */
        Task_InitTiming(TaskList[idx]);

        /* In case the priority has not been properly set */
        if (TaskList[idx]->Priority == 0)
        {
            LOG_E(0, Func, "Invalid priority for task '%s'", TaskList[idx]->Name);
            return (ERROR);
        }

        /* make sure task name string is terminated */
        TaskList[idx]->Name[M_TSKNAMELEN_A - 2] = 0;

        /* If no task name has been set: use application name and index */
        if (strlen(TaskList[idx]->Name) < 1)
            snprintf(TaskList[idx]->Name, sizeof(TaskList[idx]->Name), "a%s_%d", hstscada_AppName, idx + 1);

        snprintf(TaskName, sizeof(TaskList[idx]->Name), "%s", TaskList[idx]->Name);

        /* Task options */
        TaskOptions = 0;
        if (TaskList[idx]->UseFPU)
            TaskOptions |= VX_FP_TASK;

        /* Spawn task with properties set in task list */
        TaskList[idx]->TaskId = sys_TaskSpawn(hstscada_AppName, TaskName,
                                              TaskList[idx]->Priority, TaskOptions,
                                              TaskList[idx]->StackSize,
                                              (FUNCPTR) TaskList[idx]->pMainFunc, TaskList[idx]);

        /* Check if task has been created successfully */
        if (TaskList[idx]->TaskId == ERROR)
        {
            LOG_E(0, Func, "Error in sys_TaskSpawn for task '%s'!", TaskName);
            return (ERROR);
        }
    }

    /* At this point, all tasks have been started successfully */
    return (OK);
}

/**
********************************************************************************
* @brief Deletes all tasks which are registered in the global task list
*        Undo for all operations in Task_CreateAll
*        The function will not be left upon an error.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID Task_DeleteAll(VOID)
{
    UINT32  idx;
    UINT32  NbOfTasks = sizeof(TaskList) / sizeof(TASK_PROPERTIES *);
    UINT32  RequestTime;
    CHAR    Func[] = "Task_DeleteAll";

    /*
     * Delete software watchdog if present
     * This must be done first, because tasks will end their cyclic operation
     * and thus won't trigger their watchdogs any more.
     */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (TaskList[idx]->WdogId)
            sys_WdogDelete(TaskList[idx]->WdogId);
    }

    /*
     * Set quit request for all existing tasks
     * This should make tasks complete their cycle and quit operation.
     */
    for (idx = 0; idx < NbOfTasks; idx++)
        TaskList[idx]->Quit = TRUE;

    /*
     * Give all cycle semaphores of listed tasks
     * This wakes up all tasks and thus speeds up the completion.
     */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (TaskList[idx]->CycleSema)
            semGive(TaskList[idx]->CycleSema);
    }

    /* Take a time stamp for the timeout check */
    RequestTime = m_GetProcTime();

    /*
     * Wait for all tasks to quit their cycle.
     * Apply a timeout of 500ms
     * Wait one tick in case of missing task quit
     */
    do
    {
        UINT32  AllTasksQuitted = TRUE;

        /* allow one tick for tasks to complete */
        taskDelay(1);

        /* Check if all tasks have terminated their cycles */
        for (idx = 0; idx < NbOfTasks; idx++)
            AllTasksQuitted &= (taskIdVerify(TaskList[idx]->TaskId) == ERROR);

        /* If all tasks have terminated themselves */
        if (AllTasksQuitted)
        {
            if (hstscada_BaseParams.DebugMode & APP_DBG_INFO1)
                LOG_I(0, Func, "All tasks have terminated by themselves");
            break;
        }
        /* If timeout waiting for task self termination is over */
        else if ((m_GetProcTime() - RequestTime) > 500000)
        {
            LOG_W(0, Func, "Timeout at waiting for tasks to terminate by themselves");
            break;
        }
    }
    while (TRUE);

    /* Cleanup resources and delete all remaining tasks */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        /* Stop sync session if present and detach ISR */
        if (TaskList[idx]->SyncSessionId >= 0)
        {
            LOG_I(0, Func, "Stopping sync session for task %s", TaskList[idx]->Name);
            mio_StopSyncSession(TaskList[idx]->SyncSessionId);
        }

        /* Delete semaphore for cycle timing */
        if (TaskList[idx]->CycleSema)
        {
            if (semDelete(TaskList[idx]->CycleSema) < 0)
                LOG_W(0, Func, "Could not delete cycle semaphore of task %s!", TaskList[idx]->Name);
            else
            	TaskList[idx]->CycleSema = 0;
        }

        /* Remove application tasks which still exist */
        if (taskIdVerify(TaskList[idx]->TaskId) == OK)
        {
            if (taskDelete(TaskList[idx]->TaskId) == ERROR)
                LOG_E(0, Func, "Could not delete task %s!", TaskList[idx]->Name);
            else
                LOG_W(0, Func, "Task %s had to be deleted!", TaskList[idx]->Name);

            TaskList[idx]->TaskId = ERROR;
        }
    }
}

/**
********************************************************************************
* @brief Initializes infrastructure for task timing
*
* @param[in]  pointer to task properties data structure
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_InitTiming(TASK_PROPERTIES * pTaskData)
{
    CHAR    Func[] = "Task_InitTiming";

    if (!pTaskData)
    {
        LOG_E(0, Func, "Invalid input pointer!");
        return (ERROR);
    }

    /* independent of timing model */
    pTaskData->NbOfCycleBacklogs = 0;
    pTaskData->NbOfSkippedCycles = 0;

    /* depending on timing model */
    switch (pTaskData->TimeBase)
    {
            /* Tick based timing */
        case 0:
            return (Task_InitTiming_Tick(pTaskData));
            /* Sync based timing */
        case 1:
            pTaskData->SyncSessionId = ERROR;
            return (Task_InitTiming_Sync(pTaskData));
            /* Undefined */
        default:
            LOG_E(0, Func, "Unknown timing model!");
            return (ERROR);
    }
}

/**
********************************************************************************
* @brief Initializes infrastructure for task timing with tick timer
*
* @param[in]  pointer to task properties data structure
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_InitTiming_Tick(TASK_PROPERTIES * pTaskData)
{
    REAL32  TmpReal;
    CHAR    Func[] = "Task_InitTiming_Tick";

    if (!pTaskData)
    {
        LOG_E(0, Func, "Invalid input pointer!");
        return (ERROR);
    }

    /*
     * Calculate and check cycle time in ticks as integer value
     */
    TmpReal = ((pTaskData->CycleTime_ms / 1000.0) * sysClkRateGet()) + 0.5;
    pTaskData->CycleTime = (UINT32) TmpReal;

    /* If cycle time is less than a full tick */
    if (pTaskData->CycleTime < 1)
    {
        pTaskData->CycleTime = 1;
        LOG_W(0, Func, "Cycle time too small for tick rate %d, increased to 1 tick!",
              sysClkRateGet());
    }

    /* Take first cycle start time stamp */
    pTaskData->PrevCycleStart = tickGet();

    /* Initialize cycle time grid */
    pTaskData->NextCycleStart = tickGet() + pTaskData->CycleTime;

    return (OK);
}

/**
********************************************************************************
* @brief Initializes infrastructure for task timing with sync event.
*
* @param[in]  pointer to task properties data structure
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. Error
*******************************************************************************/
MLOCAL SINT32 Task_InitTiming_Sync(TASK_PROPERTIES * pTaskData)
{
    SINT32  ret;
    REAL32  SyncCycle_us, TmpReal;
    SYS_CPUINFO CpuInfo;
    CHAR    Func[] = "Task_InitTiming_Sync";

    if (!pTaskData)
    {
        LOG_E(0, Func, "Invalid input pointer!");
        return (ERROR);
    }

    /* Start sync session for this module (multiple starts are possible) */
    pTaskData->SyncSessionId = mio_StartSyncSession(hstscada_AppName);
    if (pTaskData->SyncSessionId < 0)
    {
        LOG_E(0, Func, "Could not start sync session for task '%s'!", pTaskData->Name);
        return (ERROR);
    }

    /* Get cpu info, contains sync timer settings (always returns OK) */
    sys_GetCpuInfo(&CpuInfo);

    /* Calculate and check period time of sync timer */
    SyncCycle_us = CpuInfo.pExtCpuInfo->SyncHigh + CpuInfo.pExtCpuInfo->SyncLow;
    if ((SyncCycle_us == 0) || (CpuInfo.pExtCpuInfo->SyncHigh == 0)
        || (CpuInfo.pExtCpuInfo->SyncLow == 0))
    {
        LOG_E(0, Func, "System sync configuration invalid, can't use sync for task '%s'!",
              pTaskData->Name);
        return (ERROR);
    }

    /* Calculate multiple of specified task cycle time */
    TmpReal = ((pTaskData->CycleTime_ms * 1000) / SyncCycle_us) + 0.5;
    pTaskData->CycleTime = TmpReal;

    /* If cycle time is less than a full sync */
    if (pTaskData->CycleTime < 1)
    {
        pTaskData->CycleTime = 1;
        LOG_W(0, Func, "Cycle time too small for sync cycle %d us, increased to 1 sync!",
              SyncCycle_us);
    }

    /*
     * For application tasks,
     * MIO_SYNC_IN (falling edge of sync signal) is the normal option.
     */
    pTaskData->SyncEdge = MIO_SYNC_IN;

    /*
     * Attach semGive to sync event
     * -> semGive will be called according to the sync attach settings below.
     * -> semGive will give the semaphore pTaskData->CycleSema.
     * -> the task will be triggered as soon as this semaphore is given.
     */
    ret = mio_AttachSync(pTaskData->SyncSessionId,      /* from mio_StartSyncSession */
                         pTaskData->SyncEdge,   /* selection of sync edge */
                         pTaskData->CycleTime,  /* number of sync cycles */
                         (VOID *) semGive,      /* register semGive as ISR */
                         (UINT32) pTaskData->CycleSema);        /* semaphore id for semGive */
    if (ret < 0)
    {
        LOG_W(0, Func, "Could not attach to sync for task '%s'!", pTaskData->Name);
        if (pTaskData->SyncSessionId >= 0)
        	mio_StopSyncSession(pTaskData->SyncSessionId);
        pTaskData->SyncSessionId = ERROR;
        return (ERROR);
    }

    return (OK);
}

/**
********************************************************************************
* @brief Performs the necessary wait time for the specified cycle.
*        The wait time results from cycle time minus own run time.
*        NOTE: The time unit depends on the used time base (ticks or sync periods).
*
* @param[in]  pointer to task properties data structure
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID Task_WaitCycle(TASK_PROPERTIES * pTaskData)
{
    UINT32  PrevCycleStart = 0;
    UINT32  NextCycleStart = 0;
    UINT32  CycleTime = 0;
    UINT32  TimeToWait = 0;
    UINT32  SkipNow = 0;
    UINT32  TimeNow = 0;
    UINT32  Backlog = 0;
    UINT32  MaxBacklog = 0;
    UINT32  CyclesSkipped = 0;

    /* Emergency behavior in case of missing task settings */
    if (!pTaskData)
    {
        LOG_E(0, "Task_WaitCycle", "Invalid input pointer, using alternative taskDelay(1000)");
        taskDelay(1000);
        return;
    }

    /* Trigger software watchdog if existing */
    if (pTaskData->WdogId)
        sys_WdogTrigg(pTaskData->WdogId);

    /*
     * Handle tick based cycle timing ("Time" unit is ticks)
     */
    if (pTaskData->TimeBase == 0)
    {
        /* Use local variables to make calculation as compressed as possible */
        PrevCycleStart = pTaskData->PrevCycleStart;
        NextCycleStart = pTaskData->NextCycleStart;
        CycleTime = pTaskData->CycleTime;
        MaxBacklog = CycleTime * 2;

        /* Calculate the time to wait before the next cycle can start. */
        NextCycleStart = PrevCycleStart + CycleTime;
        PrevCycleStart = NextCycleStart;

        /*
         * As soon as the current time stamp has been taken,
         * the code until semTake processing should be kept as short as possible,
         * so that the probability of being interrupted is as small as possible.
         */
        TimeNow = tickGet();

        /* This is the amount of until the next scheduled cycle start. */
        TimeToWait = NextCycleStart - TimeNow;

        /* Limit wait time to minimum 1 */
        if (!TimeToWait)
            TimeToWait++;

        /*
         * As long as the next scheduled cycle start lies in the future,
         * the resulting wait time must be smaller than the cycle time.
         * If the resulting wait time is higher than the cycle time,
         * the next scheduled cycle start already lies in the past.
         * This means that there is a cycle backlog.
         * NOTE: since the wait time is an unsigned value, a negative difference
         * is being interpreted as a large positive value.
         */
        if (TimeToWait > CycleTime)
        {
            /* Calculate cycle backlog */
            Backlog = TimeNow - NextCycleStart;

            /* As long as the backlog is below the limit */
            if (Backlog <= MaxBacklog)
            {
                /* Try to catch, but still use a small task delay */
                TimeToWait = 1;
            }
            /* If the backlog is beyond the limit */
            else
            {
                /* Skip the backlog and recalculate next cycle start */
                SkipNow = (Backlog / CycleTime) + 1;
                NextCycleStart = NextCycleStart + (SkipNow * CycleTime);
                PrevCycleStart = NextCycleStart;
                TimeToWait = NextCycleStart - TimeNow;
                CyclesSkipped += SkipNow;
            }
        }
    }

    /*
     * Handle sync based cycle timing ("Time" unit is syncs)
     */
    else if (pTaskData->TimeBase == 1)
    {
        /*
         * In case of sync timing, it is assumed, that the sync interrupt
         * directly determines the cycle time.
         * It would also be possible to use a multiple of sync's as cycle time.
         * The logic necessary for that is not yet implemented.
         */
        TimeToWait = WAIT_FOREVER;
    }

    /* Register cycle end in system timing statistics */
    sys_CycleEnd();

    /*
     * Wait for the calculated number of time units
     * by taking the cycle semaphore with a calculated timeout
     */
    semTake(pTaskData->CycleSema, TimeToWait);

    /*
     * Waiting for the cycle semaphore has now timed out in case of tick
     * or been given in case of sync.
     */

    /* Register cycle start in system timing statistics */
    sys_CycleStart();

    /*
     * The above logic uses local variables in order to keep the processing short.
     * Some of these local variables must be rescued for the next call of this
     * function.
     */
    pTaskData->PrevCycleStart = PrevCycleStart;
    pTaskData->NextCycleStart = NextCycleStart;
    pTaskData->NbOfSkippedCycles += CyclesSkipped;
    if (Backlog)
        pTaskData->NbOfCycleBacklogs++;

    /*
     * Consideration of software module state
     * If the module is in stop or eoi state,
     * all cyclic tasks of this module shall be stopped.
     * If the software module receives the RpcStart call,
     * it will give the state semaphore, and all tasks will continue.
     */
    if ((hstscada_ModState == RES_S_STOP) || (hstscada_ModState == RES_S_EOI))
    {
        /* Disable software watchdog if present */
        if (pTaskData->WdogId)
            sys_WdogDisable(pTaskData->WdogId);

        LOG_I(0, "Task_WaitCycle", "Stopping task '%s' due to module stop", pTaskData->Name);

        /*
         * semaphore will be given by SMI server with calls
         * RpcStart or RpcEndOfInit
         */
        semTake(hstscada_StateSema, WAIT_FOREVER);
    }
}

/**
********************************************************************************
* @brief Initializes the module configuration data structure
*        with the values obtained in the module init.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
MLOCAL VOID hstscada_CfgInit(VOID)
{
    /* Configuration file name (profile name) */
    strncpy(hstscada_BaseParams.CfgFileName, hstscada_ProfileName, M_PATHLEN);

    /* Application name */
    strncpy(hstscada_BaseParams.AppName, hstscada_AppName, M_MODNAMELEN);

    /* Line number in configuration file (used as start line for searching) */
    hstscada_BaseParams.CfgLine = hstscada_CfgLine;

    /* Worker task priority from module base parameters (BaseParms) */
    hstscada_BaseParams.DefaultPriority = hstscada_AppPrio;

    /* Debug mode from module base parameters (BaseParms) */
    hstscada_BaseParams.DebugMode = hstscada_Debug;

}

/**
********************************************************************************
* @brief Calls all configuration read functions of the application
*        Being called at module init by the bTask.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
SINT32 hstscada_CfgRead(VOID)
{
    SINT32  ret;

    /* Initialize configuration with values taken at module init */
    hstscada_CfgInit();

    /* Read all task configuration settings from mconfig.ini */
    ret = Task_CfgRead();
    if (ret < 0)
        return ret;

    /*
     * TODO:
     * Call other specific configuration read functions here
     */

    return (OK);
}

/**
********************************************************************************
* @brief Initializes the SVI-Interface.
*        Being called at module init by the bTask.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
SINT32 hstscada_SviSrvInit(VOID)
{
    SINT32  ret;
    UINT32  NbOfGlobVars = sizeof(SviGlobVarList) / sizeof(SVI_GLOBVAR);
    UINT32  i;
    CHAR    Func[] = "hstscada_SviSrvInit";

    /* If there are any SVI variables to be exported */
    if (NbOfGlobVars)
    {
        /* Initialize SVI-handler */
        hstscada_SviHandle = svi_Init(hstscada_AppName, 0, 0);
        if (!hstscada_SviHandle)
        {
            LOG_E(0, Func, "Could not initialize SVI server handle!");
            return (ERROR);
        }
    }
    else
    {
        hstscada_SviHandle = 0;
        return (OK);
    }

    /* Add the global variables from the list SviGlobVarList */
    for (i = 0; i < NbOfGlobVars; i++)
    {
        ret = svi_AddGlobVar(hstscada_SviHandle, SviGlobVarList[i].VarName,
                             SviGlobVarList[i].Format, SviGlobVarList[i].Size,
                             SviGlobVarList[i].pVar, 0, SviGlobVarList[i].UserParam,
                             SviGlobVarList[i].pSviStart, SviGlobVarList[i].pSviEnd);
        if (ret)
        {
            LOG_E(0, Func, "Could not add SVI variable '%s'!, Error %d",
                  SviGlobVarList[i].VarName, ret);
            /*
             * TODO:
             * Decide if the module shall be de-installed in this case.
             * If yes, return an error here.
             */
        }
    }

    return (OK);
}

/**
********************************************************************************
* @brief Frees SVI server resources according to hstscada_SviSrvInit()
*        Being called at module deinit by the bTask.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
VOID hstscada_SviSrvDeinit(VOID)
{
    /* If there was no or no successful SVI init */
    if (!hstscada_SviHandle)
        return;

    if (svi_DeInit(hstscada_SviHandle) < 0)
        LOG_E(0, "hstscada_SviSrvDeinit", "Could not de-initialize SVI server");

    hstscada_SviHandle = 0;
}

/**
********************************************************************************
* @brief This function demonstrates the implementation of an SVI client.
*        It will connect to the server of another software module (RES)
*        and will get the address of one SVI variable. The variable is
*        accessed in AppMain(VOID)
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 SviClt_Init(VOID)
{
    UINT32  SviFormat = 0;
    CHAR    Func[] = "SviClt_Init";

    /* Get library pointer to access SVI server of other software module. */
    pSviLib = svi_GetLib("RES");
    if (!pSviLib)
    {
        LOG_W(0, Func, "Could not get SVI of module 'RES'!");
        return (ERROR);
    }
// MATLABCODEGEN: OpenField Get Specified  App Module

pSviLib_ifcloads = svi_GetLib("ifcloads");
if (!pSviLib_ifcloads)
{
   LOG_W(0, Func, "Could not get SVI of module ifcloads!");
   return (ERROR);
}

pSviLib_ifcmm = svi_GetLib("ifcmm");
if (!pSviLib_ifcmm)
{
   LOG_W(0, Func, "Could not get SVI of module ifcmm!");
   return (ERROR);
}

pSviLib_ifcscada = svi_GetLib("ifcscada");
if (!pSviLib_ifcscada)
{
   LOG_W(0, Func, "Could not get SVI of module ifcscada!");
   return (ERROR);
}


// MATLABCODEGEN: CloseField Get Specified App Module

// MATLABCODEGEN: OpenField Get ITF SA Address

if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.Timestamp_UTC", &SA_WT1_Timestamp_UTC, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.Timestamp_UTC!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.GeneratorSpeed", &SA_WT1_GeneratorSpeed, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.GeneratorSpeed!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.ConverterTorque", &SA_WT1_ConverterTorque, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.ConverterTorque!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.NacelleDirection", &SA_WT1_NacelleDirection, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.NacelleDirection!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.PitchAngle", &SA_WT1_PitchAngle, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.PitchAngle!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.PitchAngle1", &SA_WT1_PitchAngle1, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.PitchAngle1!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.PitchAngle2", &SA_WT1_PitchAngle2, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.PitchAngle2!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.PitchAngle3", &SA_WT1_PitchAngle3, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.PitchAngle3!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.Power", &SA_WT1_Power, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.Power!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.RotorRpm", &SA_WT1_RotorRpm, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.RotorRpm!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.TurbineState", &SA_WT1_TurbineState, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.TurbineState!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.WindSpeed", &SA_WT1_WindSpeed, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.WindSpeed!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.YawError", &SA_WT1_YawError, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.YawError!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.ref_signal", &SA_WT1_ref_signal, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.ref_signal!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_scada.isConnected", &SA_WT1_isConnected, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT1_scada.isConnected!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.Timestamp_UTC", &SA_WT2_Timestamp_UTC, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.Timestamp_UTC!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.GeneratorSpeed", &SA_WT2_GeneratorSpeed, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.GeneratorSpeed!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.ConverterTorque", &SA_WT2_ConverterTorque, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.ConverterTorque!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.NacelleDirection", &SA_WT2_NacelleDirection, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.NacelleDirection!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.PitchAngle", &SA_WT2_PitchAngle, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.PitchAngle!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.PitchAngle1", &SA_WT2_PitchAngle1, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.PitchAngle1!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.PitchAngle2", &SA_WT2_PitchAngle2, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.PitchAngle2!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.PitchAngle3", &SA_WT2_PitchAngle3, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.PitchAngle3!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.Power", &SA_WT2_Power, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.Power!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.RotorRpm", &SA_WT2_RotorRpm, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.RotorRpm!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.TurbineState", &SA_WT2_TurbineState, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.TurbineState!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.WindSpeed", &SA_WT2_WindSpeed, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.WindSpeed!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.YawError", &SA_WT2_YawError, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.YawError!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.ref_signal", &SA_WT2_ref_signal, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.ref_signal!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_scada.isConnected", &SA_WT2_isConnected, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT2_scada.isConnected!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.Timestamp_UTC", &SA_WT3_Timestamp_UTC, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.Timestamp_UTC!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.GeneratorSpeed", &SA_WT3_GeneratorSpeed, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.GeneratorSpeed!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.ConverterTorque", &SA_WT3_ConverterTorque, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.ConverterTorque!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.NacelleDirection", &SA_WT3_NacelleDirection, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.NacelleDirection!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.PitchAngle", &SA_WT3_PitchAngle, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.PitchAngle!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.PitchAngle1", &SA_WT3_PitchAngle1, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.PitchAngle1!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.PitchAngle2", &SA_WT3_PitchAngle2, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.PitchAngle2!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.PitchAngle3", &SA_WT3_PitchAngle3, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.PitchAngle3!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.Power", &SA_WT3_Power, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.Power!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.RotorRpm", &SA_WT3_RotorRpm, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.RotorRpm!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.TurbineState", &SA_WT3_TurbineState, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.TurbineState!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.WindSpeed", &SA_WT3_WindSpeed, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.WindSpeed!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.YawError", &SA_WT3_YawError, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.YawError!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.ref_signal", &SA_WT3_ref_signal, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.ref_signal!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_scada.isConnected", &SA_WT3_isConnected, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value WT3_scada.isConnected!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT1_XYZ_vibrations", &SA_itfc_scada_WT1XYZvibrations, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value itfc_scada_WT1XYZvibrations!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT2_XYZ_vibrations", &SA_itfc_scada_WT2XYZvibrations, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value itfc_scada_WT2XYZvibrations!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "WT3_XYZ_vibrations", &SA_itfc_scada_WT3XYZvibrations, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value itfc_scada_WT3XYZvibrations!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.Timestamp_UTC", &SA_mm_Timestamp_UTC, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.Timestamp_UTC!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.ws_110m", &SA_mm_ws_110m, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.ws_110m!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.ws_60m", &SA_mm_ws_60m, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.ws_60m!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.wd_110m", &SA_mm_wd_110m, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.wd_110m!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.wd_60m", &SA_mm_wd_60m, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.wd_60m!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.temperature", &SA_mm_temperature, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.temperature!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.humidity", &SA_mm_humidity, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.humidity!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcmm, "met_mast.pressure", &SA_mm_pressure, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value met_mast.pressure!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.Timestamp_UTC", &SA_loads_Timestamp_UTC, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.Timestamp_UTC!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt1_blade1_root_edge", &SA_wt1_blade1_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt1_blade1_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt1_blade1_root_flap", &SA_wt1_blade1_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt1_blade1_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt1_blade2_root_edge", &SA_wt1_blade2_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt1_blade2_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt1_blade2_root_flap", &SA_wt1_blade2_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt1_blade2_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt1_blade3_root_edge", &SA_wt1_blade3_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt1_blade3_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt1_blade3_root_flap", &SA_wt1_blade3_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt1_blade3_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt2_blade1_root_edge", &SA_wt2_blade1_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt2_blade1_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt2_blade1_root_flap", &SA_wt2_blade1_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt2_blade1_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt2_blade2_root_edge", &SA_wt2_blade2_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt2_blade2_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt2_blade2_root_flap", &SA_wt2_blade2_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt2_blade2_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt2_blade3_root_edge", &SA_wt2_blade3_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt2_blade3_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt2_blade3_root_flap", &SA_wt2_blade3_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt2_blade3_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt3_blade1_root_edge", &SA_wt3_blade1_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt3_blade1_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt3_blade1_root_flap", &SA_wt3_blade1_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt3_blade1_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt3_blade2_root_edge", &SA_wt3_blade2_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt3_blade2_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt3_blade2_root_flap", &SA_wt3_blade2_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt3_blade2_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt3_blade3_root_edge", &SA_wt3_blade3_root_edge, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt3_blade3_root_edge!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcloads, "wf_loads.wt3_blade3_root_flap", &SA_wt3_blade3_root_flap, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value wf_loads.wt3_blade3_root_flap!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "exchange_data_ctrl", &SA_itfc_scada_exchangedatactrl, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value itfc_scada_exchangedatactrl!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "exchange_data_loads.wt1_blade1_IP", &SA_wt1_blade1_IP, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value exchange_data_loads.wt1_blade1_IP!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "exchange_data_loads.wt1_blade2_IP", &SA_wt1_blade2_IP, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value exchange_data_loads.wt1_blade2_IP!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "exchange_data_loads.wt1_blade3_IP", &SA_wt1_blade3_IP, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value exchange_data_loads.wt1_blade3_IP!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "exchange_data_loads.wt1_blade1_OOP", &SA_wt1_blade1_OOP, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value exchange_data_loads.wt1_blade1_OOP!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "exchange_data_loads.wt1_blade2_OOP", &SA_wt1_blade2_OOP, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value exchange_data_loads.wt1_blade2_OOP!");
    return (ERROR);
}
if (svi_GetAddr(pSviLib_ifcscada, "exchange_data_loads.wt1_blade3_OOP", &SA_wt1_blade3_OOP, &SviFormat) != SVI_E_OK)
{
    LOG_W(0, Func, "Could not get address of value exchange_data_loads.wt1_blade3_OOP!");
    return (ERROR);
}

// MATLABCODEGEN: CloseField Get ITF SA Address




    if (svi_GetAddr(pSviLib, "Time/Time_ms1970", &Time_ms1970_SviAddr, &SviFormat) != SVI_E_OK)
    {
        LOG_W(0, Func, "Could not get address of value 'Time_ms1970'!");
        return (ERROR);
    }





    /* Convert symbolic address "Time_us" to binary SVI address. */
    if (svi_GetAddr(pSviLib, "Time_us", &TimeSviAddr, &SviFormat) != SVI_E_OK)
    {
        LOG_W(0, Func, "Could not get address of value 'Time_us'!");
        return (ERROR);
    }
    return (OK);
}

/**
********************************************************************************
* @brief This function informs the RES that the function library of
*        the server (in this example the server is also RES) is no
*        longer required
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL VOID SviClt_Deinit(VOID)
{
    if (pSviLib)
    {
        if (svi_UngetLib(pSviLib) < 0)
            LOG_E(0, "SviClt_DeInit", "svi_UngetLib Error!");
        pSviLib = NULL;
    }

}

/**
********************************************************************************
* @brief This function demonstrates the implementation of an SVI client.
*        It will simply read one SVI variable. pLib and binary SVI address
*        have been retrieved in alt_SviClientInit().
*
* @param[in]  pTime_us    Buffer provided by caller to read the value
* @param[out] N/A
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 SviClt_Read()
{
    SINT32  ret;
// MATLABCODEGEN: OpenField Size of Array definition

UINT32 WT1_Timestamp_UTC_size = sizeof(WT1_Timestamp_UTC);
UINT32 WT1_GeneratorSpeed_size = sizeof(WT1_GeneratorSpeed);
UINT32 WT1_ConverterTorque_size = sizeof(WT1_ConverterTorque);
UINT32 WT1_NacelleDirection_size = sizeof(WT1_NacelleDirection);
UINT32 WT1_PitchAngle_size = sizeof(WT1_PitchAngle);
UINT32 WT1_PitchAngle1_size = sizeof(WT1_PitchAngle1);
UINT32 WT1_PitchAngle2_size = sizeof(WT1_PitchAngle2);
UINT32 WT1_PitchAngle3_size = sizeof(WT1_PitchAngle3);
UINT32 WT1_Power_size = sizeof(WT1_Power);
UINT32 WT1_RotorRpm_size = sizeof(WT1_RotorRpm);
UINT32 WT1_TurbineState_size = sizeof(WT1_TurbineState);
UINT32 WT1_WindSpeed_size = sizeof(WT1_WindSpeed);
UINT32 WT1_YawError_size = sizeof(WT1_YawError);
UINT32 WT1_ref_signal_size = sizeof(WT1_ref_signal);
UINT32 WT1_isConnected_size = sizeof(WT1_isConnected);
UINT32 WT2_Timestamp_UTC_size = sizeof(WT2_Timestamp_UTC);
UINT32 WT2_GeneratorSpeed_size = sizeof(WT2_GeneratorSpeed);
UINT32 WT2_ConverterTorque_size = sizeof(WT2_ConverterTorque);
UINT32 WT2_NacelleDirection_size = sizeof(WT2_NacelleDirection);
UINT32 WT2_PitchAngle_size = sizeof(WT2_PitchAngle);
UINT32 WT2_PitchAngle1_size = sizeof(WT2_PitchAngle1);
UINT32 WT2_PitchAngle2_size = sizeof(WT2_PitchAngle2);
UINT32 WT2_PitchAngle3_size = sizeof(WT2_PitchAngle3);
UINT32 WT2_Power_size = sizeof(WT2_Power);
UINT32 WT2_RotorRpm_size = sizeof(WT2_RotorRpm);
UINT32 WT2_TurbineState_size = sizeof(WT2_TurbineState);
UINT32 WT2_WindSpeed_size = sizeof(WT2_WindSpeed);
UINT32 WT2_YawError_size = sizeof(WT2_YawError);
UINT32 WT2_ref_signal_size = sizeof(WT2_ref_signal);
UINT32 WT2_isConnected_size = sizeof(WT2_isConnected);
UINT32 WT3_Timestamp_UTC_size = sizeof(WT3_Timestamp_UTC);
UINT32 WT3_GeneratorSpeed_size = sizeof(WT3_GeneratorSpeed);
UINT32 WT3_ConverterTorque_size = sizeof(WT3_ConverterTorque);
UINT32 WT3_NacelleDirection_size = sizeof(WT3_NacelleDirection);
UINT32 WT3_PitchAngle_size = sizeof(WT3_PitchAngle);
UINT32 WT3_PitchAngle1_size = sizeof(WT3_PitchAngle1);
UINT32 WT3_PitchAngle2_size = sizeof(WT3_PitchAngle2);
UINT32 WT3_PitchAngle3_size = sizeof(WT3_PitchAngle3);
UINT32 WT3_Power_size = sizeof(WT3_Power);
UINT32 WT3_RotorRpm_size = sizeof(WT3_RotorRpm);
UINT32 WT3_TurbineState_size = sizeof(WT3_TurbineState);
UINT32 WT3_WindSpeed_size = sizeof(WT3_WindSpeed);
UINT32 WT3_YawError_size = sizeof(WT3_YawError);
UINT32 WT3_ref_signal_size = sizeof(WT3_ref_signal);
UINT32 WT3_isConnected_size = sizeof(WT3_isConnected);
UINT32 itfc_scada_WT1XYZvibrations_size = sizeof(itfc_scada_WT1XYZvibrations);
UINT32 itfc_scada_WT2XYZvibrations_size = sizeof(itfc_scada_WT2XYZvibrations);
UINT32 itfc_scada_WT3XYZvibrations_size = sizeof(itfc_scada_WT3XYZvibrations);
UINT32 mm_Timestamp_UTC_size = sizeof(mm_Timestamp_UTC);
UINT32 mm_ws_110m_size = sizeof(mm_ws_110m);
UINT32 mm_ws_60m_size = sizeof(mm_ws_60m);
UINT32 mm_wd_110m_size = sizeof(mm_wd_110m);
UINT32 mm_wd_60m_size = sizeof(mm_wd_60m);
UINT32 mm_temperature_size = sizeof(mm_temperature);
UINT32 mm_humidity_size = sizeof(mm_humidity);
UINT32 mm_pressure_size = sizeof(mm_pressure);
UINT32 loads_Timestamp_UTC_size = sizeof(loads_Timestamp_UTC);
UINT32 wt1_blade1_root_edge_size = sizeof(wt1_blade1_root_edge);
UINT32 wt1_blade1_root_flap_size = sizeof(wt1_blade1_root_flap);
UINT32 wt1_blade2_root_edge_size = sizeof(wt1_blade2_root_edge);
UINT32 wt1_blade2_root_flap_size = sizeof(wt1_blade2_root_flap);
UINT32 wt1_blade3_root_edge_size = sizeof(wt1_blade3_root_edge);
UINT32 wt1_blade3_root_flap_size = sizeof(wt1_blade3_root_flap);
UINT32 wt2_blade1_root_edge_size = sizeof(wt2_blade1_root_edge);
UINT32 wt2_blade1_root_flap_size = sizeof(wt2_blade1_root_flap);
UINT32 wt2_blade2_root_edge_size = sizeof(wt2_blade2_root_edge);
UINT32 wt2_blade2_root_flap_size = sizeof(wt2_blade2_root_flap);
UINT32 wt2_blade3_root_edge_size = sizeof(wt2_blade3_root_edge);
UINT32 wt2_blade3_root_flap_size = sizeof(wt2_blade3_root_flap);
UINT32 wt3_blade1_root_edge_size = sizeof(wt3_blade1_root_edge);
UINT32 wt3_blade1_root_flap_size = sizeof(wt3_blade1_root_flap);
UINT32 wt3_blade2_root_edge_size = sizeof(wt3_blade2_root_edge);
UINT32 wt3_blade2_root_flap_size = sizeof(wt3_blade2_root_flap);
UINT32 wt3_blade3_root_edge_size = sizeof(wt3_blade3_root_edge);
UINT32 wt3_blade3_root_flap_size = sizeof(wt3_blade3_root_flap);

// MATLABCODEGEN: CloseField Size of Array definition

UINT32 Time_ms1970_size = sizeof(Time_ms1970);
ret = svi_GetBlk(pSviLib, Time_ms1970_SviAddr, (UINT32*) &Time_ms1970,  &Time_ms1970_size);
if (ret != SVI_E_OK)
    LOG_W(0, "SviClt_Read", "Could not read value Time_ms1970!");





    // MATLABCODEGEN: OpenField assign variable from SA_Address

ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_Timestamp_UTC, (UINT32*) &WT1_Timestamp_UTC,  &WT1_Timestamp_UTC_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_Timestamp_UTC!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_GeneratorSpeed, (UINT32*) &WT1_GeneratorSpeed,  &WT1_GeneratorSpeed_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_GeneratorSpeed!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_ConverterTorque, (UINT32*) &WT1_ConverterTorque,  &WT1_ConverterTorque_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_ConverterTorque!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_NacelleDirection, (UINT32*) &WT1_NacelleDirection,  &WT1_NacelleDirection_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_NacelleDirection!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_PitchAngle, (UINT32*) &WT1_PitchAngle,  &WT1_PitchAngle_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_PitchAngle!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_PitchAngle1, (UINT32*) &WT1_PitchAngle1,  &WT1_PitchAngle1_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_PitchAngle1!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_PitchAngle2, (UINT32*) &WT1_PitchAngle2,  &WT1_PitchAngle2_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_PitchAngle2!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_PitchAngle3, (UINT32*) &WT1_PitchAngle3,  &WT1_PitchAngle3_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_PitchAngle3!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_Power, (UINT32*) &WT1_Power,  &WT1_Power_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_Power!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_RotorRpm, (UINT32*) &WT1_RotorRpm,  &WT1_RotorRpm_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_RotorRpm!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_TurbineState, (UINT32*) &WT1_TurbineState,  &WT1_TurbineState_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_TurbineState!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_WindSpeed, (UINT32*) &WT1_WindSpeed,  &WT1_WindSpeed_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_WindSpeed!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_YawError, (UINT32*) &WT1_YawError,  &WT1_YawError_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_YawError!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_ref_signal, (UINT32*) &WT1_ref_signal,  &WT1_ref_signal_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_ref_signal!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT1_isConnected, (UINT32*) &WT1_isConnected,  &WT1_isConnected_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT1_isConnected!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_Timestamp_UTC, (UINT32*) &WT2_Timestamp_UTC,  &WT2_Timestamp_UTC_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_Timestamp_UTC!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_GeneratorSpeed, (UINT32*) &WT2_GeneratorSpeed,  &WT2_GeneratorSpeed_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_GeneratorSpeed!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_ConverterTorque, (UINT32*) &WT2_ConverterTorque,  &WT2_ConverterTorque_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_ConverterTorque!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_NacelleDirection, (UINT32*) &WT2_NacelleDirection,  &WT2_NacelleDirection_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_NacelleDirection!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_PitchAngle, (UINT32*) &WT2_PitchAngle,  &WT2_PitchAngle_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_PitchAngle!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_PitchAngle1, (UINT32*) &WT2_PitchAngle1,  &WT2_PitchAngle1_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_PitchAngle1!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_PitchAngle2, (UINT32*) &WT2_PitchAngle2,  &WT2_PitchAngle2_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_PitchAngle2!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_PitchAngle3, (UINT32*) &WT2_PitchAngle3,  &WT2_PitchAngle3_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_PitchAngle3!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_Power, (UINT32*) &WT2_Power,  &WT2_Power_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_Power!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_RotorRpm, (UINT32*) &WT2_RotorRpm,  &WT2_RotorRpm_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_RotorRpm!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_TurbineState, (UINT32*) &WT2_TurbineState,  &WT2_TurbineState_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_TurbineState!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_WindSpeed, (UINT32*) &WT2_WindSpeed,  &WT2_WindSpeed_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_WindSpeed!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_YawError, (UINT32*) &WT2_YawError,  &WT2_YawError_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_YawError!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_ref_signal, (UINT32*) &WT2_ref_signal,  &WT2_ref_signal_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_ref_signal!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT2_isConnected, (UINT32*) &WT2_isConnected,  &WT2_isConnected_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT2_isConnected!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_Timestamp_UTC, (UINT32*) &WT3_Timestamp_UTC,  &WT3_Timestamp_UTC_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_Timestamp_UTC!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_GeneratorSpeed, (UINT32*) &WT3_GeneratorSpeed,  &WT3_GeneratorSpeed_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_GeneratorSpeed!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_ConverterTorque, (UINT32*) &WT3_ConverterTorque,  &WT3_ConverterTorque_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_ConverterTorque!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_NacelleDirection, (UINT32*) &WT3_NacelleDirection,  &WT3_NacelleDirection_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_NacelleDirection!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_PitchAngle, (UINT32*) &WT3_PitchAngle,  &WT3_PitchAngle_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_PitchAngle!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_PitchAngle1, (UINT32*) &WT3_PitchAngle1,  &WT3_PitchAngle1_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_PitchAngle1!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_PitchAngle2, (UINT32*) &WT3_PitchAngle2,  &WT3_PitchAngle2_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_PitchAngle2!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_PitchAngle3, (UINT32*) &WT3_PitchAngle3,  &WT3_PitchAngle3_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_PitchAngle3!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_Power, (UINT32*) &WT3_Power,  &WT3_Power_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_Power!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_RotorRpm, (UINT32*) &WT3_RotorRpm,  &WT3_RotorRpm_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_RotorRpm!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_TurbineState, (UINT32*) &WT3_TurbineState,  &WT3_TurbineState_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_TurbineState!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_WindSpeed, (UINT32*) &WT3_WindSpeed,  &WT3_WindSpeed_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_WindSpeed!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_YawError, (UINT32*) &WT3_YawError,  &WT3_YawError_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_YawError!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_ref_signal, (UINT32*) &WT3_ref_signal,  &WT3_ref_signal_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_ref_signal!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_WT3_isConnected, (UINT32*) &WT3_isConnected,  &WT3_isConnected_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value WT3_isConnected!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_itfc_scada_WT1XYZvibrations, (UINT32*) &itfc_scada_WT1XYZvibrations,  &itfc_scada_WT1XYZvibrations_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value itfc_scada_WT1XYZvibrations!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_itfc_scada_WT2XYZvibrations, (UINT32*) &itfc_scada_WT2XYZvibrations,  &itfc_scada_WT2XYZvibrations_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value itfc_scada_WT2XYZvibrations!");
ret = svi_GetBlk(pSviLib_ifcscada, SA_itfc_scada_WT3XYZvibrations, (UINT32*) &itfc_scada_WT3XYZvibrations,  &itfc_scada_WT3XYZvibrations_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value itfc_scada_WT3XYZvibrations!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_Timestamp_UTC, (UINT32*) &mm_Timestamp_UTC,  &mm_Timestamp_UTC_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_Timestamp_UTC!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_ws_110m, (UINT32*) &mm_ws_110m,  &mm_ws_110m_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_ws_110m!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_ws_60m, (UINT32*) &mm_ws_60m,  &mm_ws_60m_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_ws_60m!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_wd_110m, (UINT32*) &mm_wd_110m,  &mm_wd_110m_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_wd_110m!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_wd_60m, (UINT32*) &mm_wd_60m,  &mm_wd_60m_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_wd_60m!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_temperature, (UINT32*) &mm_temperature,  &mm_temperature_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_temperature!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_humidity, (UINT32*) &mm_humidity,  &mm_humidity_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_humidity!");
ret = svi_GetBlk(pSviLib_ifcmm, SA_mm_pressure, (UINT32*) &mm_pressure,  &mm_pressure_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value mm_pressure!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_loads_Timestamp_UTC, (UINT32*) &loads_Timestamp_UTC,  &loads_Timestamp_UTC_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value loads_Timestamp_UTC!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt1_blade1_root_edge, (UINT32*) &wt1_blade1_root_edge,  &wt1_blade1_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt1_blade1_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt1_blade1_root_flap, (UINT32*) &wt1_blade1_root_flap,  &wt1_blade1_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt1_blade1_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt1_blade2_root_edge, (UINT32*) &wt1_blade2_root_edge,  &wt1_blade2_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt1_blade2_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt1_blade2_root_flap, (UINT32*) &wt1_blade2_root_flap,  &wt1_blade2_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt1_blade2_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt1_blade3_root_edge, (UINT32*) &wt1_blade3_root_edge,  &wt1_blade3_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt1_blade3_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt1_blade3_root_flap, (UINT32*) &wt1_blade3_root_flap,  &wt1_blade3_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt1_blade3_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt2_blade1_root_edge, (UINT32*) &wt2_blade1_root_edge,  &wt2_blade1_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt2_blade1_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt2_blade1_root_flap, (UINT32*) &wt2_blade1_root_flap,  &wt2_blade1_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt2_blade1_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt2_blade2_root_edge, (UINT32*) &wt2_blade2_root_edge,  &wt2_blade2_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt2_blade2_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt2_blade2_root_flap, (UINT32*) &wt2_blade2_root_flap,  &wt2_blade2_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt2_blade2_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt2_blade3_root_edge, (UINT32*) &wt2_blade3_root_edge,  &wt2_blade3_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt2_blade3_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt2_blade3_root_flap, (UINT32*) &wt2_blade3_root_flap,  &wt2_blade3_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt2_blade3_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt3_blade1_root_edge, (UINT32*) &wt3_blade1_root_edge,  &wt3_blade1_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt3_blade1_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt3_blade1_root_flap, (UINT32*) &wt3_blade1_root_flap,  &wt3_blade1_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt3_blade1_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt3_blade2_root_edge, (UINT32*) &wt3_blade2_root_edge,  &wt3_blade2_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt3_blade2_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt3_blade2_root_flap, (UINT32*) &wt3_blade2_root_flap,  &wt3_blade2_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt3_blade2_root_flap!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt3_blade3_root_edge, (UINT32*) &wt3_blade3_root_edge,  &wt3_blade3_root_edge_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt3_blade3_root_edge!");
ret = svi_GetBlk(pSviLib_ifcloads, SA_wt3_blade3_root_flap, (UINT32*) &wt3_blade3_root_flap,  &wt3_blade3_root_flap_size);
if (ret != SVI_E_OK)
   LOG_W(0, "SviClt_Read", "Could not read value wt3_blade3_root_flap!");
 // MATLABCODEGEN: CloseField assign variable from SA_Address
    return (ret);
}
MLOCAL SINT32 SviClt_Write(VOID)
{
    SINT32  ret;
    // MATLABCODEGEN: OpenField assign variable in SviClt_Write

ret = svi_SetBlk(pSviLib_ifcscada, SA_itfc_scada_exchangedatactrl, (UINT32*) &itfc_scada_exchangedatactrl, sizeof(itfc_scada_exchangedatactrl) );
if (ret != SVI_E_OK)
     LOG_W(0, "SviClt_Write", "Could not read value itfc_scada_exchangedatactrl!");

ret = svi_SetBlk(pSviLib_ifcscada, SA_wt1_blade1_IP, (UINT32*) &wt1_blade1_IP, sizeof(wt1_blade1_IP) );
if (ret != SVI_E_OK)
     LOG_W(0, "SviClt_Write", "Could not read value wt1_blade1_IP!");

ret = svi_SetBlk(pSviLib_ifcscada, SA_wt1_blade2_IP, (UINT32*) &wt1_blade2_IP, sizeof(wt1_blade2_IP) );
if (ret != SVI_E_OK)
     LOG_W(0, "SviClt_Write", "Could not read value wt1_blade2_IP!");

ret = svi_SetBlk(pSviLib_ifcscada, SA_wt1_blade3_IP, (UINT32*) &wt1_blade3_IP, sizeof(wt1_blade3_IP) );
if (ret != SVI_E_OK)
     LOG_W(0, "SviClt_Write", "Could not read value wt1_blade3_IP!");

ret = svi_SetBlk(pSviLib_ifcscada, SA_wt1_blade1_OOP, (UINT32*) &wt1_blade1_OOP, sizeof(wt1_blade1_OOP) );
if (ret != SVI_E_OK)
     LOG_W(0, "SviClt_Write", "Could not read value wt1_blade1_OOP!");

ret = svi_SetBlk(pSviLib_ifcscada, SA_wt1_blade2_OOP, (UINT32*) &wt1_blade2_OOP, sizeof(wt1_blade2_OOP) );
if (ret != SVI_E_OK)
     LOG_W(0, "SviClt_Write", "Could not read value wt1_blade2_OOP!");

ret = svi_SetBlk(pSviLib_ifcscada, SA_wt1_blade3_OOP, (UINT32*) &wt1_blade3_OOP, sizeof(wt1_blade3_OOP) );
if (ret != SVI_E_OK)
     LOG_W(0, "SviClt_Write", "Could not read value wt1_blade3_OOP!");

 // MATLABCODEGEN: CloseField assign variable in SviClt_Write
    return (ret);
}
MLOCAL SINT32 SviClt_Example(UINT32 * pTime_us)
{
    SINT32  ret;

    /* Read the microseconds since CPU boot from module RES. */
    ret = svi_GetVal(pSviLib, TimeSviAddr, pTime_us);
    if (ret != SVI_E_OK)
        LOG_W(0, "SviClt_Example", "Could not read value!");

    return (ret);
}


MLOCAL SINT32 write_header_fast()
{
    FILE *fp = pFileFast;

    if (fp == NULL)
     {
         LOG_W(0, "write_header_fast", "File pointer for fast file is empty when writing fast header.");
         return(1);
     }
// MATLABCODEGEN: OpenField  Label Names Output Fields fast
fprintf(fp, "TimeStamp_UTC; WT1_Timestamp_UTC[-]; WT1_GeneratorSpeed[RPM]; WT1_ConverterTorque[Nm]; WT1_PitchAngle[deg]; WT1_PitchAngle1[deg]; WT1_PitchAngle2[deg]; WT1_PitchAngle3[deg]; WT1_Power[kW]; WT1_RotorRpm[RPM]; WT1_TurbineState[-]; WT1_WindSpeed[m/s]; WT1_YawError[deg]; WT1_ref_signal[-]; WT2_Timestamp_UTC[-]; WT2_GeneratorSpeed[RPM]; WT2_ConverterTorque[Nm]; WT2_PitchAngle[deg]; WT2_PitchAngle1[deg]; WT2_PitchAngle2[deg]; WT2_PitchAngle3[deg]; WT2_Power[kW]; WT2_RotorRpm[RPM]; WT2_TurbineState[-]; WT2_WindSpeed[m/s]; WT2_YawError[deg]; WT2_ref_signal[-]; WT3_Timestamp_UTC[-]; WT3_GeneratorSpeed[RPM]; WT3_ConverterTorque[Nm]; WT3_PitchAngle[deg]; WT3_PitchAngle1[deg]; WT3_PitchAngle2[deg]; WT3_PitchAngle3[deg]; WT3_Power[kW]; WT3_RotorRpm[RPM]; WT3_TurbineState[-]; WT3_WindSpeed[m/s]; WT3_YawError[deg]; WT3_ref_signal[-]; WT1_X_vibration[m/s2]; WT1_Y_vibration[m/s2]; WT1_Z_vibration[m/s2]; WT2_X_vibration[m/s2]; WT2_Y_vibration[m/s2]; WT2_Z_vibration[m/s2]; WT3_X_vibration[m/s2]; WT3_Y_vibration[m/s2]; WT3_Z_vibration[m/s2]; mm_Timestamp_UTC[-]; mm_ws_110m[m/s]; mm_ws_60m[m/s]; mm_wd_110m[deg]; mm_wd_60m[deg]; loads_Timestamp_UTC[-]; wt1_blade1_root_edge[Nm]; wt1_blade1_root_flap[Nm]; wt1_blade2_root_edge[Nm]; wt1_blade2_root_flap[Nm]; wt1_blade3_root_edge[Nm]; wt1_blade3_root_flap[Nm]; wt2_blade1_root_edge[Nm]; wt2_blade1_root_flap[Nm]; wt2_blade2_root_edge[Nm]; wt2_blade2_root_flap[Nm]; wt2_blade3_root_edge[Nm]; wt2_blade3_root_flap[Nm]; wt3_blade1_root_edge[Nm]; wt3_blade1_root_flap[Nm]; wt3_blade2_root_edge[Nm]; wt3_blade2_root_flap[Nm]; wt3_blade3_root_edge[Nm]; wt3_blade3_root_flap[Nm]; wt1_blade1_IP[Nm]; wt1_blade2_IP[Nm]; wt1_blade3_IP[Nm]; wt1_blade1_OOP[Nm]; wt1_blade2_OOP[Nm]; wt1_blade3_OOP[Nm]; loads_process_AppStatus[-]; \n");
// MATLABCODEGEN: CloseField  Label Names Output Fields fast




    return(0);
}

MLOCAL SINT32 write_header_slow()
{
    FILE *fp = pFileSlow;

    if (fp == NULL)
     {
         LOG_W(0, "write_header_slow", "File pointer for fast file is empty when writing slow header.");
         return(1);
     }
// MATLABCODEGEN: OpenField  Label Names Output Fields slow
fprintf(fp, "TimeStamp_UTC; WT1_NacelleDirection[deg]; WT1_isConnected[-]; WT2_NacelleDirection[deg]; WT2_isConnected[-]; WT3_NacelleDirection[deg]; WT3_isConnected[-]; mm_temperature[°C]; mm_humidity[%]; mm_pressure[Pa]; avg_TI[-]; avg_ws_110m[m/s]; avg_ws_60m[m/s]; avg_shearExp[-]; avg_wd_110m[deg]; \n");
// MATLABCODEGEN: CloseField  Label Names Output Fields slow




    return(0);
}

MLOCAL SINT32 write_header_ctrl()
{
    FILE *fp = pFileCtrl;

    if (fp == NULL)
     {
         LOG_W(0, "write_header_ctrl", "File pointer for fast file is empty when writing ctrl header.");
         return(1);
     }
// MATLABCODEGEN: OpenField  Label Names Output Fields ctrl
fprintf(fp, "TimeStamp_UTC; check_isWFon[-]; check_WindInSector[-]; check_all[-]; avg_inflowState[-]; avg_inflow_AppStatus[-]; WF_status_AppStatus[-]; \n");
// MATLABCODEGEN: CloseField  Label Names Output Fields ctrl




    return(0);
}

MLOCAL SINT32 write_output_fast()
{

    FILE *fp = pFileFast;
    // Printing to file, checking for nan

    if (fp == NULL)
     {
         LOG_W(0, "write_output_fast", "File pointer for fast file is empty when writing fast.");
         return(1);
     }

    fprintf(fp, "%s; ", Date_and_Time);
// MATLABCODEGEN: OpenField  Print output to file fast

fprintf(fp, "%s; ", WT1_Timestamp_UTC);  

if (WT1_GeneratorSpeed==WT1_GeneratorSpeed)
{
     fprintf(fp, "%llf; ", WT1_GeneratorSpeed);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_ConverterTorque==WT1_ConverterTorque)
{
     fprintf(fp, "%llf; ", WT1_ConverterTorque);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_PitchAngle==WT1_PitchAngle)
{
     fprintf(fp, "%llf; ", WT1_PitchAngle);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_PitchAngle1==WT1_PitchAngle1)
{
     fprintf(fp, "%llf; ", WT1_PitchAngle1);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_PitchAngle2==WT1_PitchAngle2)
{
     fprintf(fp, "%llf; ", WT1_PitchAngle2);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_PitchAngle3==WT1_PitchAngle3)
{
     fprintf(fp, "%llf; ", WT1_PitchAngle3);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_Power==WT1_Power)
{
     fprintf(fp, "%llf; ", WT1_Power);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_RotorRpm==WT1_RotorRpm)
{
     fprintf(fp, "%llf; ", WT1_RotorRpm);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_TurbineState==WT1_TurbineState)
{
     fprintf(fp, "%d; ", WT1_TurbineState);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_WindSpeed==WT1_WindSpeed)
{
     fprintf(fp, "%llf; ", WT1_WindSpeed);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_YawError==WT1_YawError)
{
     fprintf(fp, "%llf; ", WT1_YawError);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_ref_signal==WT1_ref_signal)
{
     fprintf(fp, "%f; ", WT1_ref_signal);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}
fprintf(fp, "%s; ", WT2_Timestamp_UTC);  

if (WT2_GeneratorSpeed==WT2_GeneratorSpeed)
{
     fprintf(fp, "%llf; ", WT2_GeneratorSpeed);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_ConverterTorque==WT2_ConverterTorque)
{
     fprintf(fp, "%llf; ", WT2_ConverterTorque);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_PitchAngle==WT2_PitchAngle)
{
     fprintf(fp, "%llf; ", WT2_PitchAngle);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_PitchAngle1==WT2_PitchAngle1)
{
     fprintf(fp, "%llf; ", WT2_PitchAngle1);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_PitchAngle2==WT2_PitchAngle2)
{
     fprintf(fp, "%llf; ", WT2_PitchAngle2);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_PitchAngle3==WT2_PitchAngle3)
{
     fprintf(fp, "%llf; ", WT2_PitchAngle3);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_Power==WT2_Power)
{
     fprintf(fp, "%llf; ", WT2_Power);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_RotorRpm==WT2_RotorRpm)
{
     fprintf(fp, "%llf; ", WT2_RotorRpm);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_TurbineState==WT2_TurbineState)
{
     fprintf(fp, "%d; ", WT2_TurbineState);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_WindSpeed==WT2_WindSpeed)
{
     fprintf(fp, "%llf; ", WT2_WindSpeed);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_YawError==WT2_YawError)
{
     fprintf(fp, "%llf; ", WT2_YawError);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_ref_signal==WT2_ref_signal)
{
     fprintf(fp, "%f; ", WT2_ref_signal);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}
fprintf(fp, "%s; ", WT3_Timestamp_UTC);  

if (WT3_GeneratorSpeed==WT3_GeneratorSpeed)
{
     fprintf(fp, "%llf; ", WT3_GeneratorSpeed);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_ConverterTorque==WT3_ConverterTorque)
{
     fprintf(fp, "%llf; ", WT3_ConverterTorque);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_PitchAngle==WT3_PitchAngle)
{
     fprintf(fp, "%llf; ", WT3_PitchAngle);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_PitchAngle1==WT3_PitchAngle1)
{
     fprintf(fp, "%llf; ", WT3_PitchAngle1);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_PitchAngle2==WT3_PitchAngle2)
{
     fprintf(fp, "%llf; ", WT3_PitchAngle2);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_PitchAngle3==WT3_PitchAngle3)
{
     fprintf(fp, "%llf; ", WT3_PitchAngle3);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_Power==WT3_Power)
{
     fprintf(fp, "%llf; ", WT3_Power);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_RotorRpm==WT3_RotorRpm)
{
     fprintf(fp, "%llf; ", WT3_RotorRpm);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_TurbineState==WT3_TurbineState)
{
     fprintf(fp, "%d; ", WT3_TurbineState);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_WindSpeed==WT3_WindSpeed)
{
     fprintf(fp, "%llf; ", WT3_WindSpeed);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_YawError==WT3_YawError)
{
     fprintf(fp, "%llf; ", WT3_YawError);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_ref_signal==WT3_ref_signal)
{
     fprintf(fp, "%f; ", WT3_ref_signal);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_X_vibration==WT1_X_vibration)
{
     fprintf(fp, "%llf; ", WT1_X_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_Y_vibration==WT1_Y_vibration)
{
     fprintf(fp, "%llf; ", WT1_Y_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_Z_vibration==WT1_Z_vibration)
{
     fprintf(fp, "%llf; ", WT1_Z_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_X_vibration==WT2_X_vibration)
{
     fprintf(fp, "%llf; ", WT2_X_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_Y_vibration==WT2_Y_vibration)
{
     fprintf(fp, "%llf; ", WT2_Y_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_Z_vibration==WT2_Z_vibration)
{
     fprintf(fp, "%llf; ", WT2_Z_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_X_vibration==WT3_X_vibration)
{
     fprintf(fp, "%llf; ", WT3_X_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_Y_vibration==WT3_Y_vibration)
{
     fprintf(fp, "%llf; ", WT3_Y_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_Z_vibration==WT3_Z_vibration)
{
     fprintf(fp, "%llf; ", WT3_Z_vibration);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}
fprintf(fp, "%s; ", mm_Timestamp_UTC);  

if (mm_ws_110m==mm_ws_110m)
{
     fprintf(fp, "%llf; ", mm_ws_110m);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (mm_ws_60m==mm_ws_60m)
{
     fprintf(fp, "%llf; ", mm_ws_60m);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (mm_wd_110m==mm_wd_110m)
{
     fprintf(fp, "%llf; ", mm_wd_110m);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (mm_wd_60m==mm_wd_60m)
{
     fprintf(fp, "%llf; ", mm_wd_60m);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}
fprintf(fp, "%s; ", loads_Timestamp_UTC);  

if (wt1_blade1_root_edge==wt1_blade1_root_edge)
{
     fprintf(fp, "%llf; ", wt1_blade1_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade1_root_flap==wt1_blade1_root_flap)
{
     fprintf(fp, "%llf; ", wt1_blade1_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade2_root_edge==wt1_blade2_root_edge)
{
     fprintf(fp, "%llf; ", wt1_blade2_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade2_root_flap==wt1_blade2_root_flap)
{
     fprintf(fp, "%llf; ", wt1_blade2_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade3_root_edge==wt1_blade3_root_edge)
{
     fprintf(fp, "%llf; ", wt1_blade3_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade3_root_flap==wt1_blade3_root_flap)
{
     fprintf(fp, "%llf; ", wt1_blade3_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt2_blade1_root_edge==wt2_blade1_root_edge)
{
     fprintf(fp, "%llf; ", wt2_blade1_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt2_blade1_root_flap==wt2_blade1_root_flap)
{
     fprintf(fp, "%llf; ", wt2_blade1_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt2_blade2_root_edge==wt2_blade2_root_edge)
{
     fprintf(fp, "%llf; ", wt2_blade2_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt2_blade2_root_flap==wt2_blade2_root_flap)
{
     fprintf(fp, "%llf; ", wt2_blade2_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt2_blade3_root_edge==wt2_blade3_root_edge)
{
     fprintf(fp, "%llf; ", wt2_blade3_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt2_blade3_root_flap==wt2_blade3_root_flap)
{
     fprintf(fp, "%llf; ", wt2_blade3_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt3_blade1_root_edge==wt3_blade1_root_edge)
{
     fprintf(fp, "%llf; ", wt3_blade1_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt3_blade1_root_flap==wt3_blade1_root_flap)
{
     fprintf(fp, "%llf; ", wt3_blade1_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt3_blade2_root_edge==wt3_blade2_root_edge)
{
     fprintf(fp, "%llf; ", wt3_blade2_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt3_blade2_root_flap==wt3_blade2_root_flap)
{
     fprintf(fp, "%llf; ", wt3_blade2_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt3_blade3_root_edge==wt3_blade3_root_edge)
{
     fprintf(fp, "%llf; ", wt3_blade3_root_edge);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt3_blade3_root_flap==wt3_blade3_root_flap)
{
     fprintf(fp, "%llf; ", wt3_blade3_root_flap);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade1_IP==wt1_blade1_IP)
{
     fprintf(fp, "%llf; ", wt1_blade1_IP);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade2_IP==wt1_blade2_IP)
{
     fprintf(fp, "%llf; ", wt1_blade2_IP);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade3_IP==wt1_blade3_IP)
{
     fprintf(fp, "%llf; ", wt1_blade3_IP);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade1_OOP==wt1_blade1_OOP)
{
     fprintf(fp, "%llf; ", wt1_blade1_OOP);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade2_OOP==wt1_blade2_OOP)
{
     fprintf(fp, "%llf; ", wt1_blade2_OOP);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (wt1_blade3_OOP==wt1_blade3_OOP)
{
     fprintf(fp, "%llf; ", wt1_blade3_OOP);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (loads_process_AppStatus==loads_process_AppStatus)
{
     fprintf(fp, "%d; ", loads_process_AppStatus);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

// MATLABCODEGEN: CloseField  Print output to file fast


    fprintf(fp, "\n");
    fflush(fp);

    return 0;
}

MLOCAL SINT32 write_output_slow()
{

    FILE *fp = pFileSlow;
    // Printing to file, checking for nan

    if (fp == NULL)
     {
         LOG_W(0, "write_output_slow", "File pointer for fast file is empty when writing slow.");
         return(1);
     }

    fprintf(fp, "%s; ", Date_and_Time);
// MATLABCODEGEN: OpenField  Print output to file slow


if (WT1_NacelleDirection==WT1_NacelleDirection)
{
     fprintf(fp, "%llf; ", WT1_NacelleDirection);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT1_isConnected==WT1_isConnected)
{
     fprintf(fp, "%d; ", WT1_isConnected);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_NacelleDirection==WT2_NacelleDirection)
{
     fprintf(fp, "%llf; ", WT2_NacelleDirection);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT2_isConnected==WT2_isConnected)
{
     fprintf(fp, "%d; ", WT2_isConnected);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_NacelleDirection==WT3_NacelleDirection)
{
     fprintf(fp, "%llf; ", WT3_NacelleDirection);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WT3_isConnected==WT3_isConnected)
{
     fprintf(fp, "%d; ", WT3_isConnected);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (mm_temperature==mm_temperature)
{
     fprintf(fp, "%llf; ", mm_temperature);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (mm_humidity==mm_humidity)
{
     fprintf(fp, "%llf; ", mm_humidity);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (mm_pressure==mm_pressure)
{
     fprintf(fp, "%llf; ", mm_pressure);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (avg_TI==avg_TI)
{
     fprintf(fp, "%llf; ", avg_TI);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (avg_ws_110m==avg_ws_110m)
{
     fprintf(fp, "%llf; ", avg_ws_110m);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (avg_ws_60m==avg_ws_60m)
{
     fprintf(fp, "%llf; ", avg_ws_60m);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (avg_shearExp==avg_shearExp)
{
     fprintf(fp, "%llf; ", avg_shearExp);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (avg_wd_110m==avg_wd_110m)
{
     fprintf(fp, "%llf; ", avg_wd_110m);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

// MATLABCODEGEN: CloseField  Print output to file slow


    fprintf(fp, "\n");
    fflush(fp);

    return 0;
}

MLOCAL SINT32 write_output_ctrl()
{

    FILE *fp = pFileCtrl;
    // Printing to file, checking for nan

    if (fp == NULL)
     {
         LOG_W(0, "write_output_ctrl", "File pointer for fast file is empty when writing ctrl.");
         return(1);
     }

    fprintf(fp, "%s; ", Date_and_Time);
// MATLABCODEGEN: OpenField  Print output to file ctrl


if (check_isWFon==check_isWFon)
{
     fprintf(fp, "%llf; ", check_isWFon);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (check_WindInSector==check_WindInSector)
{
     fprintf(fp, "%llf; ", check_WindInSector);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (check_all==check_all)
{
     fprintf(fp, "%llf; ", check_all);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (avg_inflowState==avg_inflowState)
{
     fprintf(fp, "%llf; ", avg_inflowState);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (avg_inflow_AppStatus==avg_inflow_AppStatus)
{
     fprintf(fp, "%d; ", avg_inflow_AppStatus);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

if (WF_status_AppStatus==WF_status_AppStatus)
{
     fprintf(fp, "%d; ", WF_status_AppStatus);
}
else
{
     fprintf(fp, "%s; ", "NaN");
}

// MATLABCODEGEN: CloseField  Print output to file ctrl


    fprintf(fp, "\n");
    fflush(fp);

    return 0;
}