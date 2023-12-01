/**
********************************************************************************
* @file     ifcmm_app.c
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
#include "ifcmm.h"
#include "ifcmm_e.h"
#include "ifcmm_int.h"

/* Functions: administration, to be called from outside this file */
SINT32  ifcmm_AppEOI(VOID);
VOID    ifcmm_AppDeinit(VOID);
SINT32  ifcmm_CfgRead(VOID);
SINT32  ifcmm_SviSrvInit(VOID);
VOID    ifcmm_SviSrvDeinit(VOID);

/* Functions: administration, to be called only from within this file */
MLOCAL VOID ifcmm_CfgInit(VOID);

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

/* Global variables: data structure for mconfig parameters */
IFCMM_BASE_PARMS ifcmm_BaseParams;

/* Global variables: SVI client */
MLOCAL  SINT32(**pSviLib) () = NULL;    /* Information about external SVI server */
MLOCAL SVI_ADDR TimeSviAddr;            /* SVI address of server's variable */
MLOCAL BOOL8 PrintTime = TRUE;



// MATLABCODEGEN: OpenField define and declare SVI inputs

struct struct_metmast{
    CHAR Timestamp_UTC[32];
    REAL64 ws_110m[1];
    REAL64 ws_60m[1];
    REAL64 wd_110m[1];
    REAL64 wd_60m[1];
    REAL64 temperature[1];
    REAL64 humidity[1];
    REAL64 pressure[1];
}metmast;

// MATLABCODEGEN: CloseField define and declare SVI inputs

// MATLABCODEGEN: OpenField define variable Time Histories

struct struct_metmast metmast_TH[1000];

// MATLABCODEGEN: CloseField define variable Time Histories



/* Global variables: miscellaneous */
MLOCAL UINT32 CycleCount = 0;

/*
 * Global variables: Settings for application task
 * A reference to these settings must be registered in TaskList[], see below.
 * If no configuration group is being specified, all values must be set properly
 * in this initialization.
 */
MLOCAL TASK_PROPERTIES TaskProperties_aControl = {
    "aIFCMM_Ctrl",                 /* unique task name, maximum length 14 */
    "ControlTask",                      /* configuration group name */
    Control_Main,                       /* task entry function (function pointer) */
    0,                                  /* default task priority (->Task_CfgRead) */
    10.0,                               /* default task cycle time in ms (->Task_CfgRead) */
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
    {"ModuleVersion", SVI_F_OUT | SVI_F_STRING, sizeof(ifcmm_Version),
     (UINT32 *) ifcmm_Version, 0, NULL, NULL},

// MATLABCODEGEN: OpenField SVI Variables Coupling

{"met_mast", SVI_F_INOUT | SVI_F_BLK | SVI_F_MIXED, sizeof(struct struct_metmast), (UINT32 *) &metmast, 0, NULL, NULL}
,
{"met_mast.Timestamp_UTC", SVI_F_INOUT | SVI_F_STRING, sizeof(CHAR[32]), (UINT32 *) &metmast.Timestamp_UTC, 0, NULL, NULL}
,
{"met_mast.ws_110m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64 , sizeof(REAL64[1]), (UINT32 *) &metmast.ws_110m, 0, NULL, NULL}
,
{"met_mast.ws_60m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64 , sizeof(REAL64[1]), (UINT32 *) &metmast.ws_60m, 0, NULL, NULL}
,
{"met_mast.wd_110m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64 , sizeof(REAL64[1]), (UINT32 *) &metmast.wd_110m, 0, NULL, NULL}
,
{"met_mast.wd_60m", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64 , sizeof(REAL64[1]), (UINT32 *) &metmast.wd_60m, 0, NULL, NULL}
,
{"met_mast.temperature", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64 , sizeof(REAL64[1]), (UINT32 *) &metmast.temperature, 0, NULL, NULL}
,
{"met_mast.humidity", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64 , sizeof(REAL64[1]), (UINT32 *) &metmast.humidity, 0, NULL, NULL}
,
{"met_mast.pressure", SVI_F_INOUT | SVI_F_BLK | SVI_F_REAL64 , sizeof(REAL64[1]), (UINT32 *) &metmast.pressure, 0, NULL, NULL}
,

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

    /* TODO: add what is necessary before cyclic operation starts */
// MATLABCODEGEN: OpenField Initializing Variables Time History

strcpy(metmast_TH[0].Timestamp_UTC, "29-Mar-2021 00:00:00:031");
strcpy(metmast_TH[1].Timestamp_UTC, "29-Mar-2021 00:00:00:130");
strcpy(metmast_TH[2].Timestamp_UTC, "29-Mar-2021 00:00:00:228");
strcpy(metmast_TH[3].Timestamp_UTC, "29-Mar-2021 00:00:00:327");
strcpy(metmast_TH[4].Timestamp_UTC, "29-Mar-2021 00:00:00:426");
strcpy(metmast_TH[5].Timestamp_UTC, "29-Mar-2021 00:00:00:525");
strcpy(metmast_TH[6].Timestamp_UTC, "29-Mar-2021 00:00:00:634");
strcpy(metmast_TH[7].Timestamp_UTC, "29-Mar-2021 00:00:00:733");
strcpy(metmast_TH[8].Timestamp_UTC, "29-Mar-2021 00:00:00:832");
strcpy(metmast_TH[9].Timestamp_UTC, "29-Mar-2021 00:00:00:931");
strcpy(metmast_TH[10].Timestamp_UTC, "29-Mar-2021 00:00:01:031");
strcpy(metmast_TH[11].Timestamp_UTC, "29-Mar-2021 00:00:01:130");
strcpy(metmast_TH[12].Timestamp_UTC, "29-Mar-2021 00:00:01:228");
strcpy(metmast_TH[13].Timestamp_UTC, "29-Mar-2021 00:00:01:327");
strcpy(metmast_TH[14].Timestamp_UTC, "29-Mar-2021 00:00:01:426");
strcpy(metmast_TH[15].Timestamp_UTC, "29-Mar-2021 00:00:01:535");
strcpy(metmast_TH[16].Timestamp_UTC, "29-Mar-2021 00:00:01:634");
strcpy(metmast_TH[17].Timestamp_UTC, "29-Mar-2021 00:00:01:733");
strcpy(metmast_TH[18].Timestamp_UTC, "29-Mar-2021 00:00:01:832");
strcpy(metmast_TH[19].Timestamp_UTC, "29-Mar-2021 00:00:01:931");
strcpy(metmast_TH[20].Timestamp_UTC, "29-Mar-2021 00:00:02:021");
strcpy(metmast_TH[21].Timestamp_UTC, "29-Mar-2021 00:00:02:120");
strcpy(metmast_TH[22].Timestamp_UTC, "29-Mar-2021 00:00:02:219");
strcpy(metmast_TH[23].Timestamp_UTC, "29-Mar-2021 00:00:02:317");
strcpy(metmast_TH[24].Timestamp_UTC, "29-Mar-2021 00:00:02:426");
strcpy(metmast_TH[25].Timestamp_UTC, "29-Mar-2021 00:00:02:525");
strcpy(metmast_TH[26].Timestamp_UTC, "29-Mar-2021 00:00:02:624");
strcpy(metmast_TH[27].Timestamp_UTC, "29-Mar-2021 00:00:02:723");
strcpy(metmast_TH[28].Timestamp_UTC, "29-Mar-2021 00:00:02:822");
strcpy(metmast_TH[29].Timestamp_UTC, "29-Mar-2021 00:00:02:921");
strcpy(metmast_TH[30].Timestamp_UTC, "29-Mar-2021 00:00:03:021");
strcpy(metmast_TH[31].Timestamp_UTC, "29-Mar-2021 00:00:03:120");
strcpy(metmast_TH[32].Timestamp_UTC, "29-Mar-2021 00:00:03:219");
strcpy(metmast_TH[33].Timestamp_UTC, "29-Mar-2021 00:00:03:327");
strcpy(metmast_TH[34].Timestamp_UTC, "29-Mar-2021 00:00:03:426");
strcpy(metmast_TH[35].Timestamp_UTC, "29-Mar-2021 00:00:03:525");
strcpy(metmast_TH[36].Timestamp_UTC, "29-Mar-2021 00:00:03:624");
strcpy(metmast_TH[37].Timestamp_UTC, "29-Mar-2021 00:00:03:723");
strcpy(metmast_TH[38].Timestamp_UTC, "29-Mar-2021 00:00:03:822");
strcpy(metmast_TH[39].Timestamp_UTC, "29-Mar-2021 00:00:03:921");
strcpy(metmast_TH[40].Timestamp_UTC, "29-Mar-2021 00:00:04:021");
strcpy(metmast_TH[41].Timestamp_UTC, "29-Mar-2021 00:00:04:120");
strcpy(metmast_TH[42].Timestamp_UTC, "29-Mar-2021 00:00:04:228");
strcpy(metmast_TH[43].Timestamp_UTC, "29-Mar-2021 00:00:04:327");
strcpy(metmast_TH[44].Timestamp_UTC, "29-Mar-2021 00:00:04:426");
strcpy(metmast_TH[45].Timestamp_UTC, "29-Mar-2021 00:00:04:525");
strcpy(metmast_TH[46].Timestamp_UTC, "29-Mar-2021 00:00:04:624");
strcpy(metmast_TH[47].Timestamp_UTC, "29-Mar-2021 00:00:04:723");
strcpy(metmast_TH[48].Timestamp_UTC, "29-Mar-2021 00:00:04:822");
strcpy(metmast_TH[49].Timestamp_UTC, "29-Mar-2021 00:00:04:921");
strcpy(metmast_TH[50].Timestamp_UTC, "29-Mar-2021 00:00:05:021");
strcpy(metmast_TH[51].Timestamp_UTC, "29-Mar-2021 00:00:05:130");
strcpy(metmast_TH[52].Timestamp_UTC, "29-Mar-2021 00:00:05:228");
strcpy(metmast_TH[53].Timestamp_UTC, "29-Mar-2021 00:00:05:327");
strcpy(metmast_TH[54].Timestamp_UTC, "29-Mar-2021 00:00:05:426");
strcpy(metmast_TH[55].Timestamp_UTC, "29-Mar-2021 00:00:05:525");
strcpy(metmast_TH[56].Timestamp_UTC, "29-Mar-2021 00:00:05:624");
strcpy(metmast_TH[57].Timestamp_UTC, "29-Mar-2021 00:00:05:723");
strcpy(metmast_TH[58].Timestamp_UTC, "29-Mar-2021 00:00:05:822");
strcpy(metmast_TH[59].Timestamp_UTC, "29-Mar-2021 00:00:05:921");
strcpy(metmast_TH[60].Timestamp_UTC, "29-Mar-2021 00:00:06:031");
strcpy(metmast_TH[61].Timestamp_UTC, "29-Mar-2021 00:00:06:130");
strcpy(metmast_TH[62].Timestamp_UTC, "29-Mar-2021 00:00:06:228");
strcpy(metmast_TH[63].Timestamp_UTC, "29-Mar-2021 00:00:06:327");
strcpy(metmast_TH[64].Timestamp_UTC, "29-Mar-2021 00:00:06:426");
strcpy(metmast_TH[65].Timestamp_UTC, "29-Mar-2021 00:00:06:525");
strcpy(metmast_TH[66].Timestamp_UTC, "29-Mar-2021 00:00:06:624");
strcpy(metmast_TH[67].Timestamp_UTC, "29-Mar-2021 00:00:06:723");
strcpy(metmast_TH[68].Timestamp_UTC, "29-Mar-2021 00:00:06:822");
strcpy(metmast_TH[69].Timestamp_UTC, "29-Mar-2021 00:00:06:931");
strcpy(metmast_TH[70].Timestamp_UTC, "29-Mar-2021 00:00:07:031");
strcpy(metmast_TH[71].Timestamp_UTC, "29-Mar-2021 00:00:07:130");
strcpy(metmast_TH[72].Timestamp_UTC, "29-Mar-2021 00:00:07:228");
strcpy(metmast_TH[73].Timestamp_UTC, "29-Mar-2021 00:00:07:327");
strcpy(metmast_TH[74].Timestamp_UTC, "29-Mar-2021 00:00:07:426");
strcpy(metmast_TH[75].Timestamp_UTC, "29-Mar-2021 00:00:07:525");
strcpy(metmast_TH[76].Timestamp_UTC, "29-Mar-2021 00:00:07:624");
strcpy(metmast_TH[77].Timestamp_UTC, "29-Mar-2021 00:00:07:723");
strcpy(metmast_TH[78].Timestamp_UTC, "29-Mar-2021 00:00:07:832");
strcpy(metmast_TH[79].Timestamp_UTC, "29-Mar-2021 00:00:07:931");
strcpy(metmast_TH[80].Timestamp_UTC, "29-Mar-2021 00:00:08:031");
strcpy(metmast_TH[81].Timestamp_UTC, "29-Mar-2021 00:00:08:130");
strcpy(metmast_TH[82].Timestamp_UTC, "29-Mar-2021 00:00:08:228");
strcpy(metmast_TH[83].Timestamp_UTC, "29-Mar-2021 00:00:08:327");
strcpy(metmast_TH[84].Timestamp_UTC, "29-Mar-2021 00:00:08:426");
strcpy(metmast_TH[85].Timestamp_UTC, "29-Mar-2021 00:00:08:525");
strcpy(metmast_TH[86].Timestamp_UTC, "29-Mar-2021 00:00:08:624");
strcpy(metmast_TH[87].Timestamp_UTC, "29-Mar-2021 00:00:08:723");
strcpy(metmast_TH[88].Timestamp_UTC, "29-Mar-2021 00:00:08:832");
strcpy(metmast_TH[89].Timestamp_UTC, "29-Mar-2021 00:00:08:931");
strcpy(metmast_TH[90].Timestamp_UTC, "29-Mar-2021 00:00:09:031");
strcpy(metmast_TH[91].Timestamp_UTC, "29-Mar-2021 00:00:09:130");
strcpy(metmast_TH[92].Timestamp_UTC, "29-Mar-2021 00:00:09:228");
strcpy(metmast_TH[93].Timestamp_UTC, "29-Mar-2021 00:00:09:327");
strcpy(metmast_TH[94].Timestamp_UTC, "29-Mar-2021 00:00:09:426");
strcpy(metmast_TH[95].Timestamp_UTC, "29-Mar-2021 00:00:09:525");
strcpy(metmast_TH[96].Timestamp_UTC, "29-Mar-2021 00:00:09:624");
strcpy(metmast_TH[97].Timestamp_UTC, "29-Mar-2021 00:00:09:733");
strcpy(metmast_TH[98].Timestamp_UTC, "29-Mar-2021 00:00:09:832");
strcpy(metmast_TH[99].Timestamp_UTC, "29-Mar-2021 00:00:09:931");
strcpy(metmast_TH[100].Timestamp_UTC, "29-Mar-2021 00:00:10:031");
strcpy(metmast_TH[101].Timestamp_UTC, "29-Mar-2021 00:00:10:130");
strcpy(metmast_TH[102].Timestamp_UTC, "29-Mar-2021 00:00:10:228");
strcpy(metmast_TH[103].Timestamp_UTC, "29-Mar-2021 00:00:10:327");
strcpy(metmast_TH[104].Timestamp_UTC, "29-Mar-2021 00:00:10:426");
strcpy(metmast_TH[105].Timestamp_UTC, "29-Mar-2021 00:00:10:525");
strcpy(metmast_TH[106].Timestamp_UTC, "29-Mar-2021 00:00:10:634");
strcpy(metmast_TH[107].Timestamp_UTC, "29-Mar-2021 00:00:10:733");
strcpy(metmast_TH[108].Timestamp_UTC, "29-Mar-2021 00:00:10:832");
strcpy(metmast_TH[109].Timestamp_UTC, "29-Mar-2021 00:00:10:931");
strcpy(metmast_TH[110].Timestamp_UTC, "29-Mar-2021 00:00:11:021");
strcpy(metmast_TH[111].Timestamp_UTC, "29-Mar-2021 00:00:11:120");
strcpy(metmast_TH[112].Timestamp_UTC, "29-Mar-2021 00:00:11:219");
strcpy(metmast_TH[113].Timestamp_UTC, "29-Mar-2021 00:00:11:317");
strcpy(metmast_TH[114].Timestamp_UTC, "29-Mar-2021 00:00:11:416");
strcpy(metmast_TH[115].Timestamp_UTC, "29-Mar-2021 00:00:11:525");
strcpy(metmast_TH[116].Timestamp_UTC, "29-Mar-2021 00:00:11:624");
strcpy(metmast_TH[117].Timestamp_UTC, "29-Mar-2021 00:00:11:792");
strcpy(metmast_TH[118].Timestamp_UTC, "29-Mar-2021 00:00:11:822");
strcpy(metmast_TH[119].Timestamp_UTC, "29-Mar-2021 00:00:11:921");
strcpy(metmast_TH[120].Timestamp_UTC, "29-Mar-2021 00:00:12:021");
strcpy(metmast_TH[121].Timestamp_UTC, "29-Mar-2021 00:00:12:120");
strcpy(metmast_TH[122].Timestamp_UTC, "29-Mar-2021 00:00:12:219");
strcpy(metmast_TH[123].Timestamp_UTC, "29-Mar-2021 00:00:12:317");
strcpy(metmast_TH[124].Timestamp_UTC, "29-Mar-2021 00:00:12:426");
strcpy(metmast_TH[125].Timestamp_UTC, "29-Mar-2021 00:00:12:525");
strcpy(metmast_TH[126].Timestamp_UTC, "29-Mar-2021 00:00:12:624");
strcpy(metmast_TH[127].Timestamp_UTC, "29-Mar-2021 00:00:12:723");
strcpy(metmast_TH[128].Timestamp_UTC, "29-Mar-2021 00:00:12:822");
strcpy(metmast_TH[129].Timestamp_UTC, "29-Mar-2021 00:00:12:921");
strcpy(metmast_TH[130].Timestamp_UTC, "29-Mar-2021 00:00:13:021");
strcpy(metmast_TH[131].Timestamp_UTC, "29-Mar-2021 00:00:13:120");
strcpy(metmast_TH[132].Timestamp_UTC, "29-Mar-2021 00:00:13:219");
strcpy(metmast_TH[133].Timestamp_UTC, "29-Mar-2021 00:00:13:327");
strcpy(metmast_TH[134].Timestamp_UTC, "29-Mar-2021 00:00:13:426");
strcpy(metmast_TH[135].Timestamp_UTC, "29-Mar-2021 00:00:13:525");
strcpy(metmast_TH[136].Timestamp_UTC, "29-Mar-2021 00:00:13:624");
strcpy(metmast_TH[137].Timestamp_UTC, "29-Mar-2021 00:00:13:723");
strcpy(metmast_TH[138].Timestamp_UTC, "29-Mar-2021 00:00:13:822");
strcpy(metmast_TH[139].Timestamp_UTC, "29-Mar-2021 00:00:13:921");
strcpy(metmast_TH[140].Timestamp_UTC, "29-Mar-2021 00:00:14:021");
strcpy(metmast_TH[141].Timestamp_UTC, "29-Mar-2021 00:00:14:120");
strcpy(metmast_TH[142].Timestamp_UTC, "29-Mar-2021 00:00:14:228");
strcpy(metmast_TH[143].Timestamp_UTC, "29-Mar-2021 00:00:14:327");
strcpy(metmast_TH[144].Timestamp_UTC, "29-Mar-2021 00:00:14:426");
strcpy(metmast_TH[145].Timestamp_UTC, "29-Mar-2021 00:00:14:525");
strcpy(metmast_TH[146].Timestamp_UTC, "29-Mar-2021 00:00:14:624");
strcpy(metmast_TH[147].Timestamp_UTC, "29-Mar-2021 00:00:14:723");
strcpy(metmast_TH[148].Timestamp_UTC, "29-Mar-2021 00:00:14:822");
strcpy(metmast_TH[149].Timestamp_UTC, "29-Mar-2021 00:00:14:921");
strcpy(metmast_TH[150].Timestamp_UTC, "29-Mar-2021 00:00:15:021");
strcpy(metmast_TH[151].Timestamp_UTC, "29-Mar-2021 00:00:15:130");
strcpy(metmast_TH[152].Timestamp_UTC, "29-Mar-2021 00:00:15:228");
strcpy(metmast_TH[153].Timestamp_UTC, "29-Mar-2021 00:00:15:327");
strcpy(metmast_TH[154].Timestamp_UTC, "29-Mar-2021 00:00:15:426");
strcpy(metmast_TH[155].Timestamp_UTC, "29-Mar-2021 00:00:15:525");
strcpy(metmast_TH[156].Timestamp_UTC, "29-Mar-2021 00:00:15:624");
strcpy(metmast_TH[157].Timestamp_UTC, "29-Mar-2021 00:00:15:723");
strcpy(metmast_TH[158].Timestamp_UTC, "29-Mar-2021 00:00:15:822");
strcpy(metmast_TH[159].Timestamp_UTC, "29-Mar-2021 00:00:15:921");
strcpy(metmast_TH[160].Timestamp_UTC, "29-Mar-2021 00:00:16:031");
strcpy(metmast_TH[161].Timestamp_UTC, "29-Mar-2021 00:00:16:130");
strcpy(metmast_TH[162].Timestamp_UTC, "29-Mar-2021 00:00:16:228");
strcpy(metmast_TH[163].Timestamp_UTC, "29-Mar-2021 00:00:16:327");
strcpy(metmast_TH[164].Timestamp_UTC, "29-Mar-2021 00:00:16:426");
strcpy(metmast_TH[165].Timestamp_UTC, "29-Mar-2021 00:00:16:525");
strcpy(metmast_TH[166].Timestamp_UTC, "29-Mar-2021 00:00:16:624");
strcpy(metmast_TH[167].Timestamp_UTC, "29-Mar-2021 00:00:16:723");
strcpy(metmast_TH[168].Timestamp_UTC, "29-Mar-2021 00:00:16:822");
strcpy(metmast_TH[169].Timestamp_UTC, "29-Mar-2021 00:00:16:931");
strcpy(metmast_TH[170].Timestamp_UTC, "29-Mar-2021 00:00:17:031");
strcpy(metmast_TH[171].Timestamp_UTC, "29-Mar-2021 00:00:17:130");
strcpy(metmast_TH[172].Timestamp_UTC, "29-Mar-2021 00:00:17:228");
strcpy(metmast_TH[173].Timestamp_UTC, "29-Mar-2021 00:00:17:327");
strcpy(metmast_TH[174].Timestamp_UTC, "29-Mar-2021 00:00:17:426");
strcpy(metmast_TH[175].Timestamp_UTC, "29-Mar-2021 00:00:17:525");
strcpy(metmast_TH[176].Timestamp_UTC, "29-Mar-2021 00:00:17:624");
strcpy(metmast_TH[177].Timestamp_UTC, "29-Mar-2021 00:00:17:723");
strcpy(metmast_TH[178].Timestamp_UTC, "29-Mar-2021 00:00:17:832");
strcpy(metmast_TH[179].Timestamp_UTC, "29-Mar-2021 00:00:17:931");
strcpy(metmast_TH[180].Timestamp_UTC, "29-Mar-2021 00:00:18:031");
strcpy(metmast_TH[181].Timestamp_UTC, "29-Mar-2021 00:00:18:130");
strcpy(metmast_TH[182].Timestamp_UTC, "29-Mar-2021 00:00:18:228");
strcpy(metmast_TH[183].Timestamp_UTC, "29-Mar-2021 00:00:18:327");
strcpy(metmast_TH[184].Timestamp_UTC, "29-Mar-2021 00:00:18:426");
strcpy(metmast_TH[185].Timestamp_UTC, "29-Mar-2021 00:00:18:525");
strcpy(metmast_TH[186].Timestamp_UTC, "29-Mar-2021 00:00:18:624");
strcpy(metmast_TH[187].Timestamp_UTC, "29-Mar-2021 00:00:18:723");
strcpy(metmast_TH[188].Timestamp_UTC, "29-Mar-2021 00:00:18:832");
strcpy(metmast_TH[189].Timestamp_UTC, "29-Mar-2021 00:00:18:931");
strcpy(metmast_TH[190].Timestamp_UTC, "29-Mar-2021 00:00:19:031");
strcpy(metmast_TH[191].Timestamp_UTC, "29-Mar-2021 00:00:19:130");
strcpy(metmast_TH[192].Timestamp_UTC, "29-Mar-2021 00:00:19:228");
strcpy(metmast_TH[193].Timestamp_UTC, "29-Mar-2021 00:00:19:327");
strcpy(metmast_TH[194].Timestamp_UTC, "29-Mar-2021 00:00:19:426");
strcpy(metmast_TH[195].Timestamp_UTC, "29-Mar-2021 00:00:19:525");
strcpy(metmast_TH[196].Timestamp_UTC, "29-Mar-2021 00:00:19:624");
strcpy(metmast_TH[197].Timestamp_UTC, "29-Mar-2021 00:00:19:733");
strcpy(metmast_TH[198].Timestamp_UTC, "29-Mar-2021 00:00:19:832");
strcpy(metmast_TH[199].Timestamp_UTC, "29-Mar-2021 00:00:19:931");
strcpy(metmast_TH[200].Timestamp_UTC, "29-Mar-2021 00:00:20:021");
strcpy(metmast_TH[201].Timestamp_UTC, "29-Mar-2021 00:00:20:120");
strcpy(metmast_TH[202].Timestamp_UTC, "29-Mar-2021 00:00:20:219");
strcpy(metmast_TH[203].Timestamp_UTC, "29-Mar-2021 00:00:20:317");
strcpy(metmast_TH[204].Timestamp_UTC, "29-Mar-2021 00:00:20:416");
strcpy(metmast_TH[205].Timestamp_UTC, "29-Mar-2021 00:00:20:515");
strcpy(metmast_TH[206].Timestamp_UTC, "29-Mar-2021 00:00:20:624");
strcpy(metmast_TH[207].Timestamp_UTC, "29-Mar-2021 00:00:20:723");
strcpy(metmast_TH[208].Timestamp_UTC, "29-Mar-2021 00:00:20:822");
strcpy(metmast_TH[209].Timestamp_UTC, "29-Mar-2021 00:00:20:921");
strcpy(metmast_TH[210].Timestamp_UTC, "29-Mar-2021 00:00:21:021");
strcpy(metmast_TH[211].Timestamp_UTC, "29-Mar-2021 00:00:21:120");
strcpy(metmast_TH[212].Timestamp_UTC, "29-Mar-2021 00:00:21:219");
strcpy(metmast_TH[213].Timestamp_UTC, "29-Mar-2021 00:00:21:317");
strcpy(metmast_TH[214].Timestamp_UTC, "29-Mar-2021 00:00:21:416");
strcpy(metmast_TH[215].Timestamp_UTC, "29-Mar-2021 00:00:21:525");
strcpy(metmast_TH[216].Timestamp_UTC, "29-Mar-2021 00:00:21:624");
strcpy(metmast_TH[217].Timestamp_UTC, "29-Mar-2021 00:00:21:723");
strcpy(metmast_TH[218].Timestamp_UTC, "29-Mar-2021 00:00:21:822");
strcpy(metmast_TH[219].Timestamp_UTC, "29-Mar-2021 00:00:21:921");
strcpy(metmast_TH[220].Timestamp_UTC, "29-Mar-2021 00:00:22:021");
strcpy(metmast_TH[221].Timestamp_UTC, "29-Mar-2021 00:00:22:120");
strcpy(metmast_TH[222].Timestamp_UTC, "29-Mar-2021 00:00:22:219");
strcpy(metmast_TH[223].Timestamp_UTC, "29-Mar-2021 00:00:22:317");
strcpy(metmast_TH[224].Timestamp_UTC, "29-Mar-2021 00:00:22:426");
strcpy(metmast_TH[225].Timestamp_UTC, "29-Mar-2021 00:00:22:525");
strcpy(metmast_TH[226].Timestamp_UTC, "29-Mar-2021 00:00:22:624");
strcpy(metmast_TH[227].Timestamp_UTC, "29-Mar-2021 00:00:22:723");
strcpy(metmast_TH[228].Timestamp_UTC, "29-Mar-2021 00:00:22:822");
strcpy(metmast_TH[229].Timestamp_UTC, "29-Mar-2021 00:00:22:921");
strcpy(metmast_TH[230].Timestamp_UTC, "29-Mar-2021 00:00:23:021");
strcpy(metmast_TH[231].Timestamp_UTC, "29-Mar-2021 00:00:23:120");
strcpy(metmast_TH[232].Timestamp_UTC, "29-Mar-2021 00:00:23:219");
strcpy(metmast_TH[233].Timestamp_UTC, "29-Mar-2021 00:00:23:327");
strcpy(metmast_TH[234].Timestamp_UTC, "29-Mar-2021 00:00:23:426");
strcpy(metmast_TH[235].Timestamp_UTC, "29-Mar-2021 00:00:23:525");
strcpy(metmast_TH[236].Timestamp_UTC, "29-Mar-2021 00:00:23:624");
strcpy(metmast_TH[237].Timestamp_UTC, "29-Mar-2021 00:00:23:723");
strcpy(metmast_TH[238].Timestamp_UTC, "29-Mar-2021 00:00:23:822");
strcpy(metmast_TH[239].Timestamp_UTC, "29-Mar-2021 00:00:23:921");
strcpy(metmast_TH[240].Timestamp_UTC, "29-Mar-2021 00:00:24:021");
strcpy(metmast_TH[241].Timestamp_UTC, "29-Mar-2021 00:00:24:120");
strcpy(metmast_TH[242].Timestamp_UTC, "29-Mar-2021 00:00:24:228");
strcpy(metmast_TH[243].Timestamp_UTC, "29-Mar-2021 00:00:24:327");
strcpy(metmast_TH[244].Timestamp_UTC, "29-Mar-2021 00:00:24:426");
strcpy(metmast_TH[245].Timestamp_UTC, "29-Mar-2021 00:00:24:525");
strcpy(metmast_TH[246].Timestamp_UTC, "29-Mar-2021 00:00:24:624");
strcpy(metmast_TH[247].Timestamp_UTC, "29-Mar-2021 00:00:24:723");
strcpy(metmast_TH[248].Timestamp_UTC, "29-Mar-2021 00:00:24:822");
strcpy(metmast_TH[249].Timestamp_UTC, "29-Mar-2021 00:00:24:921");
strcpy(metmast_TH[250].Timestamp_UTC, "29-Mar-2021 00:00:25:021");
strcpy(metmast_TH[251].Timestamp_UTC, "29-Mar-2021 00:00:25:130");
strcpy(metmast_TH[252].Timestamp_UTC, "29-Mar-2021 00:00:25:228");
strcpy(metmast_TH[253].Timestamp_UTC, "29-Mar-2021 00:00:25:327");
strcpy(metmast_TH[254].Timestamp_UTC, "29-Mar-2021 00:00:25:505");
strcpy(metmast_TH[255].Timestamp_UTC, "29-Mar-2021 00:00:25:525");
strcpy(metmast_TH[256].Timestamp_UTC, "29-Mar-2021 00:00:25:624");
strcpy(metmast_TH[257].Timestamp_UTC, "29-Mar-2021 00:00:25:733");
strcpy(metmast_TH[258].Timestamp_UTC, "29-Mar-2021 00:00:25:822");
strcpy(metmast_TH[259].Timestamp_UTC, "29-Mar-2021 00:00:25:921");
strcpy(metmast_TH[260].Timestamp_UTC, "29-Mar-2021 00:00:26:031");
strcpy(metmast_TH[261].Timestamp_UTC, "29-Mar-2021 00:00:26:130");
strcpy(metmast_TH[262].Timestamp_UTC, "29-Mar-2021 00:00:26:228");
strcpy(metmast_TH[263].Timestamp_UTC, "29-Mar-2021 00:00:26:327");
strcpy(metmast_TH[264].Timestamp_UTC, "29-Mar-2021 00:00:26:426");
strcpy(metmast_TH[265].Timestamp_UTC, "29-Mar-2021 00:00:26:525");
strcpy(metmast_TH[266].Timestamp_UTC, "29-Mar-2021 00:00:26:624");
strcpy(metmast_TH[267].Timestamp_UTC, "29-Mar-2021 00:00:26:723");
strcpy(metmast_TH[268].Timestamp_UTC, "29-Mar-2021 00:00:26:822");
strcpy(metmast_TH[269].Timestamp_UTC, "29-Mar-2021 00:00:26:931");
strcpy(metmast_TH[270].Timestamp_UTC, "29-Mar-2021 00:00:27:031");
strcpy(metmast_TH[271].Timestamp_UTC, "29-Mar-2021 00:00:27:130");
strcpy(metmast_TH[272].Timestamp_UTC, "29-Mar-2021 00:00:27:228");
strcpy(metmast_TH[273].Timestamp_UTC, "29-Mar-2021 00:00:27:327");
strcpy(metmast_TH[274].Timestamp_UTC, "29-Mar-2021 00:00:27:426");
strcpy(metmast_TH[275].Timestamp_UTC, "29-Mar-2021 00:00:27:525");
strcpy(metmast_TH[276].Timestamp_UTC, "29-Mar-2021 00:00:27:624");
strcpy(metmast_TH[277].Timestamp_UTC, "29-Mar-2021 00:00:27:723");
strcpy(metmast_TH[278].Timestamp_UTC, "29-Mar-2021 00:00:27:832");
strcpy(metmast_TH[279].Timestamp_UTC, "29-Mar-2021 00:00:27:931");
strcpy(metmast_TH[280].Timestamp_UTC, "29-Mar-2021 00:00:28:031");
strcpy(metmast_TH[281].Timestamp_UTC, "29-Mar-2021 00:00:28:130");
strcpy(metmast_TH[282].Timestamp_UTC, "29-Mar-2021 00:00:28:228");
strcpy(metmast_TH[283].Timestamp_UTC, "29-Mar-2021 00:00:28:327");
strcpy(metmast_TH[284].Timestamp_UTC, "29-Mar-2021 00:00:28:426");
strcpy(metmast_TH[285].Timestamp_UTC, "29-Mar-2021 00:00:28:525");
strcpy(metmast_TH[286].Timestamp_UTC, "29-Mar-2021 00:00:28:624");
strcpy(metmast_TH[287].Timestamp_UTC, "29-Mar-2021 00:00:28:723");
strcpy(metmast_TH[288].Timestamp_UTC, "29-Mar-2021 00:00:28:832");
strcpy(metmast_TH[289].Timestamp_UTC, "29-Mar-2021 00:00:28:931");
strcpy(metmast_TH[290].Timestamp_UTC, "29-Mar-2021 00:00:29:021");
strcpy(metmast_TH[291].Timestamp_UTC, "29-Mar-2021 00:00:29:120");
strcpy(metmast_TH[292].Timestamp_UTC, "29-Mar-2021 00:00:29:219");
strcpy(metmast_TH[293].Timestamp_UTC, "29-Mar-2021 00:00:29:317");
strcpy(metmast_TH[294].Timestamp_UTC, "29-Mar-2021 00:00:29:416");
strcpy(metmast_TH[295].Timestamp_UTC, "29-Mar-2021 00:00:29:515");
strcpy(metmast_TH[296].Timestamp_UTC, "29-Mar-2021 00:00:29:614");
strcpy(metmast_TH[297].Timestamp_UTC, "29-Mar-2021 00:00:29:723");
strcpy(metmast_TH[298].Timestamp_UTC, "29-Mar-2021 00:00:29:822");
strcpy(metmast_TH[299].Timestamp_UTC, "29-Mar-2021 00:00:29:921");
strcpy(metmast_TH[300].Timestamp_UTC, "29-Mar-2021 00:00:30:021");
strcpy(metmast_TH[301].Timestamp_UTC, "29-Mar-2021 00:00:30:120");
strcpy(metmast_TH[302].Timestamp_UTC, "29-Mar-2021 00:00:30:219");
strcpy(metmast_TH[303].Timestamp_UTC, "29-Mar-2021 00:00:30:317");
strcpy(metmast_TH[304].Timestamp_UTC, "29-Mar-2021 00:00:30:416");
strcpy(metmast_TH[305].Timestamp_UTC, "29-Mar-2021 00:00:30:515");
strcpy(metmast_TH[306].Timestamp_UTC, "29-Mar-2021 00:00:30:624");
strcpy(metmast_TH[307].Timestamp_UTC, "29-Mar-2021 00:00:30:723");
strcpy(metmast_TH[308].Timestamp_UTC, "29-Mar-2021 00:00:30:822");
strcpy(metmast_TH[309].Timestamp_UTC, "29-Mar-2021 00:00:30:921");
strcpy(metmast_TH[310].Timestamp_UTC, "29-Mar-2021 00:00:31:021");
strcpy(metmast_TH[311].Timestamp_UTC, "29-Mar-2021 00:00:31:120");
strcpy(metmast_TH[312].Timestamp_UTC, "29-Mar-2021 00:00:31:219");
strcpy(metmast_TH[313].Timestamp_UTC, "29-Mar-2021 00:00:31:317");
strcpy(metmast_TH[314].Timestamp_UTC, "29-Mar-2021 00:00:31:416");
strcpy(metmast_TH[315].Timestamp_UTC, "29-Mar-2021 00:00:31:525");
strcpy(metmast_TH[316].Timestamp_UTC, "29-Mar-2021 00:00:31:624");
strcpy(metmast_TH[317].Timestamp_UTC, "29-Mar-2021 00:00:31:723");
strcpy(metmast_TH[318].Timestamp_UTC, "29-Mar-2021 00:00:31:822");
strcpy(metmast_TH[319].Timestamp_UTC, "29-Mar-2021 00:00:31:921");
strcpy(metmast_TH[320].Timestamp_UTC, "29-Mar-2021 00:00:32:021");
strcpy(metmast_TH[321].Timestamp_UTC, "29-Mar-2021 00:00:32:120");
strcpy(metmast_TH[322].Timestamp_UTC, "29-Mar-2021 00:00:32:219");
strcpy(metmast_TH[323].Timestamp_UTC, "29-Mar-2021 00:00:32:317");
strcpy(metmast_TH[324].Timestamp_UTC, "29-Mar-2021 00:00:32:426");
strcpy(metmast_TH[325].Timestamp_UTC, "29-Mar-2021 00:00:32:525");
strcpy(metmast_TH[326].Timestamp_UTC, "29-Mar-2021 00:00:32:624");
strcpy(metmast_TH[327].Timestamp_UTC, "29-Mar-2021 00:00:32:723");
strcpy(metmast_TH[328].Timestamp_UTC, "29-Mar-2021 00:00:32:822");
strcpy(metmast_TH[329].Timestamp_UTC, "29-Mar-2021 00:00:32:921");
strcpy(metmast_TH[330].Timestamp_UTC, "29-Mar-2021 00:00:33:021");
strcpy(metmast_TH[331].Timestamp_UTC, "29-Mar-2021 00:00:33:120");
strcpy(metmast_TH[332].Timestamp_UTC, "29-Mar-2021 00:00:33:219");
strcpy(metmast_TH[333].Timestamp_UTC, "29-Mar-2021 00:00:33:327");
strcpy(metmast_TH[334].Timestamp_UTC, "29-Mar-2021 00:00:33:426");
strcpy(metmast_TH[335].Timestamp_UTC, "29-Mar-2021 00:00:33:525");
strcpy(metmast_TH[336].Timestamp_UTC, "29-Mar-2021 00:00:33:624");
strcpy(metmast_TH[337].Timestamp_UTC, "29-Mar-2021 00:00:33:723");
strcpy(metmast_TH[338].Timestamp_UTC, "29-Mar-2021 00:00:33:822");
strcpy(metmast_TH[339].Timestamp_UTC, "29-Mar-2021 00:00:33:921");
strcpy(metmast_TH[340].Timestamp_UTC, "29-Mar-2021 00:00:34:021");
strcpy(metmast_TH[341].Timestamp_UTC, "29-Mar-2021 00:00:34:120");
strcpy(metmast_TH[342].Timestamp_UTC, "29-Mar-2021 00:00:34:228");
strcpy(metmast_TH[343].Timestamp_UTC, "29-Mar-2021 00:00:34:327");
strcpy(metmast_TH[344].Timestamp_UTC, "29-Mar-2021 00:00:34:426");
strcpy(metmast_TH[345].Timestamp_UTC, "29-Mar-2021 00:00:34:525");
strcpy(metmast_TH[346].Timestamp_UTC, "29-Mar-2021 00:00:34:624");
strcpy(metmast_TH[347].Timestamp_UTC, "29-Mar-2021 00:00:34:723");
strcpy(metmast_TH[348].Timestamp_UTC, "29-Mar-2021 00:00:34:822");
strcpy(metmast_TH[349].Timestamp_UTC, "29-Mar-2021 00:00:34:921");
strcpy(metmast_TH[350].Timestamp_UTC, "29-Mar-2021 00:00:35:021");
strcpy(metmast_TH[351].Timestamp_UTC, "29-Mar-2021 00:00:35:130");
strcpy(metmast_TH[352].Timestamp_UTC, "29-Mar-2021 00:00:35:228");
strcpy(metmast_TH[353].Timestamp_UTC, "29-Mar-2021 00:00:35:327");
strcpy(metmast_TH[354].Timestamp_UTC, "29-Mar-2021 00:00:35:426");
strcpy(metmast_TH[355].Timestamp_UTC, "29-Mar-2021 00:00:35:525");
strcpy(metmast_TH[356].Timestamp_UTC, "29-Mar-2021 00:00:35:624");
strcpy(metmast_TH[357].Timestamp_UTC, "29-Mar-2021 00:00:35:723");
strcpy(metmast_TH[358].Timestamp_UTC, "29-Mar-2021 00:00:35:822");
strcpy(metmast_TH[359].Timestamp_UTC, "29-Mar-2021 00:00:35:921");
strcpy(metmast_TH[360].Timestamp_UTC, "29-Mar-2021 00:00:36:031");
strcpy(metmast_TH[361].Timestamp_UTC, "29-Mar-2021 00:00:36:130");
strcpy(metmast_TH[362].Timestamp_UTC, "29-Mar-2021 00:00:36:228");
strcpy(metmast_TH[363].Timestamp_UTC, "29-Mar-2021 00:00:36:327");
strcpy(metmast_TH[364].Timestamp_UTC, "29-Mar-2021 00:00:36:426");
strcpy(metmast_TH[365].Timestamp_UTC, "29-Mar-2021 00:00:36:525");
strcpy(metmast_TH[366].Timestamp_UTC, "29-Mar-2021 00:00:36:624");
strcpy(metmast_TH[367].Timestamp_UTC, "29-Mar-2021 00:00:36:723");
strcpy(metmast_TH[368].Timestamp_UTC, "29-Mar-2021 00:00:36:822");
strcpy(metmast_TH[369].Timestamp_UTC, "29-Mar-2021 00:00:36:931");
strcpy(metmast_TH[370].Timestamp_UTC, "29-Mar-2021 00:00:37:031");
strcpy(metmast_TH[371].Timestamp_UTC, "29-Mar-2021 00:00:37:130");
strcpy(metmast_TH[372].Timestamp_UTC, "29-Mar-2021 00:00:37:228");
strcpy(metmast_TH[373].Timestamp_UTC, "29-Mar-2021 00:00:37:327");
strcpy(metmast_TH[374].Timestamp_UTC, "29-Mar-2021 00:00:37:426");
strcpy(metmast_TH[375].Timestamp_UTC, "29-Mar-2021 00:00:37:525");
strcpy(metmast_TH[376].Timestamp_UTC, "29-Mar-2021 00:00:37:624");
strcpy(metmast_TH[377].Timestamp_UTC, "29-Mar-2021 00:00:37:723");
strcpy(metmast_TH[378].Timestamp_UTC, "29-Mar-2021 00:00:37:832");
strcpy(metmast_TH[379].Timestamp_UTC, "29-Mar-2021 00:00:37:931");
strcpy(metmast_TH[380].Timestamp_UTC, "29-Mar-2021 00:00:38:021");
strcpy(metmast_TH[381].Timestamp_UTC, "29-Mar-2021 00:00:38:120");
strcpy(metmast_TH[382].Timestamp_UTC, "29-Mar-2021 00:00:38:219");
strcpy(metmast_TH[383].Timestamp_UTC, "29-Mar-2021 00:00:38:317");
strcpy(metmast_TH[384].Timestamp_UTC, "29-Mar-2021 00:00:38:416");
strcpy(metmast_TH[385].Timestamp_UTC, "29-Mar-2021 00:00:38:515");
strcpy(metmast_TH[386].Timestamp_UTC, "29-Mar-2021 00:00:38:614");
strcpy(metmast_TH[387].Timestamp_UTC, "29-Mar-2021 00:00:38:713");
strcpy(metmast_TH[388].Timestamp_UTC, "29-Mar-2021 00:00:38:822");
strcpy(metmast_TH[389].Timestamp_UTC, "29-Mar-2021 00:00:38:921");
strcpy(metmast_TH[390].Timestamp_UTC, "29-Mar-2021 00:00:39:021");
strcpy(metmast_TH[391].Timestamp_UTC, "29-Mar-2021 00:00:39:120");
strcpy(metmast_TH[392].Timestamp_UTC, "29-Mar-2021 00:00:39:219");
strcpy(metmast_TH[393].Timestamp_UTC, "29-Mar-2021 00:00:39:317");
strcpy(metmast_TH[394].Timestamp_UTC, "29-Mar-2021 00:00:39:416");
strcpy(metmast_TH[395].Timestamp_UTC, "29-Mar-2021 00:00:39:515");
strcpy(metmast_TH[396].Timestamp_UTC, "29-Mar-2021 00:00:39:614");
strcpy(metmast_TH[397].Timestamp_UTC, "29-Mar-2021 00:00:39:723");
strcpy(metmast_TH[398].Timestamp_UTC, "29-Mar-2021 00:00:39:822");
strcpy(metmast_TH[399].Timestamp_UTC, "29-Mar-2021 00:00:39:921");
strcpy(metmast_TH[400].Timestamp_UTC, "29-Mar-2021 00:00:40:021");
strcpy(metmast_TH[401].Timestamp_UTC, "29-Mar-2021 00:00:40:120");
strcpy(metmast_TH[402].Timestamp_UTC, "29-Mar-2021 00:00:40:219");
strcpy(metmast_TH[403].Timestamp_UTC, "29-Mar-2021 00:00:40:317");
strcpy(metmast_TH[404].Timestamp_UTC, "29-Mar-2021 00:00:40:416");
strcpy(metmast_TH[405].Timestamp_UTC, "29-Mar-2021 00:00:40:515");
strcpy(metmast_TH[406].Timestamp_UTC, "29-Mar-2021 00:00:40:624");
strcpy(metmast_TH[407].Timestamp_UTC, "29-Mar-2021 00:00:40:723");
strcpy(metmast_TH[408].Timestamp_UTC, "29-Mar-2021 00:00:40:822");
strcpy(metmast_TH[409].Timestamp_UTC, "29-Mar-2021 00:00:40:921");
strcpy(metmast_TH[410].Timestamp_UTC, "29-Mar-2021 00:00:41:021");
strcpy(metmast_TH[411].Timestamp_UTC, "29-Mar-2021 00:00:41:120");
strcpy(metmast_TH[412].Timestamp_UTC, "29-Mar-2021 00:00:41:219");
strcpy(metmast_TH[413].Timestamp_UTC, "29-Mar-2021 00:00:41:317");
strcpy(metmast_TH[414].Timestamp_UTC, "29-Mar-2021 00:00:41:416");
strcpy(metmast_TH[415].Timestamp_UTC, "29-Mar-2021 00:00:41:525");
strcpy(metmast_TH[416].Timestamp_UTC, "29-Mar-2021 00:00:41:624");
strcpy(metmast_TH[417].Timestamp_UTC, "29-Mar-2021 00:00:41:723");
strcpy(metmast_TH[418].Timestamp_UTC, "29-Mar-2021 00:00:41:822");
strcpy(metmast_TH[419].Timestamp_UTC, "29-Mar-2021 00:00:41:921");
strcpy(metmast_TH[420].Timestamp_UTC, "29-Mar-2021 00:00:42:021");
strcpy(metmast_TH[421].Timestamp_UTC, "29-Mar-2021 00:00:42:120");
strcpy(metmast_TH[422].Timestamp_UTC, "29-Mar-2021 00:00:42:219");
strcpy(metmast_TH[423].Timestamp_UTC, "29-Mar-2021 00:00:42:317");
strcpy(metmast_TH[424].Timestamp_UTC, "29-Mar-2021 00:00:42:426");
strcpy(metmast_TH[425].Timestamp_UTC, "29-Mar-2021 00:00:42:525");
strcpy(metmast_TH[426].Timestamp_UTC, "29-Mar-2021 00:00:42:624");
strcpy(metmast_TH[427].Timestamp_UTC, "29-Mar-2021 00:00:42:723");
strcpy(metmast_TH[428].Timestamp_UTC, "29-Mar-2021 00:00:42:822");
strcpy(metmast_TH[429].Timestamp_UTC, "29-Mar-2021 00:00:42:921");
strcpy(metmast_TH[430].Timestamp_UTC, "29-Mar-2021 00:00:43:021");
strcpy(metmast_TH[431].Timestamp_UTC, "29-Mar-2021 00:00:43:120");
strcpy(metmast_TH[432].Timestamp_UTC, "29-Mar-2021 00:00:43:219");
strcpy(metmast_TH[433].Timestamp_UTC, "29-Mar-2021 00:00:43:327");
strcpy(metmast_TH[434].Timestamp_UTC, "29-Mar-2021 00:00:43:426");
strcpy(metmast_TH[435].Timestamp_UTC, "29-Mar-2021 00:00:43:525");
strcpy(metmast_TH[436].Timestamp_UTC, "29-Mar-2021 00:00:43:624");
strcpy(metmast_TH[437].Timestamp_UTC, "29-Mar-2021 00:00:43:723");
strcpy(metmast_TH[438].Timestamp_UTC, "29-Mar-2021 00:00:43:822");
strcpy(metmast_TH[439].Timestamp_UTC, "29-Mar-2021 00:00:43:921");
strcpy(metmast_TH[440].Timestamp_UTC, "29-Mar-2021 00:00:44:021");
strcpy(metmast_TH[441].Timestamp_UTC, "29-Mar-2021 00:00:44:120");
strcpy(metmast_TH[442].Timestamp_UTC, "29-Mar-2021 00:00:44:228");
strcpy(metmast_TH[443].Timestamp_UTC, "29-Mar-2021 00:00:44:327");
strcpy(metmast_TH[444].Timestamp_UTC, "29-Mar-2021 00:00:44:426");
strcpy(metmast_TH[445].Timestamp_UTC, "29-Mar-2021 00:00:44:525");
strcpy(metmast_TH[446].Timestamp_UTC, "29-Mar-2021 00:00:44:624");
strcpy(metmast_TH[447].Timestamp_UTC, "29-Mar-2021 00:00:44:723");
strcpy(metmast_TH[448].Timestamp_UTC, "29-Mar-2021 00:00:44:822");
strcpy(metmast_TH[449].Timestamp_UTC, "29-Mar-2021 00:00:44:921");
strcpy(metmast_TH[450].Timestamp_UTC, "29-Mar-2021 00:00:45:021");
strcpy(metmast_TH[451].Timestamp_UTC, "29-Mar-2021 00:00:45:130");
strcpy(metmast_TH[452].Timestamp_UTC, "29-Mar-2021 00:00:45:228");
strcpy(metmast_TH[453].Timestamp_UTC, "29-Mar-2021 00:00:45:327");
strcpy(metmast_TH[454].Timestamp_UTC, "29-Mar-2021 00:00:45:426");
strcpy(metmast_TH[455].Timestamp_UTC, "29-Mar-2021 00:00:45:525");
strcpy(metmast_TH[456].Timestamp_UTC, "29-Mar-2021 00:00:45:624");
strcpy(metmast_TH[457].Timestamp_UTC, "29-Mar-2021 00:00:45:723");
strcpy(metmast_TH[458].Timestamp_UTC, "29-Mar-2021 00:00:45:822");
strcpy(metmast_TH[459].Timestamp_UTC, "29-Mar-2021 00:00:45:921");
strcpy(metmast_TH[460].Timestamp_UTC, "29-Mar-2021 00:00:46:031");
strcpy(metmast_TH[461].Timestamp_UTC, "29-Mar-2021 00:00:46:130");
strcpy(metmast_TH[462].Timestamp_UTC, "29-Mar-2021 00:00:46:228");
strcpy(metmast_TH[463].Timestamp_UTC, "29-Mar-2021 00:00:46:327");
strcpy(metmast_TH[464].Timestamp_UTC, "29-Mar-2021 00:00:46:426");
strcpy(metmast_TH[465].Timestamp_UTC, "29-Mar-2021 00:00:46:525");
strcpy(metmast_TH[466].Timestamp_UTC, "29-Mar-2021 00:00:46:624");
strcpy(metmast_TH[467].Timestamp_UTC, "29-Mar-2021 00:00:46:723");
strcpy(metmast_TH[468].Timestamp_UTC, "29-Mar-2021 00:00:46:822");
strcpy(metmast_TH[469].Timestamp_UTC, "29-Mar-2021 00:00:46:931");
strcpy(metmast_TH[470].Timestamp_UTC, "29-Mar-2021 00:00:47:021");
strcpy(metmast_TH[471].Timestamp_UTC, "29-Mar-2021 00:00:47:120");
strcpy(metmast_TH[472].Timestamp_UTC, "29-Mar-2021 00:00:47:219");
strcpy(metmast_TH[473].Timestamp_UTC, "29-Mar-2021 00:00:47:317");
strcpy(metmast_TH[474].Timestamp_UTC, "29-Mar-2021 00:00:47:416");
strcpy(metmast_TH[475].Timestamp_UTC, "29-Mar-2021 00:00:47:515");
strcpy(metmast_TH[476].Timestamp_UTC, "29-Mar-2021 00:00:47:614");
strcpy(metmast_TH[477].Timestamp_UTC, "29-Mar-2021 00:00:47:713");
strcpy(metmast_TH[478].Timestamp_UTC, "29-Mar-2021 00:00:47:822");
strcpy(metmast_TH[479].Timestamp_UTC, "29-Mar-2021 00:00:47:921");
strcpy(metmast_TH[480].Timestamp_UTC, "29-Mar-2021 00:00:48:021");
strcpy(metmast_TH[481].Timestamp_UTC, "29-Mar-2021 00:00:48:120");
strcpy(metmast_TH[482].Timestamp_UTC, "29-Mar-2021 00:00:48:219");
strcpy(metmast_TH[483].Timestamp_UTC, "29-Mar-2021 00:00:48:317");
strcpy(metmast_TH[484].Timestamp_UTC, "29-Mar-2021 00:00:48:416");
strcpy(metmast_TH[485].Timestamp_UTC, "29-Mar-2021 00:00:48:515");
strcpy(metmast_TH[486].Timestamp_UTC, "29-Mar-2021 00:00:48:614");
strcpy(metmast_TH[487].Timestamp_UTC, "29-Mar-2021 00:00:48:713");
strcpy(metmast_TH[488].Timestamp_UTC, "29-Mar-2021 00:00:48:822");
strcpy(metmast_TH[489].Timestamp_UTC, "29-Mar-2021 00:00:48:921");
strcpy(metmast_TH[490].Timestamp_UTC, "29-Mar-2021 00:00:49:021");
strcpy(metmast_TH[491].Timestamp_UTC, "29-Mar-2021 00:00:49:120");
strcpy(metmast_TH[492].Timestamp_UTC, "29-Mar-2021 00:00:49:219");
strcpy(metmast_TH[493].Timestamp_UTC, "29-Mar-2021 00:00:49:317");
strcpy(metmast_TH[494].Timestamp_UTC, "29-Mar-2021 00:00:49:416");
strcpy(metmast_TH[495].Timestamp_UTC, "29-Mar-2021 00:00:49:515");
strcpy(metmast_TH[496].Timestamp_UTC, "29-Mar-2021 00:00:49:614");
strcpy(metmast_TH[497].Timestamp_UTC, "29-Mar-2021 00:00:49:723");
strcpy(metmast_TH[498].Timestamp_UTC, "29-Mar-2021 00:00:49:822");
strcpy(metmast_TH[499].Timestamp_UTC, "29-Mar-2021 00:00:49:921");
strcpy(metmast_TH[500].Timestamp_UTC, "29-Mar-2021 00:00:50:021");
strcpy(metmast_TH[501].Timestamp_UTC, "29-Mar-2021 00:00:50:120");
strcpy(metmast_TH[502].Timestamp_UTC, "29-Mar-2021 00:00:50:219");
strcpy(metmast_TH[503].Timestamp_UTC, "29-Mar-2021 00:00:50:317");
strcpy(metmast_TH[504].Timestamp_UTC, "29-Mar-2021 00:00:50:416");
strcpy(metmast_TH[505].Timestamp_UTC, "29-Mar-2021 00:00:50:515");
strcpy(metmast_TH[506].Timestamp_UTC, "29-Mar-2021 00:00:50:624");
strcpy(metmast_TH[507].Timestamp_UTC, "29-Mar-2021 00:00:50:723");
strcpy(metmast_TH[508].Timestamp_UTC, "29-Mar-2021 00:00:50:822");
strcpy(metmast_TH[509].Timestamp_UTC, "29-Mar-2021 00:00:50:921");
strcpy(metmast_TH[510].Timestamp_UTC, "29-Mar-2021 00:00:51:021");
strcpy(metmast_TH[511].Timestamp_UTC, "29-Mar-2021 00:00:51:120");
strcpy(metmast_TH[512].Timestamp_UTC, "29-Mar-2021 00:00:51:219");
strcpy(metmast_TH[513].Timestamp_UTC, "29-Mar-2021 00:00:51:317");
strcpy(metmast_TH[514].Timestamp_UTC, "29-Mar-2021 00:00:51:416");
strcpy(metmast_TH[515].Timestamp_UTC, "29-Mar-2021 00:00:51:525");
strcpy(metmast_TH[516].Timestamp_UTC, "29-Mar-2021 00:00:51:624");
strcpy(metmast_TH[517].Timestamp_UTC, "29-Mar-2021 00:00:51:723");
strcpy(metmast_TH[518].Timestamp_UTC, "29-Mar-2021 00:00:51:822");
strcpy(metmast_TH[519].Timestamp_UTC, "29-Mar-2021 00:00:51:921");
strcpy(metmast_TH[520].Timestamp_UTC, "29-Mar-2021 00:00:52:021");
strcpy(metmast_TH[521].Timestamp_UTC, "29-Mar-2021 00:00:52:120");
strcpy(metmast_TH[522].Timestamp_UTC, "29-Mar-2021 00:00:52:219");
strcpy(metmast_TH[523].Timestamp_UTC, "29-Mar-2021 00:00:52:317");
strcpy(metmast_TH[524].Timestamp_UTC, "29-Mar-2021 00:00:52:426");
strcpy(metmast_TH[525].Timestamp_UTC, "29-Mar-2021 00:00:52:525");
strcpy(metmast_TH[526].Timestamp_UTC, "29-Mar-2021 00:00:52:624");
strcpy(metmast_TH[527].Timestamp_UTC, "29-Mar-2021 00:00:52:723");
strcpy(metmast_TH[528].Timestamp_UTC, "29-Mar-2021 00:00:52:822");
strcpy(metmast_TH[529].Timestamp_UTC, "29-Mar-2021 00:00:52:921");
strcpy(metmast_TH[530].Timestamp_UTC, "29-Mar-2021 00:00:53:021");
strcpy(metmast_TH[531].Timestamp_UTC, "29-Mar-2021 00:00:53:120");
strcpy(metmast_TH[532].Timestamp_UTC, "29-Mar-2021 00:00:53:219");
strcpy(metmast_TH[533].Timestamp_UTC, "29-Mar-2021 00:00:53:327");
strcpy(metmast_TH[534].Timestamp_UTC, "29-Mar-2021 00:00:53:426");
strcpy(metmast_TH[535].Timestamp_UTC, "29-Mar-2021 00:00:53:525");
strcpy(metmast_TH[536].Timestamp_UTC, "29-Mar-2021 00:00:53:624");
strcpy(metmast_TH[537].Timestamp_UTC, "29-Mar-2021 00:00:53:723");
strcpy(metmast_TH[538].Timestamp_UTC, "29-Mar-2021 00:00:53:822");
strcpy(metmast_TH[539].Timestamp_UTC, "29-Mar-2021 00:00:53:921");
strcpy(metmast_TH[540].Timestamp_UTC, "29-Mar-2021 00:00:54:021");
strcpy(metmast_TH[541].Timestamp_UTC, "29-Mar-2021 00:00:54:120");
strcpy(metmast_TH[542].Timestamp_UTC, "29-Mar-2021 00:00:54:228");
strcpy(metmast_TH[543].Timestamp_UTC, "29-Mar-2021 00:00:54:327");
strcpy(metmast_TH[544].Timestamp_UTC, "29-Mar-2021 00:00:54:426");
strcpy(metmast_TH[545].Timestamp_UTC, "29-Mar-2021 00:00:54:525");
strcpy(metmast_TH[546].Timestamp_UTC, "29-Mar-2021 00:00:54:624");
strcpy(metmast_TH[547].Timestamp_UTC, "29-Mar-2021 00:00:54:723");
strcpy(metmast_TH[548].Timestamp_UTC, "29-Mar-2021 00:00:54:822");
strcpy(metmast_TH[549].Timestamp_UTC, "29-Mar-2021 00:00:54:921");
strcpy(metmast_TH[550].Timestamp_UTC, "29-Mar-2021 00:00:55:021");
strcpy(metmast_TH[551].Timestamp_UTC, "29-Mar-2021 00:00:55:130");
strcpy(metmast_TH[552].Timestamp_UTC, "29-Mar-2021 00:00:55:228");
strcpy(metmast_TH[553].Timestamp_UTC, "29-Mar-2021 00:00:55:001");
strcpy(metmast_TH[554].Timestamp_UTC, "29-Mar-2021 00:00:55:100");
strcpy(metmast_TH[555].Timestamp_UTC, "29-Mar-2021 00:00:55:199");
strcpy(metmast_TH[556].Timestamp_UTC, "29-Mar-2021 00:00:55:298");
strcpy(metmast_TH[557].Timestamp_UTC, "29-Mar-2021 00:00:55:397");
strcpy(metmast_TH[558].Timestamp_UTC, "29-Mar-2021 00:00:55:496");
strcpy(metmast_TH[559].Timestamp_UTC, "29-Mar-2021 00:00:55:594");
strcpy(metmast_TH[560].Timestamp_UTC, "29-Mar-2021 00:00:55:703");
strcpy(metmast_TH[561].Timestamp_UTC, "29-Mar-2021 00:00:56:031");
strcpy(metmast_TH[562].Timestamp_UTC, "29-Mar-2021 00:00:56:120");
strcpy(metmast_TH[563].Timestamp_UTC, "29-Mar-2021 00:00:56:219");
strcpy(metmast_TH[564].Timestamp_UTC, "29-Mar-2021 00:00:56:317");
strcpy(metmast_TH[565].Timestamp_UTC, "29-Mar-2021 00:00:56:416");
strcpy(metmast_TH[566].Timestamp_UTC, "29-Mar-2021 00:00:56:515");
strcpy(metmast_TH[567].Timestamp_UTC, "29-Mar-2021 00:00:56:614");
strcpy(metmast_TH[568].Timestamp_UTC, "29-Mar-2021 00:00:56:713");
strcpy(metmast_TH[569].Timestamp_UTC, "29-Mar-2021 00:00:56:822");
strcpy(metmast_TH[570].Timestamp_UTC, "29-Mar-2021 00:00:56:921");
strcpy(metmast_TH[571].Timestamp_UTC, "29-Mar-2021 00:00:57:021");
strcpy(metmast_TH[572].Timestamp_UTC, "29-Mar-2021 00:00:57:120");
strcpy(metmast_TH[573].Timestamp_UTC, "29-Mar-2021 00:00:57:219");
strcpy(metmast_TH[574].Timestamp_UTC, "29-Mar-2021 00:00:57:317");
strcpy(metmast_TH[575].Timestamp_UTC, "29-Mar-2021 00:00:57:416");
strcpy(metmast_TH[576].Timestamp_UTC, "29-Mar-2021 00:00:57:505");
strcpy(metmast_TH[577].Timestamp_UTC, "29-Mar-2021 00:00:57:583");
strcpy(metmast_TH[578].Timestamp_UTC, "29-Mar-2021 00:00:57:670");
strcpy(metmast_TH[579].Timestamp_UTC, "29-Mar-2021 00:00:57:748");
strcpy(metmast_TH[580].Timestamp_UTC, "29-Mar-2021 00:00:57:826");
strcpy(metmast_TH[581].Timestamp_UTC, "29-Mar-2021 00:00:57:904");
strcpy(metmast_TH[582].Timestamp_UTC, "29-Mar-2021 00:00:57:982");
strcpy(metmast_TH[583].Timestamp_UTC, "29-Mar-2021 00:00:58:056");
strcpy(metmast_TH[584].Timestamp_UTC, "29-Mar-2021 00:00:58:134");
strcpy(metmast_TH[585].Timestamp_UTC, "29-Mar-2021 00:00:58:212");
strcpy(metmast_TH[586].Timestamp_UTC, "29-Mar-2021 00:00:58:300");
strcpy(metmast_TH[587].Timestamp_UTC, "29-Mar-2021 00:00:58:399");
strcpy(metmast_TH[588].Timestamp_UTC, "29-Mar-2021 00:00:58:508");
strcpy(metmast_TH[589].Timestamp_UTC, "29-Mar-2021 00:00:58:608");
strcpy(metmast_TH[590].Timestamp_UTC, "29-Mar-2021 00:00:58:707");
strcpy(metmast_TH[591].Timestamp_UTC, "29-Mar-2021 00:00:58:806");
strcpy(metmast_TH[592].Timestamp_UTC, "29-Mar-2021 00:00:58:905");
strcpy(metmast_TH[593].Timestamp_UTC, "29-Mar-2021 00:00:59:001");
strcpy(metmast_TH[594].Timestamp_UTC, "29-Mar-2021 00:00:59:100");
strcpy(metmast_TH[595].Timestamp_UTC, "29-Mar-2021 00:00:59:199");
strcpy(metmast_TH[596].Timestamp_UTC, "29-Mar-2021 00:00:59:298");
strcpy(metmast_TH[597].Timestamp_UTC, "29-Mar-2021 00:00:59:407");
strcpy(metmast_TH[598].Timestamp_UTC, "29-Mar-2021 00:00:59:506");
strcpy(metmast_TH[599].Timestamp_UTC, "29-Mar-2021 00:00:59:605");
strcpy(metmast_TH[600].Timestamp_UTC, "29-Mar-2021 00:00:59:704");
strcpy(metmast_TH[601].Timestamp_UTC, "29-Mar-2021 00:00:59:803");
strcpy(metmast_TH[602].Timestamp_UTC, "29-Mar-2021 00:00:59:902");
strcpy(metmast_TH[603].Timestamp_UTC, "29-Mar-2021 00:01:00:001");
strcpy(metmast_TH[604].Timestamp_UTC, "29-Mar-2021 00:01:00:100");
strcpy(metmast_TH[605].Timestamp_UTC, "29-Mar-2021 00:01:00:199");
strcpy(metmast_TH[606].Timestamp_UTC, "29-Mar-2021 00:01:00:308");
strcpy(metmast_TH[607].Timestamp_UTC, "29-Mar-2021 00:01:00:406");
strcpy(metmast_TH[608].Timestamp_UTC, "29-Mar-2021 00:01:00:505");
strcpy(metmast_TH[609].Timestamp_UTC, "29-Mar-2021 00:01:00:604");
strcpy(metmast_TH[610].Timestamp_UTC, "29-Mar-2021 00:01:00:703");
strcpy(metmast_TH[611].Timestamp_UTC, "29-Mar-2021 00:01:00:802");
strcpy(metmast_TH[612].Timestamp_UTC, "29-Mar-2021 00:01:00:901");
strcpy(metmast_TH[613].Timestamp_UTC, "29-Mar-2021 00:01:01:001");
strcpy(metmast_TH[614].Timestamp_UTC, "29-Mar-2021 00:01:01:100");
strcpy(metmast_TH[615].Timestamp_UTC, "29-Mar-2021 00:01:01:209");
strcpy(metmast_TH[616].Timestamp_UTC, "29-Mar-2021 00:01:01:308");
strcpy(metmast_TH[617].Timestamp_UTC, "29-Mar-2021 00:01:01:406");
strcpy(metmast_TH[618].Timestamp_UTC, "29-Mar-2021 00:01:01:505");
strcpy(metmast_TH[619].Timestamp_UTC, "29-Mar-2021 00:01:01:604");
strcpy(metmast_TH[620].Timestamp_UTC, "29-Mar-2021 00:01:01:703");
strcpy(metmast_TH[621].Timestamp_UTC, "29-Mar-2021 00:01:01:802");
strcpy(metmast_TH[622].Timestamp_UTC, "29-Mar-2021 00:01:01:901");
strcpy(metmast_TH[623].Timestamp_UTC, "29-Mar-2021 00:01:02:001");
strcpy(metmast_TH[624].Timestamp_UTC, "29-Mar-2021 00:01:02:110");
strcpy(metmast_TH[625].Timestamp_UTC, "29-Mar-2021 00:01:02:209");
strcpy(metmast_TH[626].Timestamp_UTC, "29-Mar-2021 00:01:02:308");
strcpy(metmast_TH[627].Timestamp_UTC, "29-Mar-2021 00:01:02:406");
strcpy(metmast_TH[628].Timestamp_UTC, "29-Mar-2021 00:01:02:505");
strcpy(metmast_TH[629].Timestamp_UTC, "29-Mar-2021 00:01:02:604");
strcpy(metmast_TH[630].Timestamp_UTC, "29-Mar-2021 00:01:02:703");
strcpy(metmast_TH[631].Timestamp_UTC, "29-Mar-2021 00:01:02:802");
strcpy(metmast_TH[632].Timestamp_UTC, "29-Mar-2021 00:01:02:901");
strcpy(metmast_TH[633].Timestamp_UTC, "29-Mar-2021 00:01:03:011");
strcpy(metmast_TH[634].Timestamp_UTC, "29-Mar-2021 00:01:03:110");
strcpy(metmast_TH[635].Timestamp_UTC, "29-Mar-2021 00:01:03:209");
strcpy(metmast_TH[636].Timestamp_UTC, "29-Mar-2021 00:01:03:308");
strcpy(metmast_TH[637].Timestamp_UTC, "29-Mar-2021 00:01:03:406");
strcpy(metmast_TH[638].Timestamp_UTC, "29-Mar-2021 00:01:03:505");
strcpy(metmast_TH[639].Timestamp_UTC, "29-Mar-2021 00:01:03:604");
strcpy(metmast_TH[640].Timestamp_UTC, "29-Mar-2021 00:01:03:703");
strcpy(metmast_TH[641].Timestamp_UTC, "29-Mar-2021 00:01:03:802");
strcpy(metmast_TH[642].Timestamp_UTC, "29-Mar-2021 00:01:03:911");
strcpy(metmast_TH[643].Timestamp_UTC, "29-Mar-2021 00:01:04:011");
strcpy(metmast_TH[644].Timestamp_UTC, "29-Mar-2021 00:01:04:110");
strcpy(metmast_TH[645].Timestamp_UTC, "29-Mar-2021 00:01:04:209");
strcpy(metmast_TH[646].Timestamp_UTC, "29-Mar-2021 00:01:04:308");
strcpy(metmast_TH[647].Timestamp_UTC, "29-Mar-2021 00:01:04:406");
strcpy(metmast_TH[648].Timestamp_UTC, "29-Mar-2021 00:01:04:505");
strcpy(metmast_TH[649].Timestamp_UTC, "29-Mar-2021 00:01:04:604");
strcpy(metmast_TH[650].Timestamp_UTC, "29-Mar-2021 00:01:04:703");
strcpy(metmast_TH[651].Timestamp_UTC, "29-Mar-2021 00:01:04:812");
strcpy(metmast_TH[652].Timestamp_UTC, "29-Mar-2021 00:01:04:911");
strcpy(metmast_TH[653].Timestamp_UTC, "29-Mar-2021 00:01:05:011");
strcpy(metmast_TH[654].Timestamp_UTC, "29-Mar-2021 00:01:05:110");
strcpy(metmast_TH[655].Timestamp_UTC, "29-Mar-2021 00:01:05:209");
strcpy(metmast_TH[656].Timestamp_UTC, "29-Mar-2021 00:01:05:308");
strcpy(metmast_TH[657].Timestamp_UTC, "29-Mar-2021 00:01:05:406");
strcpy(metmast_TH[658].Timestamp_UTC, "29-Mar-2021 00:01:05:505");
strcpy(metmast_TH[659].Timestamp_UTC, "29-Mar-2021 00:01:05:604");
strcpy(metmast_TH[660].Timestamp_UTC, "29-Mar-2021 00:01:05:713");
strcpy(metmast_TH[661].Timestamp_UTC, "29-Mar-2021 00:01:05:812");
strcpy(metmast_TH[662].Timestamp_UTC, "29-Mar-2021 00:01:05:911");
strcpy(metmast_TH[663].Timestamp_UTC, "29-Mar-2021 00:01:06:011");
strcpy(metmast_TH[664].Timestamp_UTC, "29-Mar-2021 00:01:06:110");
strcpy(metmast_TH[665].Timestamp_UTC, "29-Mar-2021 00:01:06:209");
strcpy(metmast_TH[666].Timestamp_UTC, "29-Mar-2021 00:01:06:308");
strcpy(metmast_TH[667].Timestamp_UTC, "29-Mar-2021 00:01:06:406");
strcpy(metmast_TH[668].Timestamp_UTC, "29-Mar-2021 00:01:06:505");
strcpy(metmast_TH[669].Timestamp_UTC, "29-Mar-2021 00:01:06:614");
strcpy(metmast_TH[670].Timestamp_UTC, "29-Mar-2021 00:01:06:713");
strcpy(metmast_TH[671].Timestamp_UTC, "29-Mar-2021 00:01:06:812");
strcpy(metmast_TH[672].Timestamp_UTC, "29-Mar-2021 00:01:06:911");
strcpy(metmast_TH[673].Timestamp_UTC, "29-Mar-2021 00:01:07:011");
strcpy(metmast_TH[674].Timestamp_UTC, "29-Mar-2021 00:01:07:110");
strcpy(metmast_TH[675].Timestamp_UTC, "29-Mar-2021 00:01:07:209");
strcpy(metmast_TH[676].Timestamp_UTC, "29-Mar-2021 00:01:07:308");
strcpy(metmast_TH[677].Timestamp_UTC, "29-Mar-2021 00:01:07:407");
strcpy(metmast_TH[678].Timestamp_UTC, "29-Mar-2021 00:01:07:515");
strcpy(metmast_TH[679].Timestamp_UTC, "29-Mar-2021 00:01:07:614");
strcpy(metmast_TH[680].Timestamp_UTC, "29-Mar-2021 00:01:07:713");
strcpy(metmast_TH[681].Timestamp_UTC, "29-Mar-2021 00:01:07:812");
strcpy(metmast_TH[682].Timestamp_UTC, "29-Mar-2021 00:01:07:911");
strcpy(metmast_TH[683].Timestamp_UTC, "29-Mar-2021 00:01:08:011");
strcpy(metmast_TH[684].Timestamp_UTC, "29-Mar-2021 00:01:08:110");
strcpy(metmast_TH[685].Timestamp_UTC, "29-Mar-2021 00:01:08:209");
strcpy(metmast_TH[686].Timestamp_UTC, "29-Mar-2021 00:01:08:307");
strcpy(metmast_TH[687].Timestamp_UTC, "29-Mar-2021 00:01:08:406");
strcpy(metmast_TH[688].Timestamp_UTC, "29-Mar-2021 00:01:08:515");
strcpy(metmast_TH[689].Timestamp_UTC, "29-Mar-2021 00:01:08:614");
strcpy(metmast_TH[690].Timestamp_UTC, "29-Mar-2021 00:01:08:713");
strcpy(metmast_TH[691].Timestamp_UTC, "29-Mar-2021 00:01:08:812");
strcpy(metmast_TH[692].Timestamp_UTC, "29-Mar-2021 00:01:08:911");
strcpy(metmast_TH[693].Timestamp_UTC, "29-Mar-2021 00:01:09:001");
strcpy(metmast_TH[694].Timestamp_UTC, "29-Mar-2021 00:01:09:100");
strcpy(metmast_TH[695].Timestamp_UTC, "29-Mar-2021 00:01:09:199");
strcpy(metmast_TH[696].Timestamp_UTC, "29-Mar-2021 00:01:09:298");
strcpy(metmast_TH[697].Timestamp_UTC, "29-Mar-2021 00:01:09:406");
strcpy(metmast_TH[698].Timestamp_UTC, "29-Mar-2021 00:01:09:505");
strcpy(metmast_TH[699].Timestamp_UTC, "29-Mar-2021 00:01:09:604");
strcpy(metmast_TH[700].Timestamp_UTC, "29-Mar-2021 00:01:09:703");
strcpy(metmast_TH[701].Timestamp_UTC, "29-Mar-2021 00:01:09:802");
strcpy(metmast_TH[702].Timestamp_UTC, "29-Mar-2021 00:01:09:901");
strcpy(metmast_TH[703].Timestamp_UTC, "29-Mar-2021 00:01:10:001");
strcpy(metmast_TH[704].Timestamp_UTC, "29-Mar-2021 00:01:10:100");
strcpy(metmast_TH[705].Timestamp_UTC, "29-Mar-2021 00:01:10:199");
strcpy(metmast_TH[706].Timestamp_UTC, "29-Mar-2021 00:01:10:317");
strcpy(metmast_TH[707].Timestamp_UTC, "29-Mar-2021 00:01:10:407");
strcpy(metmast_TH[708].Timestamp_UTC, "29-Mar-2021 00:01:10:505");
strcpy(metmast_TH[709].Timestamp_UTC, "29-Mar-2021 00:01:10:604");
strcpy(metmast_TH[710].Timestamp_UTC, "29-Mar-2021 00:01:10:703");
strcpy(metmast_TH[711].Timestamp_UTC, "29-Mar-2021 00:01:10:802");
strcpy(metmast_TH[712].Timestamp_UTC, "29-Mar-2021 00:01:10:901");
strcpy(metmast_TH[713].Timestamp_UTC, "29-Mar-2021 00:01:11:001");
strcpy(metmast_TH[714].Timestamp_UTC, "29-Mar-2021 00:01:11:100");
strcpy(metmast_TH[715].Timestamp_UTC, "29-Mar-2021 00:01:11:209");
strcpy(metmast_TH[716].Timestamp_UTC, "29-Mar-2021 00:01:11:308");
strcpy(metmast_TH[717].Timestamp_UTC, "29-Mar-2021 00:01:11:406");
strcpy(metmast_TH[718].Timestamp_UTC, "29-Mar-2021 00:01:11:505");
strcpy(metmast_TH[719].Timestamp_UTC, "29-Mar-2021 00:01:11:604");
strcpy(metmast_TH[720].Timestamp_UTC, "29-Mar-2021 00:01:11:703");
strcpy(metmast_TH[721].Timestamp_UTC, "29-Mar-2021 00:01:11:802");
strcpy(metmast_TH[722].Timestamp_UTC, "29-Mar-2021 00:01:11:901");
strcpy(metmast_TH[723].Timestamp_UTC, "29-Mar-2021 00:01:12:001");
strcpy(metmast_TH[724].Timestamp_UTC, "29-Mar-2021 00:01:12:110");
strcpy(metmast_TH[725].Timestamp_UTC, "29-Mar-2021 00:01:12:209");
strcpy(metmast_TH[726].Timestamp_UTC, "29-Mar-2021 00:01:12:308");
strcpy(metmast_TH[727].Timestamp_UTC, "29-Mar-2021 00:01:12:406");
strcpy(metmast_TH[728].Timestamp_UTC, "29-Mar-2021 00:01:12:505");
strcpy(metmast_TH[729].Timestamp_UTC, "29-Mar-2021 00:01:12:604");
strcpy(metmast_TH[730].Timestamp_UTC, "29-Mar-2021 00:01:12:703");
strcpy(metmast_TH[731].Timestamp_UTC, "29-Mar-2021 00:01:12:802");
strcpy(metmast_TH[732].Timestamp_UTC, "29-Mar-2021 00:01:12:901");
strcpy(metmast_TH[733].Timestamp_UTC, "29-Mar-2021 00:01:13:011");
strcpy(metmast_TH[734].Timestamp_UTC, "29-Mar-2021 00:01:13:110");
strcpy(metmast_TH[735].Timestamp_UTC, "29-Mar-2021 00:01:13:209");
strcpy(metmast_TH[736].Timestamp_UTC, "29-Mar-2021 00:01:13:308");
strcpy(metmast_TH[737].Timestamp_UTC, "29-Mar-2021 00:01:13:406");
strcpy(metmast_TH[738].Timestamp_UTC, "29-Mar-2021 00:01:13:505");
strcpy(metmast_TH[739].Timestamp_UTC, "29-Mar-2021 00:01:13:604");
strcpy(metmast_TH[740].Timestamp_UTC, "29-Mar-2021 00:01:13:703");
strcpy(metmast_TH[741].Timestamp_UTC, "29-Mar-2021 00:01:13:802");
strcpy(metmast_TH[742].Timestamp_UTC, "29-Mar-2021 00:01:13:911");
strcpy(metmast_TH[743].Timestamp_UTC, "29-Mar-2021 00:01:14:011");
strcpy(metmast_TH[744].Timestamp_UTC, "29-Mar-2021 00:01:14:110");
strcpy(metmast_TH[745].Timestamp_UTC, "29-Mar-2021 00:01:14:209");
strcpy(metmast_TH[746].Timestamp_UTC, "29-Mar-2021 00:01:14:308");
strcpy(metmast_TH[747].Timestamp_UTC, "29-Mar-2021 00:01:14:406");
strcpy(metmast_TH[748].Timestamp_UTC, "29-Mar-2021 00:01:14:505");
strcpy(metmast_TH[749].Timestamp_UTC, "29-Mar-2021 00:01:14:604");
strcpy(metmast_TH[750].Timestamp_UTC, "29-Mar-2021 00:01:14:703");
strcpy(metmast_TH[751].Timestamp_UTC, "29-Mar-2021 00:01:14:812");
strcpy(metmast_TH[752].Timestamp_UTC, "29-Mar-2021 00:01:14:911");
strcpy(metmast_TH[753].Timestamp_UTC, "29-Mar-2021 00:01:15:011");
strcpy(metmast_TH[754].Timestamp_UTC, "29-Mar-2021 00:01:15:110");
strcpy(metmast_TH[755].Timestamp_UTC, "29-Mar-2021 00:01:15:209");
strcpy(metmast_TH[756].Timestamp_UTC, "29-Mar-2021 00:01:15:308");
strcpy(metmast_TH[757].Timestamp_UTC, "29-Mar-2021 00:01:15:407");
strcpy(metmast_TH[758].Timestamp_UTC, "29-Mar-2021 00:01:15:505");
strcpy(metmast_TH[759].Timestamp_UTC, "29-Mar-2021 00:01:15:604");
strcpy(metmast_TH[760].Timestamp_UTC, "29-Mar-2021 00:01:15:713");
strcpy(metmast_TH[761].Timestamp_UTC, "29-Mar-2021 00:01:15:812");
strcpy(metmast_TH[762].Timestamp_UTC, "29-Mar-2021 00:01:15:911");
strcpy(metmast_TH[763].Timestamp_UTC, "29-Mar-2021 00:01:16:011");
strcpy(metmast_TH[764].Timestamp_UTC, "29-Mar-2021 00:01:16:110");
strcpy(metmast_TH[765].Timestamp_UTC, "29-Mar-2021 00:01:16:209");
strcpy(metmast_TH[766].Timestamp_UTC, "29-Mar-2021 00:01:16:308");
strcpy(metmast_TH[767].Timestamp_UTC, "29-Mar-2021 00:01:16:406");
strcpy(metmast_TH[768].Timestamp_UTC, "29-Mar-2021 00:01:16:505");
strcpy(metmast_TH[769].Timestamp_UTC, "29-Mar-2021 00:01:16:614");
strcpy(metmast_TH[770].Timestamp_UTC, "29-Mar-2021 00:01:16:713");
strcpy(metmast_TH[771].Timestamp_UTC, "29-Mar-2021 00:01:16:812");
strcpy(metmast_TH[772].Timestamp_UTC, "29-Mar-2021 00:01:16:911");
strcpy(metmast_TH[773].Timestamp_UTC, "29-Mar-2021 00:01:17:011");
strcpy(metmast_TH[774].Timestamp_UTC, "29-Mar-2021 00:01:17:110");
strcpy(metmast_TH[775].Timestamp_UTC, "29-Mar-2021 00:01:17:209");
strcpy(metmast_TH[776].Timestamp_UTC, "29-Mar-2021 00:01:17:307");
strcpy(metmast_TH[777].Timestamp_UTC, "29-Mar-2021 00:01:17:406");
strcpy(metmast_TH[778].Timestamp_UTC, "29-Mar-2021 00:01:17:515");
strcpy(metmast_TH[779].Timestamp_UTC, "29-Mar-2021 00:01:17:614");
strcpy(metmast_TH[780].Timestamp_UTC, "29-Mar-2021 00:01:17:713");
strcpy(metmast_TH[781].Timestamp_UTC, "29-Mar-2021 00:01:17:812");
strcpy(metmast_TH[782].Timestamp_UTC, "29-Mar-2021 00:01:17:911");
strcpy(metmast_TH[783].Timestamp_UTC, "29-Mar-2021 00:01:18:001");
strcpy(metmast_TH[784].Timestamp_UTC, "29-Mar-2021 00:01:18:100");
strcpy(metmast_TH[785].Timestamp_UTC, "29-Mar-2021 00:01:18:199");
strcpy(metmast_TH[786].Timestamp_UTC, "29-Mar-2021 00:01:18:298");
strcpy(metmast_TH[787].Timestamp_UTC, "29-Mar-2021 00:01:18:397");
strcpy(metmast_TH[788].Timestamp_UTC, "29-Mar-2021 00:01:18:505");
strcpy(metmast_TH[789].Timestamp_UTC, "29-Mar-2021 00:01:18:604");
strcpy(metmast_TH[790].Timestamp_UTC, "29-Mar-2021 00:01:18:703");
strcpy(metmast_TH[791].Timestamp_UTC, "29-Mar-2021 00:01:18:802");
strcpy(metmast_TH[792].Timestamp_UTC, "29-Mar-2021 00:01:18:901");
strcpy(metmast_TH[793].Timestamp_UTC, "29-Mar-2021 00:01:19:001");
strcpy(metmast_TH[794].Timestamp_UTC, "29-Mar-2021 00:01:19:100");
strcpy(metmast_TH[795].Timestamp_UTC, "29-Mar-2021 00:01:19:199");
strcpy(metmast_TH[796].Timestamp_UTC, "29-Mar-2021 00:01:19:298");
strcpy(metmast_TH[797].Timestamp_UTC, "29-Mar-2021 00:01:19:406");
strcpy(metmast_TH[798].Timestamp_UTC, "29-Mar-2021 00:01:19:505");
strcpy(metmast_TH[799].Timestamp_UTC, "29-Mar-2021 00:01:19:604");
strcpy(metmast_TH[800].Timestamp_UTC, "29-Mar-2021 00:01:19:703");
strcpy(metmast_TH[801].Timestamp_UTC, "29-Mar-2021 00:01:19:802");
strcpy(metmast_TH[802].Timestamp_UTC, "29-Mar-2021 00:01:19:921");
strcpy(metmast_TH[803].Timestamp_UTC, "29-Mar-2021 00:01:20:001");
strcpy(metmast_TH[804].Timestamp_UTC, "29-Mar-2021 00:01:20:100");
strcpy(metmast_TH[805].Timestamp_UTC, "29-Mar-2021 00:01:20:199");
strcpy(metmast_TH[806].Timestamp_UTC, "29-Mar-2021 00:01:20:317");
strcpy(metmast_TH[807].Timestamp_UTC, "29-Mar-2021 00:01:20:406");
strcpy(metmast_TH[808].Timestamp_UTC, "29-Mar-2021 00:01:20:505");
strcpy(metmast_TH[809].Timestamp_UTC, "29-Mar-2021 00:01:20:604");
strcpy(metmast_TH[810].Timestamp_UTC, "29-Mar-2021 00:01:20:703");
strcpy(metmast_TH[811].Timestamp_UTC, "29-Mar-2021 00:01:20:802");
strcpy(metmast_TH[812].Timestamp_UTC, "29-Mar-2021 00:01:20:901");
strcpy(metmast_TH[813].Timestamp_UTC, "29-Mar-2021 00:01:21:001");
strcpy(metmast_TH[814].Timestamp_UTC, "29-Mar-2021 00:01:21:100");
strcpy(metmast_TH[815].Timestamp_UTC, "29-Mar-2021 00:01:21:209");
strcpy(metmast_TH[816].Timestamp_UTC, "29-Mar-2021 00:01:21:308");
strcpy(metmast_TH[817].Timestamp_UTC, "29-Mar-2021 00:01:21:406");
strcpy(metmast_TH[818].Timestamp_UTC, "29-Mar-2021 00:01:21:505");
strcpy(metmast_TH[819].Timestamp_UTC, "29-Mar-2021 00:01:21:604");
strcpy(metmast_TH[820].Timestamp_UTC, "29-Mar-2021 00:01:21:703");
strcpy(metmast_TH[821].Timestamp_UTC, "29-Mar-2021 00:01:21:802");
strcpy(metmast_TH[822].Timestamp_UTC, "29-Mar-2021 00:01:21:901");
strcpy(metmast_TH[823].Timestamp_UTC, "29-Mar-2021 00:01:22:001");
strcpy(metmast_TH[824].Timestamp_UTC, "29-Mar-2021 00:01:22:110");
strcpy(metmast_TH[825].Timestamp_UTC, "29-Mar-2021 00:01:22:209");
strcpy(metmast_TH[826].Timestamp_UTC, "29-Mar-2021 00:01:22:308");
strcpy(metmast_TH[827].Timestamp_UTC, "29-Mar-2021 00:01:22:406");
strcpy(metmast_TH[828].Timestamp_UTC, "29-Mar-2021 00:01:22:505");
strcpy(metmast_TH[829].Timestamp_UTC, "29-Mar-2021 00:01:22:604");
strcpy(metmast_TH[830].Timestamp_UTC, "29-Mar-2021 00:01:22:703");
strcpy(metmast_TH[831].Timestamp_UTC, "29-Mar-2021 00:01:22:802");
strcpy(metmast_TH[832].Timestamp_UTC, "29-Mar-2021 00:01:22:901");
strcpy(metmast_TH[833].Timestamp_UTC, "29-Mar-2021 00:01:23:011");
strcpy(metmast_TH[834].Timestamp_UTC, "29-Mar-2021 00:01:23:110");
strcpy(metmast_TH[835].Timestamp_UTC, "29-Mar-2021 00:01:23:209");
strcpy(metmast_TH[836].Timestamp_UTC, "29-Mar-2021 00:01:23:308");
strcpy(metmast_TH[837].Timestamp_UTC, "29-Mar-2021 00:01:23:406");
strcpy(metmast_TH[838].Timestamp_UTC, "29-Mar-2021 00:01:23:505");
strcpy(metmast_TH[839].Timestamp_UTC, "29-Mar-2021 00:01:23:604");
strcpy(metmast_TH[840].Timestamp_UTC, "29-Mar-2021 00:01:23:703");
strcpy(metmast_TH[841].Timestamp_UTC, "29-Mar-2021 00:01:23:802");
strcpy(metmast_TH[842].Timestamp_UTC, "29-Mar-2021 00:01:23:911");
strcpy(metmast_TH[843].Timestamp_UTC, "29-Mar-2021 00:01:24:011");
strcpy(metmast_TH[844].Timestamp_UTC, "29-Mar-2021 00:01:24:110");
strcpy(metmast_TH[845].Timestamp_UTC, "29-Mar-2021 00:01:24:209");
strcpy(metmast_TH[846].Timestamp_UTC, "29-Mar-2021 00:01:24:308");
strcpy(metmast_TH[847].Timestamp_UTC, "29-Mar-2021 00:01:24:406");
strcpy(metmast_TH[848].Timestamp_UTC, "29-Mar-2021 00:01:24:505");
strcpy(metmast_TH[849].Timestamp_UTC, "29-Mar-2021 00:01:24:604");
strcpy(metmast_TH[850].Timestamp_UTC, "29-Mar-2021 00:01:24:703");
strcpy(metmast_TH[851].Timestamp_UTC, "29-Mar-2021 00:01:24:812");
strcpy(metmast_TH[852].Timestamp_UTC, "29-Mar-2021 00:01:24:911");
strcpy(metmast_TH[853].Timestamp_UTC, "29-Mar-2021 00:01:25:011");
strcpy(metmast_TH[854].Timestamp_UTC, "29-Mar-2021 00:01:25:110");
strcpy(metmast_TH[855].Timestamp_UTC, "29-Mar-2021 00:01:25:209");
strcpy(metmast_TH[856].Timestamp_UTC, "29-Mar-2021 00:01:25:308");
strcpy(metmast_TH[857].Timestamp_UTC, "29-Mar-2021 00:01:25:406");
strcpy(metmast_TH[858].Timestamp_UTC, "29-Mar-2021 00:01:25:505");
strcpy(metmast_TH[859].Timestamp_UTC, "29-Mar-2021 00:01:25:604");
strcpy(metmast_TH[860].Timestamp_UTC, "29-Mar-2021 00:01:25:713");
strcpy(metmast_TH[861].Timestamp_UTC, "29-Mar-2021 00:01:25:812");
strcpy(metmast_TH[862].Timestamp_UTC, "29-Mar-2021 00:01:25:911");
strcpy(metmast_TH[863].Timestamp_UTC, "29-Mar-2021 00:01:26:011");
strcpy(metmast_TH[864].Timestamp_UTC, "29-Mar-2021 00:01:26:110");
strcpy(metmast_TH[865].Timestamp_UTC, "29-Mar-2021 00:01:26:209");
strcpy(metmast_TH[866].Timestamp_UTC, "29-Mar-2021 00:01:26:308");
strcpy(metmast_TH[867].Timestamp_UTC, "29-Mar-2021 00:01:26:407");
strcpy(metmast_TH[868].Timestamp_UTC, "29-Mar-2021 00:01:26:505");
strcpy(metmast_TH[869].Timestamp_UTC, "29-Mar-2021 00:01:26:614");
strcpy(metmast_TH[870].Timestamp_UTC, "29-Mar-2021 00:01:26:713");
strcpy(metmast_TH[871].Timestamp_UTC, "29-Mar-2021 00:01:26:812");
strcpy(metmast_TH[872].Timestamp_UTC, "29-Mar-2021 00:01:26:911");
strcpy(metmast_TH[873].Timestamp_UTC, "29-Mar-2021 00:01:27:001");
strcpy(metmast_TH[874].Timestamp_UTC, "29-Mar-2021 00:01:27:100");
strcpy(metmast_TH[875].Timestamp_UTC, "29-Mar-2021 00:01:27:199");
strcpy(metmast_TH[876].Timestamp_UTC, "29-Mar-2021 00:01:27:298");
strcpy(metmast_TH[877].Timestamp_UTC, "29-Mar-2021 00:01:27:397");
strcpy(metmast_TH[878].Timestamp_UTC, "29-Mar-2021 00:01:27:505");
strcpy(metmast_TH[879].Timestamp_UTC, "29-Mar-2021 00:01:27:604");
strcpy(metmast_TH[880].Timestamp_UTC, "29-Mar-2021 00:01:27:703");
strcpy(metmast_TH[881].Timestamp_UTC, "29-Mar-2021 00:01:27:802");
strcpy(metmast_TH[882].Timestamp_UTC, "29-Mar-2021 00:01:27:901");
strcpy(metmast_TH[883].Timestamp_UTC, "29-Mar-2021 00:01:28:001");
strcpy(metmast_TH[884].Timestamp_UTC, "29-Mar-2021 00:01:28:100");
strcpy(metmast_TH[885].Timestamp_UTC, "29-Mar-2021 00:01:28:199");
strcpy(metmast_TH[886].Timestamp_UTC, "29-Mar-2021 00:01:28:298");
strcpy(metmast_TH[887].Timestamp_UTC, "29-Mar-2021 00:01:28:397");
strcpy(metmast_TH[888].Timestamp_UTC, "29-Mar-2021 00:01:28:505");
strcpy(metmast_TH[889].Timestamp_UTC, "29-Mar-2021 00:01:28:604");
strcpy(metmast_TH[890].Timestamp_UTC, "29-Mar-2021 00:01:28:703");
strcpy(metmast_TH[891].Timestamp_UTC, "29-Mar-2021 00:01:28:802");
strcpy(metmast_TH[892].Timestamp_UTC, "29-Mar-2021 00:01:28:901");
strcpy(metmast_TH[893].Timestamp_UTC, "29-Mar-2021 00:01:29:001");
strcpy(metmast_TH[894].Timestamp_UTC, "29-Mar-2021 00:01:29:100");
strcpy(metmast_TH[895].Timestamp_UTC, "29-Mar-2021 00:01:29:199");
strcpy(metmast_TH[896].Timestamp_UTC, "29-Mar-2021 00:01:29:298");
strcpy(metmast_TH[897].Timestamp_UTC, "29-Mar-2021 00:01:29:406");
strcpy(metmast_TH[898].Timestamp_UTC, "29-Mar-2021 00:01:29:505");
strcpy(metmast_TH[899].Timestamp_UTC, "29-Mar-2021 00:01:29:604");
strcpy(metmast_TH[900].Timestamp_UTC, "29-Mar-2021 00:01:29:703");
strcpy(metmast_TH[901].Timestamp_UTC, "29-Mar-2021 00:01:29:802");
strcpy(metmast_TH[902].Timestamp_UTC, "29-Mar-2021 00:01:29:901");
strcpy(metmast_TH[903].Timestamp_UTC, "29-Mar-2021 00:01:30:001");
strcpy(metmast_TH[904].Timestamp_UTC, "29-Mar-2021 00:01:30:100");
strcpy(metmast_TH[905].Timestamp_UTC, "29-Mar-2021 00:01:30:199");
strcpy(metmast_TH[906].Timestamp_UTC, "29-Mar-2021 00:01:30:308");
strcpy(metmast_TH[907].Timestamp_UTC, "29-Mar-2021 00:01:30:406");
strcpy(metmast_TH[908].Timestamp_UTC, "29-Mar-2021 00:01:30:505");
strcpy(metmast_TH[909].Timestamp_UTC, "29-Mar-2021 00:01:30:604");
strcpy(metmast_TH[910].Timestamp_UTC, "29-Mar-2021 00:01:30:703");
strcpy(metmast_TH[911].Timestamp_UTC, "29-Mar-2021 00:01:30:802");
strcpy(metmast_TH[912].Timestamp_UTC, "29-Mar-2021 00:01:30:901");
strcpy(metmast_TH[913].Timestamp_UTC, "29-Mar-2021 00:01:31:001");
strcpy(metmast_TH[914].Timestamp_UTC, "29-Mar-2021 00:01:31:100");
strcpy(metmast_TH[915].Timestamp_UTC, "29-Mar-2021 00:01:31:209");
strcpy(metmast_TH[916].Timestamp_UTC, "29-Mar-2021 00:01:31:308");
strcpy(metmast_TH[917].Timestamp_UTC, "29-Mar-2021 00:01:31:407");
strcpy(metmast_TH[918].Timestamp_UTC, "29-Mar-2021 00:01:31:505");
strcpy(metmast_TH[919].Timestamp_UTC, "29-Mar-2021 00:01:31:604");
strcpy(metmast_TH[920].Timestamp_UTC, "29-Mar-2021 00:01:31:703");
strcpy(metmast_TH[921].Timestamp_UTC, "29-Mar-2021 00:01:31:802");
strcpy(metmast_TH[922].Timestamp_UTC, "29-Mar-2021 00:01:31:901");
strcpy(metmast_TH[923].Timestamp_UTC, "29-Mar-2021 00:01:32:001");
strcpy(metmast_TH[924].Timestamp_UTC, "29-Mar-2021 00:01:32:110");
strcpy(metmast_TH[925].Timestamp_UTC, "29-Mar-2021 00:01:32:209");
strcpy(metmast_TH[926].Timestamp_UTC, "29-Mar-2021 00:01:32:308");
strcpy(metmast_TH[927].Timestamp_UTC, "29-Mar-2021 00:01:32:406");
strcpy(metmast_TH[928].Timestamp_UTC, "29-Mar-2021 00:01:32:505");
strcpy(metmast_TH[929].Timestamp_UTC, "29-Mar-2021 00:01:32:604");
strcpy(metmast_TH[930].Timestamp_UTC, "29-Mar-2021 00:01:32:703");
strcpy(metmast_TH[931].Timestamp_UTC, "29-Mar-2021 00:01:32:812");
strcpy(metmast_TH[932].Timestamp_UTC, "29-Mar-2021 00:01:32:901");
strcpy(metmast_TH[933].Timestamp_UTC, "29-Mar-2021 00:01:33:011");
strcpy(metmast_TH[934].Timestamp_UTC, "29-Mar-2021 00:01:33:110");
strcpy(metmast_TH[935].Timestamp_UTC, "29-Mar-2021 00:01:33:209");
strcpy(metmast_TH[936].Timestamp_UTC, "29-Mar-2021 00:01:33:308");
strcpy(metmast_TH[937].Timestamp_UTC, "29-Mar-2021 00:01:33:406");
strcpy(metmast_TH[938].Timestamp_UTC, "29-Mar-2021 00:01:33:505");
strcpy(metmast_TH[939].Timestamp_UTC, "29-Mar-2021 00:01:33:604");
strcpy(metmast_TH[940].Timestamp_UTC, "29-Mar-2021 00:01:33:703");
strcpy(metmast_TH[941].Timestamp_UTC, "29-Mar-2021 00:01:33:802");
strcpy(metmast_TH[942].Timestamp_UTC, "29-Mar-2021 00:01:33:911");
strcpy(metmast_TH[943].Timestamp_UTC, "29-Mar-2021 00:01:34:011");
strcpy(metmast_TH[944].Timestamp_UTC, "29-Mar-2021 00:01:34:110");
strcpy(metmast_TH[945].Timestamp_UTC, "29-Mar-2021 00:01:34:209");
strcpy(metmast_TH[946].Timestamp_UTC, "29-Mar-2021 00:01:34:308");
strcpy(metmast_TH[947].Timestamp_UTC, "29-Mar-2021 00:01:34:406");
strcpy(metmast_TH[948].Timestamp_UTC, "29-Mar-2021 00:01:34:505");
strcpy(metmast_TH[949].Timestamp_UTC, "29-Mar-2021 00:01:34:604");
strcpy(metmast_TH[950].Timestamp_UTC, "29-Mar-2021 00:01:34:703");
strcpy(metmast_TH[951].Timestamp_UTC, "29-Mar-2021 00:01:34:812");
strcpy(metmast_TH[952].Timestamp_UTC, "29-Mar-2021 00:01:34:911");
strcpy(metmast_TH[953].Timestamp_UTC, "29-Mar-2021 00:01:35:011");
strcpy(metmast_TH[954].Timestamp_UTC, "29-Mar-2021 00:01:35:110");
strcpy(metmast_TH[955].Timestamp_UTC, "29-Mar-2021 00:01:35:209");
strcpy(metmast_TH[956].Timestamp_UTC, "29-Mar-2021 00:01:35:308");
strcpy(metmast_TH[957].Timestamp_UTC, "29-Mar-2021 00:01:35:407");
strcpy(metmast_TH[958].Timestamp_UTC, "29-Mar-2021 00:01:35:505");
strcpy(metmast_TH[959].Timestamp_UTC, "29-Mar-2021 00:01:35:604");
strcpy(metmast_TH[960].Timestamp_UTC, "29-Mar-2021 00:01:35:713");
strcpy(metmast_TH[961].Timestamp_UTC, "29-Mar-2021 00:01:35:812");
strcpy(metmast_TH[962].Timestamp_UTC, "29-Mar-2021 00:01:35:911");
strcpy(metmast_TH[963].Timestamp_UTC, "29-Mar-2021 00:01:36:001");
strcpy(metmast_TH[964].Timestamp_UTC, "29-Mar-2021 00:01:36:100");
strcpy(metmast_TH[965].Timestamp_UTC, "29-Mar-2021 00:01:36:199");
strcpy(metmast_TH[966].Timestamp_UTC, "29-Mar-2021 00:01:36:298");
strcpy(metmast_TH[967].Timestamp_UTC, "29-Mar-2021 00:01:36:397");
strcpy(metmast_TH[968].Timestamp_UTC, "29-Mar-2021 00:01:36:495");
strcpy(metmast_TH[969].Timestamp_UTC, "29-Mar-2021 00:01:36:604");
strcpy(metmast_TH[970].Timestamp_UTC, "29-Mar-2021 00:01:36:703");
strcpy(metmast_TH[971].Timestamp_UTC, "29-Mar-2021 00:01:36:802");
strcpy(metmast_TH[972].Timestamp_UTC, "29-Mar-2021 00:01:36:901");
strcpy(metmast_TH[973].Timestamp_UTC, "29-Mar-2021 00:01:37:001");
strcpy(metmast_TH[974].Timestamp_UTC, "29-Mar-2021 00:01:37:100");
strcpy(metmast_TH[975].Timestamp_UTC, "29-Mar-2021 00:01:37:199");
strcpy(metmast_TH[976].Timestamp_UTC, "29-Mar-2021 00:01:37:298");
strcpy(metmast_TH[977].Timestamp_UTC, "29-Mar-2021 00:01:37:397");
strcpy(metmast_TH[978].Timestamp_UTC, "29-Mar-2021 00:01:37:505");
strcpy(metmast_TH[979].Timestamp_UTC, "29-Mar-2021 00:01:37:604");
strcpy(metmast_TH[980].Timestamp_UTC, "29-Mar-2021 00:01:37:703");
strcpy(metmast_TH[981].Timestamp_UTC, "29-Mar-2021 00:01:37:802");
strcpy(metmast_TH[982].Timestamp_UTC, "29-Mar-2021 00:01:37:901");
strcpy(metmast_TH[983].Timestamp_UTC, "29-Mar-2021 00:01:38:001");
strcpy(metmast_TH[984].Timestamp_UTC, "29-Mar-2021 00:01:38:100");
strcpy(metmast_TH[985].Timestamp_UTC, "29-Mar-2021 00:01:38:199");
strcpy(metmast_TH[986].Timestamp_UTC, "29-Mar-2021 00:01:38:298");
strcpy(metmast_TH[987].Timestamp_UTC, "29-Mar-2021 00:01:38:397");
strcpy(metmast_TH[988].Timestamp_UTC, "29-Mar-2021 00:01:38:505");
strcpy(metmast_TH[989].Timestamp_UTC, "29-Mar-2021 00:01:38:604");
strcpy(metmast_TH[990].Timestamp_UTC, "29-Mar-2021 00:01:38:703");
strcpy(metmast_TH[991].Timestamp_UTC, "29-Mar-2021 00:01:38:802");
strcpy(metmast_TH[992].Timestamp_UTC, "29-Mar-2021 00:01:38:901");
strcpy(metmast_TH[993].Timestamp_UTC, "29-Mar-2021 00:01:39:001");
strcpy(metmast_TH[994].Timestamp_UTC, "29-Mar-2021 00:01:39:100");
strcpy(metmast_TH[995].Timestamp_UTC, "29-Mar-2021 00:01:39:199");
strcpy(metmast_TH[996].Timestamp_UTC, "29-Mar-2021 00:01:39:298");
strcpy(metmast_TH[997].Timestamp_UTC, "29-Mar-2021 00:01:39:406");
strcpy(metmast_TH[998].Timestamp_UTC, "29-Mar-2021 00:01:39:505");
strcpy(metmast_TH[999].Timestamp_UTC, "29-Mar-2021 00:01:39:604");
metmast_TH[0].ws_110m[0]=11.4509;
metmast_TH[1].ws_110m[0]=11.3743;
metmast_TH[2].ws_110m[0]=11.4536;
metmast_TH[3].ws_110m[0]=11.5165;
metmast_TH[4].ws_110m[0]=11.3743;
metmast_TH[5].ws_110m[0]=11.4454;
metmast_TH[6].ws_110m[0]=11.9267;
metmast_TH[7].ws_110m[0]=11.6013;
metmast_TH[8].ws_110m[0]=11.6724;
metmast_TH[9].ws_110m[0]=11.3415;
metmast_TH[10].ws_110m[0]=11.4892;
metmast_TH[11].ws_110m[0]=11.6614;
metmast_TH[12].ws_110m[0]=11.7599;
metmast_TH[13].ws_110m[0]=11.8064;
metmast_TH[14].ws_110m[0]=11.8829;
metmast_TH[15].ws_110m[0]=11.9157;
metmast_TH[16].ws_110m[0]=12.3478;
metmast_TH[17].ws_110m[0]=12.3259;
metmast_TH[18].ws_110m[0]=12.3505;
metmast_TH[19].ws_110m[0]=12.2958;
metmast_TH[20].ws_110m[0]=11.965;
metmast_TH[21].ws_110m[0]=12.1646;
metmast_TH[22].ws_110m[0]=12.3478;
metmast_TH[23].ws_110m[0]=12.3587;
metmast_TH[24].ws_110m[0]=12.006;
metmast_TH[25].ws_110m[0]=12.1728;
metmast_TH[26].ws_110m[0]=12.0005;
metmast_TH[27].ws_110m[0]=12.0169;
metmast_TH[28].ws_110m[0]=12.6759;
metmast_TH[29].ws_110m[0]=12.0962;
metmast_TH[30].ws_110m[0]=12.5036;
metmast_TH[31].ws_110m[0]=12.561;
metmast_TH[32].ws_110m[0]=12.8755;
metmast_TH[33].ws_110m[0]=12.8536;
metmast_TH[34].ws_110m[0]=12.9411;
metmast_TH[35].ws_110m[0]=12.9821;
metmast_TH[36].ws_110m[0]=13.0177;
metmast_TH[37].ws_110m[0]=12.788;
metmast_TH[38].ws_110m[0]=12.6704;
metmast_TH[39].ws_110m[0]=12.6622;
metmast_TH[40].ws_110m[0]=12.4954;
metmast_TH[41].ws_110m[0]=12.1071;
metmast_TH[42].ws_110m[0]=11.4536;
metmast_TH[43].ws_110m[0]=11.6423;
metmast_TH[44].ws_110m[0]=11.5439;
metmast_TH[45].ws_110m[0]=11.6587;
metmast_TH[46].ws_110m[0]=11.8255;
metmast_TH[47].ws_110m[0]=11.8939;
metmast_TH[48].ws_110m[0]=11.5384;
metmast_TH[49].ws_110m[0]=11.3935;
metmast_TH[50].ws_110m[0]=11.2868;
metmast_TH[51].ws_110m[0]=11.1747;
metmast_TH[52].ws_110m[0]=11.4646;
metmast_TH[53].ws_110m[0]=11.1474;
metmast_TH[54].ws_110m[0]=11.4646;
metmast_TH[55].ws_110m[0]=11.3306;
metmast_TH[56].ws_110m[0]=11.4263;
metmast_TH[57].ws_110m[0]=11.2759;
metmast_TH[58].ws_110m[0]=11.4044;
metmast_TH[59].ws_110m[0]=11.3908;
metmast_TH[60].ws_110m[0]=11.2786;
metmast_TH[61].ws_110m[0]=11.0025;
metmast_TH[62].ws_110m[0]=11.3415;
metmast_TH[63].ws_110m[0]=11.306;
metmast_TH[64].ws_110m[0]=11.1556;
metmast_TH[65].ws_110m[0]=11.0189;
metmast_TH[66].ws_110m[0]=11.1911;
metmast_TH[67].ws_110m[0]=11.6068;
metmast_TH[68].ws_110m[0]=11.6204;
metmast_TH[69].ws_110m[0]=11.4181;
metmast_TH[70].ws_110m[0]=11.4236;
metmast_TH[71].ws_110m[0]=11.3935;
metmast_TH[72].ws_110m[0]=11.7052;
metmast_TH[73].ws_110m[0]=11.5876;
metmast_TH[74].ws_110m[0]=11.7435;
metmast_TH[75].ws_110m[0]=11.8611;
metmast_TH[76].ws_110m[0]=11.5111;
metmast_TH[77].ws_110m[0]=11.3853;
metmast_TH[78].ws_110m[0]=11.6177;
metmast_TH[79].ws_110m[0]=11.3443;
metmast_TH[80].ws_110m[0]=11.3798;
metmast_TH[81].ws_110m[0]=11.2841;
metmast_TH[82].ws_110m[0]=11.4126;
metmast_TH[83].ws_110m[0]=11.2048;
metmast_TH[84].ws_110m[0]=11.5712;
metmast_TH[85].ws_110m[0]=11.9048;
metmast_TH[86].ws_110m[0]=12.1017;
metmast_TH[87].ws_110m[0]=12.211;
metmast_TH[88].ws_110m[0]=12.3314;
metmast_TH[89].ws_110m[0]=12.5884;
metmast_TH[90].ws_110m[0]=12.5966;
metmast_TH[91].ws_110m[0]=12.9958;
metmast_TH[92].ws_110m[0]=12.3149;
metmast_TH[93].ws_110m[0]=12.2274;
metmast_TH[94].ws_110m[0]=12.2548;
metmast_TH[95].ws_110m[0]=12.479;
metmast_TH[96].ws_110m[0]=12.1454;
metmast_TH[97].ws_110m[0]=12.0743;
metmast_TH[98].ws_110m[0]=12.0443;
metmast_TH[99].ws_110m[0]=12.0579;
metmast_TH[100].ws_110m[0]=12.14;
metmast_TH[101].ws_110m[0]=11.7325;
metmast_TH[102].ws_110m[0]=11.5439;
metmast_TH[103].ws_110m[0]=11.4263;
metmast_TH[104].ws_110m[0]=11.6013;
metmast_TH[105].ws_110m[0]=11.3908;
metmast_TH[106].ws_110m[0]=11.6396;
metmast_TH[107].ws_110m[0]=11.4236;
metmast_TH[108].ws_110m[0]=11.5111;
metmast_TH[109].ws_110m[0]=11.3771;
metmast_TH[110].ws_110m[0]=11.3087;
metmast_TH[111].ws_110m[0]=11.6833;
metmast_TH[112].ws_110m[0]=11.5466;
metmast_TH[113].ws_110m[0]=11.5493;
metmast_TH[114].ws_110m[0]=11.6341;
metmast_TH[115].ws_110m[0]=11.8446;
metmast_TH[116].ws_110m[0]=11.4318;
metmast_TH[117].ws_110m[0]=11.3607;
metmast_TH[118].ws_110m[0]=11.3333;
metmast_TH[119].ws_110m[0]=11.7216;
metmast_TH[120].ws_110m[0]=11.8419;
metmast_TH[121].ws_110m[0]=11.9157;
metmast_TH[122].ws_110m[0]=11.8173;
metmast_TH[123].ws_110m[0]=12.0743;
metmast_TH[124].ws_110m[0]=12.1892;
metmast_TH[125].ws_110m[0]=12.3095;
metmast_TH[126].ws_110m[0]=12.4544;
metmast_TH[127].ws_110m[0]=12.1235;
metmast_TH[128].ws_110m[0]=12.0634;
metmast_TH[129].ws_110m[0]=12.0661;
metmast_TH[130].ws_110m[0]=12.0661;
metmast_TH[131].ws_110m[0]=12.304;
metmast_TH[132].ws_110m[0]=12.7688;
metmast_TH[133].ws_110m[0]=12.952;
metmast_TH[134].ws_110m[0]=12.9548;
metmast_TH[135].ws_110m[0]=12.9575;
metmast_TH[136].ws_110m[0]=12.8563;
metmast_TH[137].ws_110m[0]=12.6239;
metmast_TH[138].ws_110m[0]=12.5747;
metmast_TH[139].ws_110m[0]=12.5856;
metmast_TH[140].ws_110m[0]=12.788;
metmast_TH[141].ws_110m[0]=12.9821;
metmast_TH[142].ws_110m[0]=13.1216;
metmast_TH[143].ws_110m[0]=12.7825;
metmast_TH[144].ws_110m[0]=13.004;
metmast_TH[145].ws_110m[0]=13.4305;
metmast_TH[146].ws_110m[0]=13.4087;
metmast_TH[147].ws_110m[0]=13.168;
metmast_TH[148].ws_110m[0]=12.7661;
metmast_TH[149].ws_110m[0]=12.531;
metmast_TH[150].ws_110m[0]=12.3778;
metmast_TH[151].ws_110m[0]=12.0907;
metmast_TH[152].ws_110m[0]=12.088;
metmast_TH[153].ws_110m[0]=12.0525;
metmast_TH[154].ws_110m[0]=12.6485;
metmast_TH[155].ws_110m[0]=12.7251;
metmast_TH[156].ws_110m[0]=12.5528;
metmast_TH[157].ws_110m[0]=12.4872;
metmast_TH[158].ws_110m[0]=12.6157;
metmast_TH[159].ws_110m[0]=13.1817;
metmast_TH[160].ws_110m[0]=13.0122;
metmast_TH[161].ws_110m[0]=13.1489;
metmast_TH[162].ws_110m[0]=12.8645;
metmast_TH[163].ws_110m[0]=12.6977;
metmast_TH[164].ws_110m[0]=12.7962;
metmast_TH[165].ws_110m[0]=12.8345;
metmast_TH[166].ws_110m[0]=12.736;
metmast_TH[167].ws_110m[0]=12.6622;
metmast_TH[168].ws_110m[0]=12.7989;
metmast_TH[169].ws_110m[0]=12.4981;
metmast_TH[170].ws_110m[0]=12.8153;
metmast_TH[171].ws_110m[0]=12.7825;
metmast_TH[172].ws_110m[0]=12.7552;
metmast_TH[173].ws_110m[0]=12.8591;
metmast_TH[174].ws_110m[0]=13.1845;
metmast_TH[175].ws_110m[0]=13.4251;
metmast_TH[176].ws_110m[0]=12.4599;
metmast_TH[177].ws_110m[0]=12.5173;
metmast_TH[178].ws_110m[0]=12.4927;
metmast_TH[179].ws_110m[0]=12.5774;
metmast_TH[180].ws_110m[0]=12.6185;
metmast_TH[181].ws_110m[0]=12.695;
metmast_TH[182].ws_110m[0]=12.9356;
metmast_TH[183].ws_110m[0]=13.1544;
metmast_TH[184].ws_110m[0]=13.1134;
metmast_TH[185].ws_110m[0]=12.8235;
metmast_TH[186].ws_110m[0]=13.2446;
metmast_TH[187].ws_110m[0]=12.8946;
metmast_TH[188].ws_110m[0]=12.2985;
metmast_TH[189].ws_110m[0]=12.1974;
metmast_TH[190].ws_110m[0]=12.5938;
metmast_TH[191].ws_110m[0]=11.9677;
metmast_TH[192].ws_110m[0]=12.129;
metmast_TH[193].ws_110m[0]=12.695;
metmast_TH[194].ws_110m[0]=12.1071;
metmast_TH[195].ws_110m[0]=12.5966;
metmast_TH[196].ws_110m[0]=12.2958;
metmast_TH[197].ws_110m[0]=12.3942;
metmast_TH[198].ws_110m[0]=12.2329;
metmast_TH[199].ws_110m[0]=12.4954;
metmast_TH[200].ws_110m[0]=12.4243;
metmast_TH[201].ws_110m[0]=12.3942;
metmast_TH[202].ws_110m[0]=12.2138;
metmast_TH[203].ws_110m[0]=11.8255;
metmast_TH[204].ws_110m[0]=12.0907;
metmast_TH[205].ws_110m[0]=11.8392;
metmast_TH[206].ws_110m[0]=12.2876;
metmast_TH[207].ws_110m[0]=11.9814;
metmast_TH[208].ws_110m[0]=11.7626;
metmast_TH[209].ws_110m[0]=11.8829;
metmast_TH[210].ws_110m[0]=11.7872;
metmast_TH[211].ws_110m[0]=11.9431;
metmast_TH[212].ws_110m[0]=11.44;
metmast_TH[213].ws_110m[0]=11.5822;
metmast_TH[214].ws_110m[0]=11.224;
metmast_TH[215].ws_110m[0]=11.2404;
metmast_TH[216].ws_110m[0]=11.7489;
metmast_TH[217].ws_110m[0]=11.574;
metmast_TH[218].ws_110m[0]=11.7435;
metmast_TH[219].ws_110m[0]=11.7353;
metmast_TH[220].ws_110m[0]=12.2739;
metmast_TH[221].ws_110m[0]=12.1974;
metmast_TH[222].ws_110m[0]=12.5774;
metmast_TH[223].ws_110m[0]=12.5255;
metmast_TH[224].ws_110m[0]=12.1235;
metmast_TH[225].ws_110m[0]=11.8228;
metmast_TH[226].ws_110m[0]=12.0005;
metmast_TH[227].ws_110m[0]=11.9622;
metmast_TH[228].ws_110m[0]=12.0825;
metmast_TH[229].ws_110m[0]=12.006;
metmast_TH[230].ws_110m[0]=11.9212;
metmast_TH[231].ws_110m[0]=12.0087;
metmast_TH[232].ws_110m[0]=11.7243;
metmast_TH[233].ws_110m[0]=11.5275;
metmast_TH[234].ws_110m[0]=11.6888;
metmast_TH[235].ws_110m[0]=11.7626;
metmast_TH[236].ws_110m[0]=12.2192;
metmast_TH[237].ws_110m[0]=12.2712;
metmast_TH[238].ws_110m[0]=12.1153;
metmast_TH[239].ws_110m[0]=12.2357;
metmast_TH[240].ws_110m[0]=12.4243;
metmast_TH[241].ws_110m[0]=12.0169;
metmast_TH[242].ws_110m[0]=11.6532;
metmast_TH[243].ws_110m[0]=12.1892;
metmast_TH[244].ws_110m[0]=12.1564;
metmast_TH[245].ws_110m[0]=12.0087;
metmast_TH[246].ws_110m[0]=12.181;
metmast_TH[247].ws_110m[0]=11.8419;
metmast_TH[248].ws_110m[0]=11.831;
metmast_TH[249].ws_110m[0]=11.8857;
metmast_TH[250].ws_110m[0]=11.7243;
metmast_TH[251].ws_110m[0]=11.9978;
metmast_TH[252].ws_110m[0]=11.7736;
metmast_TH[253].ws_110m[0]=11.7161;
metmast_TH[254].ws_110m[0]=11.8583;
metmast_TH[255].ws_110m[0]=11.6396;
metmast_TH[256].ws_110m[0]=11.5822;
metmast_TH[257].ws_110m[0]=11.7681;
metmast_TH[258].ws_110m[0]=11.7517;
metmast_TH[259].ws_110m[0]=11.7571;
metmast_TH[260].ws_110m[0]=11.8446;
metmast_TH[261].ws_110m[0]=11.8064;
metmast_TH[262].ws_110m[0]=11.6642;
metmast_TH[263].ws_110m[0]=11.7216;
metmast_TH[264].ws_110m[0]=11.5357;
metmast_TH[265].ws_110m[0]=11.9978;
metmast_TH[266].ws_110m[0]=11.9595;
metmast_TH[267].ws_110m[0]=12.0579;
metmast_TH[268].ws_110m[0]=12.5364;
metmast_TH[269].ws_110m[0]=12.4845;
metmast_TH[270].ws_110m[0]=12.1427;
metmast_TH[271].ws_110m[0]=12.3806;
metmast_TH[272].ws_110m[0]=12.3505;
metmast_TH[273].ws_110m[0]=12.6704;
metmast_TH[274].ws_110m[0]=12.7169;
metmast_TH[275].ws_110m[0]=12.7087;
metmast_TH[276].ws_110m[0]=12.6431;
metmast_TH[277].ws_110m[0]=12.3122;
metmast_TH[278].ws_110m[0]=12.6485;
metmast_TH[279].ws_110m[0]=12.8727;
metmast_TH[280].ws_110m[0]=12.8372;
metmast_TH[281].ws_110m[0]=12.5364;
metmast_TH[282].ws_110m[0]=12.52;
metmast_TH[283].ws_110m[0]=12.654;
metmast_TH[284].ws_110m[0]=12.2603;
metmast_TH[285].ws_110m[0]=12.6349;
metmast_TH[286].ws_110m[0]=12.8235;
metmast_TH[287].ws_110m[0]=12.2821;
metmast_TH[288].ws_110m[0]=12.3013;
metmast_TH[289].ws_110m[0]=12.2357;
metmast_TH[290].ws_110m[0]=12.6759;
metmast_TH[291].ws_110m[0]=12.6349;
metmast_TH[292].ws_110m[0]=12.6813;
metmast_TH[293].ws_110m[0]=12.7306;
metmast_TH[294].ws_110m[0]=12.8481;
metmast_TH[295].ws_110m[0]=12.6731;
metmast_TH[296].ws_110m[0]=12.6349;
metmast_TH[297].ws_110m[0]=12.9192;
metmast_TH[298].ws_110m[0]=12.5419;
metmast_TH[299].ws_110m[0]=12.4161;
metmast_TH[300].ws_110m[0]=12.1673;
metmast_TH[301].ws_110m[0]=12.1509;
metmast_TH[302].ws_110m[0]=12.2439;
metmast_TH[303].ws_110m[0]=12.1017;
metmast_TH[304].ws_110m[0]=11.965;
metmast_TH[305].ws_110m[0]=11.965;
metmast_TH[306].ws_110m[0]=11.82;
metmast_TH[307].ws_110m[0]=12.3177;
metmast_TH[308].ws_110m[0]=11.5657;
metmast_TH[309].ws_110m[0]=11.8255;
metmast_TH[310].ws_110m[0]=12.0771;
metmast_TH[311].ws_110m[0]=11.6997;
metmast_TH[312].ws_110m[0]=11.9431;
metmast_TH[313].ws_110m[0]=12.5009;
metmast_TH[314].ws_110m[0]=12.3942;
metmast_TH[315].ws_110m[0]=12.1454;
metmast_TH[316].ws_110m[0]=12.2384;
metmast_TH[317].ws_110m[0]=12.3532;
metmast_TH[318].ws_110m[0]=12.0962;
metmast_TH[319].ws_110m[0]=11.9212;
metmast_TH[320].ws_110m[0]=12.2028;
metmast_TH[321].ws_110m[0]=11.8118;
metmast_TH[322].ws_110m[0]=11.7872;
metmast_TH[323].ws_110m[0]=12.0114;
metmast_TH[324].ws_110m[0]=11.9595;
metmast_TH[325].ws_110m[0]=11.9704;
metmast_TH[326].ws_110m[0]=12.2138;
metmast_TH[327].ws_110m[0]=12.3122;
metmast_TH[328].ws_110m[0]=12.036;
metmast_TH[329].ws_110m[0]=12.2247;
metmast_TH[330].ws_110m[0]=12.3997;
metmast_TH[331].ws_110m[0]=11.9458;
metmast_TH[332].ws_110m[0]=11.8693;
metmast_TH[333].ws_110m[0]=11.8146;
metmast_TH[334].ws_110m[0]=11.5548;
metmast_TH[335].ws_110m[0]=12.3232;
metmast_TH[336].ws_110m[0]=12.0388;
metmast_TH[337].ws_110m[0]=12.3915;
metmast_TH[338].ws_110m[0]=12.2903;
metmast_TH[339].ws_110m[0]=12.2849;
metmast_TH[340].ws_110m[0]=12.3833;
metmast_TH[341].ws_110m[0]=11.9622;
metmast_TH[342].ws_110m[0]=11.9622;
metmast_TH[343].ws_110m[0]=11.7216;
metmast_TH[344].ws_110m[0]=11.7325;
metmast_TH[345].ws_110m[0]=12.0251;
metmast_TH[346].ws_110m[0]=11.9403;
metmast_TH[347].ws_110m[0]=12.2192;
metmast_TH[348].ws_110m[0]=11.9568;
metmast_TH[349].ws_110m[0]=12.0853;
metmast_TH[350].ws_110m[0]=12.2575;
metmast_TH[351].ws_110m[0]=12.0743;
metmast_TH[352].ws_110m[0]=12.211;
metmast_TH[353].ws_110m[0]=11.779;
metmast_TH[354].ws_110m[0]=11.7626;
metmast_TH[355].ws_110m[0]=11.7134;
metmast_TH[356].ws_110m[0]=11.5657;
metmast_TH[357].ws_110m[0]=11.6614;
metmast_TH[358].ws_110m[0]=11.8775;
metmast_TH[359].ws_110m[0]=11.7407;
metmast_TH[360].ws_110m[0]=11.8747;
metmast_TH[361].ws_110m[0]=11.7107;
metmast_TH[362].ws_110m[0]=11.5794;
metmast_TH[363].ws_110m[0]=11.4454;
metmast_TH[364].ws_110m[0]=11.2677;
metmast_TH[365].ws_110m[0]=11.0544;
metmast_TH[366].ws_110m[0]=10.7673;
metmast_TH[367].ws_110m[0]=11.1665;
metmast_TH[368].ws_110m[0]=11.4427;
metmast_TH[369].ws_110m[0]=11.6204;
metmast_TH[370].ws_110m[0]=11.2431;
metmast_TH[371].ws_110m[0]=11.0271;
metmast_TH[372].ws_110m[0]=11.0654;
metmast_TH[373].ws_110m[0]=11.5083;
metmast_TH[374].ws_110m[0]=11.5439;
metmast_TH[375].ws_110m[0]=10.9997;
metmast_TH[376].ws_110m[0]=10.8685;
metmast_TH[377].ws_110m[0]=11.3333;
metmast_TH[378].ws_110m[0]=10.7291;
metmast_TH[379].ws_110m[0]=10.3572;
metmast_TH[380].ws_110m[0]=10.1412;
metmast_TH[381].ws_110m[0]=10.2451;
metmast_TH[382].ws_110m[0]=10.3736;
metmast_TH[383].ws_110m[0]=10.2615;
metmast_TH[384].ws_110m[0]=10.3927;
metmast_TH[385].ws_110m[0]=10.5513;
metmast_TH[386].ws_110m[0]=10.5541;
metmast_TH[387].ws_110m[0]=11.2048;
metmast_TH[388].ws_110m[0]=11.3306;
metmast_TH[389].ws_110m[0]=11.6669;
metmast_TH[390].ws_110m[0]=12.5501;
metmast_TH[391].ws_110m[0]=12.2138;
metmast_TH[392].ws_110m[0]=12.4817;
metmast_TH[393].ws_110m[0]=13.1079;
metmast_TH[394].ws_110m[0]=12.6977;
metmast_TH[395].ws_110m[0]=12.5966;
metmast_TH[396].ws_110m[0]=12.3888;
metmast_TH[397].ws_110m[0]=12.9001;
metmast_TH[398].ws_110m[0]=12.7688;
metmast_TH[399].ws_110m[0]=13.1325;
metmast_TH[400].ws_110m[0]=12.9192;
metmast_TH[401].ws_110m[0]=12.8563;
metmast_TH[402].ws_110m[0]=12.7005;
metmast_TH[403].ws_110m[0]=12.6376;
metmast_TH[404].ws_110m[0]=12.829;
metmast_TH[405].ws_110m[0]=12.7907;
metmast_TH[406].ws_110m[0]=12.3368;
metmast_TH[407].ws_110m[0]=11.9759;
metmast_TH[408].ws_110m[0]=11.9622;
metmast_TH[409].ws_110m[0]=12.0032;
metmast_TH[410].ws_110m[0]=11.9759;
metmast_TH[411].ws_110m[0]=12.2192;
metmast_TH[412].ws_110m[0]=12.5774;
metmast_TH[413].ws_110m[0]=12.9466;
metmast_TH[414].ws_110m[0]=12.7442;
metmast_TH[415].ws_110m[0]=12.4544;
metmast_TH[416].ws_110m[0]=12.3669;
metmast_TH[417].ws_110m[0]=12.1673;
metmast_TH[418].ws_110m[0]=12.2958;
metmast_TH[419].ws_110m[0]=12.2603;
metmast_TH[420].ws_110m[0]=11.9677;
metmast_TH[421].ws_110m[0]=11.9622;
metmast_TH[422].ws_110m[0]=11.9704;
metmast_TH[423].ws_110m[0]=11.697;
metmast_TH[424].ws_110m[0]=11.9759;
metmast_TH[425].ws_110m[0]=11.8337;
metmast_TH[426].ws_110m[0]=11.6314;
metmast_TH[427].ws_110m[0]=11.522;
metmast_TH[428].ws_110m[0]=11.5712;
metmast_TH[429].ws_110m[0]=11.3333;
metmast_TH[430].ws_110m[0]=11.2349;
metmast_TH[431].ws_110m[0]=11.3743;
metmast_TH[432].ws_110m[0]=11.4837;
metmast_TH[433].ws_110m[0]=11.4755;
metmast_TH[434].ws_110m[0]=11.5794;
metmast_TH[435].ws_110m[0]=11.3552;
metmast_TH[436].ws_110m[0]=11.3716;
metmast_TH[437].ws_110m[0]=12.0443;
metmast_TH[438].ws_110m[0]=11.8747;
metmast_TH[439].ws_110m[0]=11.7025;
metmast_TH[440].ws_110m[0]=11.7353;
metmast_TH[441].ws_110m[0]=11.5876;
metmast_TH[442].ws_110m[0]=11.7982;
metmast_TH[443].ws_110m[0]=11.8146;
metmast_TH[444].ws_110m[0]=12.1071;
metmast_TH[445].ws_110m[0]=12.0005;
metmast_TH[446].ws_110m[0]=12.6157;
metmast_TH[447].ws_110m[0]=11.8282;
metmast_TH[448].ws_110m[0]=11.831;
metmast_TH[449].ws_110m[0]=11.9978;
metmast_TH[450].ws_110m[0]=12.2001;
metmast_TH[451].ws_110m[0]=12.356;
metmast_TH[452].ws_110m[0]=12.1153;
metmast_TH[453].ws_110m[0]=12.8946;
metmast_TH[454].ws_110m[0]=12.6321;
metmast_TH[455].ws_110m[0]=12.5228;
metmast_TH[456].ws_110m[0]=12.4599;
metmast_TH[457].ws_110m[0]=12.5036;
metmast_TH[458].ws_110m[0]=12.5446;
metmast_TH[459].ws_110m[0]=12.4435;
metmast_TH[460].ws_110m[0]=12.1782;
metmast_TH[461].ws_110m[0]=12.2521;
metmast_TH[462].ws_110m[0]=12.3724;
metmast_TH[463].ws_110m[0]=12.2329;
metmast_TH[464].ws_110m[0]=12.2384;
metmast_TH[465].ws_110m[0]=12.1317;
metmast_TH[466].ws_110m[0]=12.3478;
metmast_TH[467].ws_110m[0]=11.8446;
metmast_TH[468].ws_110m[0]=11.8802;
metmast_TH[469].ws_110m[0]=11.8693;
metmast_TH[470].ws_110m[0]=12.0798;
metmast_TH[471].ws_110m[0]=11.9622;
metmast_TH[472].ws_110m[0]=12.3177;
metmast_TH[473].ws_110m[0]=12.3314;
metmast_TH[474].ws_110m[0]=12.2903;
metmast_TH[475].ws_110m[0]=12.4243;
metmast_TH[476].ws_110m[0]=12.6759;
metmast_TH[477].ws_110m[0]=12.6759;
metmast_TH[478].ws_110m[0]=12.7251;
metmast_TH[479].ws_110m[0]=12.7688;
metmast_TH[480].ws_110m[0]=12.9849;
metmast_TH[481].ws_110m[0]=13.138;
metmast_TH[482].ws_110m[0]=12.8345;
metmast_TH[483].ws_110m[0]=13.0915;
metmast_TH[484].ws_110m[0]=12.2028;
metmast_TH[485].ws_110m[0]=12.4817;
metmast_TH[486].ws_110m[0]=12.3259;
metmast_TH[487].ws_110m[0]=12.2274;
metmast_TH[488].ws_110m[0]=12.006;
metmast_TH[489].ws_110m[0]=12.0087;
metmast_TH[490].ws_110m[0]=11.9513;
metmast_TH[491].ws_110m[0]=11.7681;
metmast_TH[492].ws_110m[0]=11.7243;
metmast_TH[493].ws_110m[0]=11.1939;
metmast_TH[494].ws_110m[0]=11.3415;
metmast_TH[495].ws_110m[0]=11.3415;
metmast_TH[496].ws_110m[0]=11.4263;
metmast_TH[497].ws_110m[0]=11.3743;
metmast_TH[498].ws_110m[0]=11.1283;
metmast_TH[499].ws_110m[0]=11.0708;
metmast_TH[500].ws_110m[0]=11.2978;
metmast_TH[501].ws_110m[0]=11.2978;
metmast_TH[502].ws_110m[0]=11.4673;
metmast_TH[503].ws_110m[0]=10.956;
metmast_TH[504].ws_110m[0]=10.9615;
metmast_TH[505].ws_110m[0]=11.1365;
metmast_TH[506].ws_110m[0]=10.915;
metmast_TH[507].ws_110m[0]=11.0654;
metmast_TH[508].ws_110m[0]=10.8931;
metmast_TH[509].ws_110m[0]=11.1146;
metmast_TH[510].ws_110m[0]=11.0244;
metmast_TH[511].ws_110m[0]=10.9259;
metmast_TH[512].ws_110m[0]=10.9177;
metmast_TH[513].ws_110m[0]=10.7208;
metmast_TH[514].ws_110m[0]=10.7208;
metmast_TH[515].ws_110m[0]=10.6689;
metmast_TH[516].ws_110m[0]=10.6962;
metmast_TH[517].ws_110m[0]=10.5376;
metmast_TH[518].ws_110m[0]=10.483;
metmast_TH[519].ws_110m[0]=10.6388;
metmast_TH[520].ws_110m[0]=10.2041;
metmast_TH[521].ws_110m[0]=9.9498;
metmast_TH[522].ws_110m[0]=10.0181;
metmast_TH[523].ws_110m[0]=10.3599;
metmast_TH[524].ws_110m[0]=10.3025;
metmast_TH[525].ws_110m[0]=10.5267;
metmast_TH[526].ws_110m[0]=10.5267;
metmast_TH[527].ws_110m[0]=10.2533;
metmast_TH[528].ws_110m[0]=10.3955;
metmast_TH[529].ws_110m[0]=10.3955;
metmast_TH[530].ws_110m[0]=10.4228;
metmast_TH[531].ws_110m[0]=10.6251;
metmast_TH[532].ws_110m[0]=10.6251;
metmast_TH[533].ws_110m[0]=10.7755;
metmast_TH[534].ws_110m[0]=10.9833;
metmast_TH[535].ws_110m[0]=10.524;
metmast_TH[536].ws_110m[0]=10.472;
metmast_TH[537].ws_110m[0]=10.2369;
metmast_TH[538].ws_110m[0]=9.8978;
metmast_TH[539].ws_110m[0]=9.8978;
metmast_TH[540].ws_110m[0]=9.6517;
metmast_TH[541].ws_110m[0]=10.2451;
metmast_TH[542].ws_110m[0]=10.0947;
metmast_TH[543].ws_110m[0]=10.7427;
metmast_TH[544].ws_110m[0]=10.7154;
metmast_TH[545].ws_110m[0]=11.3579;
metmast_TH[546].ws_110m[0]=11.2322;
metmast_TH[547].ws_110m[0]=11.2076;
metmast_TH[548].ws_110m[0]=10.8466;
metmast_TH[549].ws_110m[0]=10.9669;
metmast_TH[550].ws_110m[0]=10.8849;
metmast_TH[551].ws_110m[0]=10.9751;
metmast_TH[552].ws_110m[0]=10.9751;
metmast_TH[553].ws_110m[0]=10.9177;
metmast_TH[554].ws_110m[0]=10.9177;
metmast_TH[555].ws_110m[0]=10.9451;
metmast_TH[556].ws_110m[0]=10.9806;
metmast_TH[557].ws_110m[0]=10.9806;
metmast_TH[558].ws_110m[0]=10.9095;
metmast_TH[559].ws_110m[0]=10.8056;
metmast_TH[560].ws_110m[0]=10.8056;
metmast_TH[561].ws_110m[0]=10.8056;
metmast_TH[562].ws_110m[0]=10.9724;
metmast_TH[563].ws_110m[0]=11.3634;
metmast_TH[564].ws_110m[0]=11.3634;
metmast_TH[565].ws_110m[0]=10.8876;
metmast_TH[566].ws_110m[0]=11.1337;
metmast_TH[567].ws_110m[0]=11.1337;
metmast_TH[568].ws_110m[0]=11.4755;
metmast_TH[569].ws_110m[0]=10.4502;
metmast_TH[570].ws_110m[0]=10.4502;
metmast_TH[571].ws_110m[0]=10.4502;
metmast_TH[572].ws_110m[0]=10.4857;
metmast_TH[573].ws_110m[0]=10.4857;
metmast_TH[574].ws_110m[0]=10.3107;
metmast_TH[575].ws_110m[0]=10.0045;
metmast_TH[576].ws_110m[0]=10.3353;
metmast_TH[577].ws_110m[0]=10.1959;
metmast_TH[578].ws_110m[0]=10.0728;
metmast_TH[579].ws_110m[0]=10.338;
metmast_TH[580].ws_110m[0]=10.4447;
metmast_TH[581].ws_110m[0]=10.2533;
metmast_TH[582].ws_110m[0]=10.3572;
metmast_TH[583].ws_110m[0]=10.3572;
metmast_TH[584].ws_110m[0]=10.3572;
metmast_TH[585].ws_110m[0]=11.0271;
metmast_TH[586].ws_110m[0]=11.0271;
metmast_TH[587].ws_110m[0]=11.0271;
metmast_TH[588].ws_110m[0]=11.0271;
metmast_TH[589].ws_110m[0]=11.0271;
metmast_TH[590].ws_110m[0]=10.4912;
metmast_TH[591].ws_110m[0]=10.4912;
metmast_TH[592].ws_110m[0]=10.4912;
metmast_TH[593].ws_110m[0]=10.5267;
metmast_TH[594].ws_110m[0]=10.5404;
metmast_TH[595].ws_110m[0]=10.5404;
metmast_TH[596].ws_110m[0]=10.3599;
metmast_TH[597].ws_110m[0]=10.4802;
metmast_TH[598].ws_110m[0]=10.3627;
metmast_TH[599].ws_110m[0]=10.2451;
metmast_TH[600].ws_110m[0]=10.0646;
metmast_TH[601].ws_110m[0]=9.9826;
metmast_TH[602].ws_110m[0]=9.9826;
metmast_TH[603].ws_110m[0]=10.2232;
metmast_TH[604].ws_110m[0]=10.174;
metmast_TH[605].ws_110m[0]=10.174;
metmast_TH[606].ws_110m[0]=9.8732;
metmast_TH[607].ws_110m[0]=9.6955;
metmast_TH[608].ws_110m[0]=9.7802;
metmast_TH[609].ws_110m[0]=9.5943;
metmast_TH[610].ws_110m[0]=9.5205;
metmast_TH[611].ws_110m[0]=9.5205;
metmast_TH[612].ws_110m[0]=9.6572;
metmast_TH[613].ws_110m[0]=9.474;
metmast_TH[614].ws_110m[0]=9.474;
metmast_TH[615].ws_110m[0]=8.8889;
metmast_TH[616].ws_110m[0]=9.0502;
metmast_TH[617].ws_110m[0]=8.7768;
metmast_TH[618].ws_110m[0]=8.7768;
metmast_TH[619].ws_110m[0]=8.6756;
metmast_TH[620].ws_110m[0]=8.6756;
metmast_TH[621].ws_110m[0]=8.7385;
metmast_TH[622].ws_110m[0]=8.6018;
metmast_TH[623].ws_110m[0]=8.8943;
metmast_TH[624].ws_110m[0]=8.8943;
metmast_TH[625].ws_110m[0]=8.8533;
metmast_TH[626].ws_110m[0]=9.0939;
metmast_TH[627].ws_110m[0]=9.0939;
metmast_TH[628].ws_110m[0]=9.0912;
metmast_TH[629].ws_110m[0]=9.1705;
metmast_TH[630].ws_110m[0]=9.1705;
metmast_TH[631].ws_110m[0]=9.9662;
metmast_TH[632].ws_110m[0]=9.5451;
metmast_TH[633].ws_110m[0]=9.5451;
metmast_TH[634].ws_110m[0]=9.5451;
metmast_TH[635].ws_110m[0]=10.1931;
metmast_TH[636].ws_110m[0]=10.1931;
metmast_TH[637].ws_110m[0]=10.1904;
metmast_TH[638].ws_110m[0]=9.7392;
metmast_TH[639].ws_110m[0]=9.7392;
metmast_TH[640].ws_110m[0]=9.7885;
metmast_TH[641].ws_110m[0]=9.7201;
metmast_TH[642].ws_110m[0]=9.7201;
metmast_TH[643].ws_110m[0]=9.6709;
metmast_TH[644].ws_110m[0]=9.6709;
metmast_TH[645].ws_110m[0]=9.6709;
metmast_TH[646].ws_110m[0]=9.4603;
metmast_TH[647].ws_110m[0]=9.4603;
metmast_TH[648].ws_110m[0]=9.4603;
metmast_TH[649].ws_110m[0]=9.2963;
metmast_TH[650].ws_110m[0]=9.3482;
metmast_TH[651].ws_110m[0]=9.6654;
metmast_TH[652].ws_110m[0]=9.6654;
metmast_TH[653].ws_110m[0]=9.4904;
metmast_TH[654].ws_110m[0]=9.6654;
metmast_TH[655].ws_110m[0]=9.6654;
metmast_TH[656].ws_110m[0]=9.6107;
metmast_TH[657].ws_110m[0]=9.6107;
metmast_TH[658].ws_110m[0]=9.6353;
metmast_TH[659].ws_110m[0]=9.6353;
metmast_TH[660].ws_110m[0]=9.526;
metmast_TH[661].ws_110m[0]=9.526;
metmast_TH[662].ws_110m[0]=9.526;
metmast_TH[663].ws_110m[0]=9.1814;
metmast_TH[664].ws_110m[0]=9.1814;
metmast_TH[665].ws_110m[0]=9.1814;
metmast_TH[666].ws_110m[0]=9.1814;
metmast_TH[667].ws_110m[0]=9.4849;
metmast_TH[668].ws_110m[0]=9.3646;
metmast_TH[669].ws_110m[0]=9.2471;
metmast_TH[670].ws_110m[0]=9.2334;
metmast_TH[671].ws_110m[0]=9.2307;
metmast_TH[672].ws_110m[0]=8.9846;
metmast_TH[673].ws_110m[0]=9.3373;
metmast_TH[674].ws_110m[0]=9.3373;
metmast_TH[675].ws_110m[0]=9.3428;
metmast_TH[676].ws_110m[0]=9.1295;
metmast_TH[677].ws_110m[0]=9.0584;
metmast_TH[678].ws_110m[0]=8.9436;
metmast_TH[679].ws_110m[0]=9.1568;
metmast_TH[680].ws_110m[0]=8.9654;
metmast_TH[681].ws_110m[0]=9.7283;
metmast_TH[682].ws_110m[0]=9.0857;
metmast_TH[683].ws_110m[0]=8.9463;
metmast_TH[684].ws_110m[0]=9.0228;
metmast_TH[685].ws_110m[0]=9.0201;
metmast_TH[686].ws_110m[0]=9.0529;
metmast_TH[687].ws_110m[0]=8.9107;
metmast_TH[688].ws_110m[0]=9.2088;
metmast_TH[689].ws_110m[0]=8.9818;
metmast_TH[690].ws_110m[0]=9.1322;
metmast_TH[691].ws_110m[0]=8.9025;
metmast_TH[692].ws_110m[0]=8.8834;
metmast_TH[693].ws_110m[0]=8.6072;
metmast_TH[694].ws_110m[0]=8.4979;
metmast_TH[695].ws_110m[0]=8.6018;
metmast_TH[696].ws_110m[0]=8.4268;
metmast_TH[697].ws_110m[0]=8.2354;
metmast_TH[698].ws_110m[0]=8.2026;
metmast_TH[699].ws_110m[0]=8.2135;
metmast_TH[700].ws_110m[0]=8.3666;
metmast_TH[701].ws_110m[0]=8.3775;
metmast_TH[702].ws_110m[0]=8.435;
metmast_TH[703].ws_110m[0]=8.9053;
metmast_TH[704].ws_110m[0]=8.61;
metmast_TH[705].ws_110m[0]=9.1842;
metmast_TH[706].ws_110m[0]=8.9107;
metmast_TH[707].ws_110m[0]=8.9572;
metmast_TH[708].ws_110m[0]=8.8779;
metmast_TH[709].ws_110m[0]=8.8561;
metmast_TH[710].ws_110m[0]=8.815;
metmast_TH[711].ws_110m[0]=8.7713;
metmast_TH[712].ws_110m[0]=8.8506;
metmast_TH[713].ws_110m[0]=8.9682;
metmast_TH[714].ws_110m[0]=8.8943;
metmast_TH[715].ws_110m[0]=8.8943;
metmast_TH[716].ws_110m[0]=8.8232;
metmast_TH[717].ws_110m[0]=8.8232;
metmast_TH[718].ws_110m[0]=8.8232;
metmast_TH[719].ws_110m[0]=9.3072;
metmast_TH[720].ws_110m[0]=9.0283;
metmast_TH[721].ws_110m[0]=8.7713;
metmast_TH[722].ws_110m[0]=8.7084;
metmast_TH[723].ws_110m[0]=8.7467;
metmast_TH[724].ws_110m[0]=8.8232;
metmast_TH[725].ws_110m[0]=8.9736;
metmast_TH[726].ws_110m[0]=8.64;
metmast_TH[727].ws_110m[0]=8.5744;
metmast_TH[728].ws_110m[0]=8.5279;
metmast_TH[729].ws_110m[0]=8.5143;
metmast_TH[730].ws_110m[0]=8.558;
metmast_TH[731].ws_110m[0]=8.6647;
metmast_TH[732].ws_110m[0]=8.5443;
metmast_TH[733].ws_110m[0]=8.5197;
metmast_TH[734].ws_110m[0]=8.6619;
metmast_TH[735].ws_110m[0]=8.6209;
metmast_TH[736].ws_110m[0]=8.826;
metmast_TH[737].ws_110m[0]=8.826;
metmast_TH[738].ws_110m[0]=8.6127;
metmast_TH[739].ws_110m[0]=8.8943;
metmast_TH[740].ws_110m[0]=8.8096;
metmast_TH[741].ws_110m[0]=8.8096;
metmast_TH[742].ws_110m[0]=8.6729;
metmast_TH[743].ws_110m[0]=8.3858;
metmast_TH[744].ws_110m[0]=8.7604;
metmast_TH[745].ws_110m[0]=8.8096;
metmast_TH[746].ws_110m[0]=8.949;
metmast_TH[747].ws_110m[0]=8.867;
metmast_TH[748].ws_110m[0]=8.8068;
metmast_TH[749].ws_110m[0]=8.4979;
metmast_TH[750].ws_110m[0]=8.4404;
metmast_TH[751].ws_110m[0]=8.1889;
metmast_TH[752].ws_110m[0]=8.3393;
metmast_TH[753].ws_110m[0]=8.692;
metmast_TH[754].ws_110m[0]=8.2053;
metmast_TH[755].ws_110m[0]=8.2053;
metmast_TH[756].ws_110m[0]=8.1779;
metmast_TH[757].ws_110m[0]=8.3775;
metmast_TH[758].ws_110m[0]=8.2791;
metmast_TH[759].ws_110m[0]=8.3201;
metmast_TH[760].ws_110m[0]=8.1342;
metmast_TH[761].ws_110m[0]=8.2381;
metmast_TH[762].ws_110m[0]=8.301;
metmast_TH[763].ws_110m[0]=8.5334;
metmast_TH[764].ws_110m[0]=8.8615;
metmast_TH[765].ws_110m[0]=8.8615;
metmast_TH[766].ws_110m[0]=8.8232;
metmast_TH[767].ws_110m[0]=8.8998;
metmast_TH[768].ws_110m[0]=8.8943;
metmast_TH[769].ws_110m[0]=8.8943;
metmast_TH[770].ws_110m[0]=8.8096;
metmast_TH[771].ws_110m[0]=8.7904;
metmast_TH[772].ws_110m[0]=8.6045;
metmast_TH[773].ws_110m[0]=8.465;
metmast_TH[774].ws_110m[0]=8.6373;
metmast_TH[775].ws_110m[0]=8.599;
metmast_TH[776].ws_110m[0]=8.6592;
metmast_TH[777].ws_110m[0]=8.4705;
metmast_TH[778].ws_110m[0]=8.7494;
metmast_TH[779].ws_110m[0]=8.5525;
metmast_TH[780].ws_110m[0]=9.0529;
metmast_TH[781].ws_110m[0]=8.8916;
metmast_TH[782].ws_110m[0]=9.3838;
metmast_TH[783].ws_110m[0]=9.4931;
metmast_TH[784].ws_110m[0]=9.7502;
metmast_TH[785].ws_110m[0]=9.3236;
metmast_TH[786].ws_110m[0]=9.2443;
metmast_TH[787].ws_110m[0]=9.4111;
metmast_TH[788].ws_110m[0]=9.1103;
metmast_TH[789].ws_110m[0]=9.0475;
metmast_TH[790].ws_110m[0]=9.2525;
metmast_TH[791].ws_110m[0]=9.1514;
metmast_TH[792].ws_110m[0]=9.2908;
metmast_TH[793].ws_110m[0]=9.031;
metmast_TH[794].ws_110m[0]=8.8861;
metmast_TH[795].ws_110m[0]=8.8889;
metmast_TH[796].ws_110m[0]=8.7631;
metmast_TH[797].ws_110m[0]=8.7002;
metmast_TH[798].ws_110m[0]=8.7221;
metmast_TH[799].ws_110m[0]=8.4158;
metmast_TH[800].ws_110m[0]=8.4897;
metmast_TH[801].ws_110m[0]=8.8861;
metmast_TH[802].ws_110m[0]=8.7576;
metmast_TH[803].ws_110m[0]=8.7439;
metmast_TH[804].ws_110m[0]=8.8615;
metmast_TH[805].ws_110m[0]=8.8561;
metmast_TH[806].ws_110m[0]=8.9189;
metmast_TH[807].ws_110m[0]=8.99;
metmast_TH[808].ws_110m[0]=8.9627;
metmast_TH[809].ws_110m[0]=9.2115;
metmast_TH[810].ws_110m[0]=9.1404;
metmast_TH[811].ws_110m[0]=9.2416;
metmast_TH[812].ws_110m[0]=9.2826;
metmast_TH[813].ws_110m[0]=9.1322;
metmast_TH[814].ws_110m[0]=9.2197;
metmast_TH[815].ws_110m[0]=9.2799;
metmast_TH[816].ws_110m[0]=9.2881;
metmast_TH[817].ws_110m[0]=9.1377;
metmast_TH[818].ws_110m[0]=9.2307;
metmast_TH[819].ws_110m[0]=9.2307;
metmast_TH[820].ws_110m[0]=9.1267;
metmast_TH[821].ws_110m[0]=9.0092;
metmast_TH[822].ws_110m[0]=8.7193;
metmast_TH[823].ws_110m[0]=8.867;
metmast_TH[824].ws_110m[0]=9.031;
metmast_TH[825].ws_110m[0]=8.9955;
metmast_TH[826].ws_110m[0]=9.1951;
metmast_TH[827].ws_110m[0]=8.867;
metmast_TH[828].ws_110m[0]=8.8752;
metmast_TH[829].ws_110m[0]=8.8287;
metmast_TH[830].ws_110m[0]=8.9873;
metmast_TH[831].ws_110m[0]=8.7658;
metmast_TH[832].ws_110m[0]=9.0256;
metmast_TH[833].ws_110m[0]=8.8342;
metmast_TH[834].ws_110m[0]=8.8506;
metmast_TH[835].ws_110m[0]=8.6455;
metmast_TH[836].ws_110m[0]=8.7193;
metmast_TH[837].ws_110m[0]=8.5717;
metmast_TH[838].ws_110m[0]=8.6674;
metmast_TH[839].ws_110m[0]=8.5498;
metmast_TH[840].ws_110m[0]=8.6373;
metmast_TH[841].ws_110m[0]=8.4022;
metmast_TH[842].ws_110m[0]=8.517;
metmast_TH[843].ws_110m[0]=8.3611;
metmast_TH[844].ws_110m[0]=8.5143;
metmast_TH[845].ws_110m[0]=8.6428;
metmast_TH[846].ws_110m[0]=8.7439;
metmast_TH[847].ws_110m[0]=8.6537;
metmast_TH[848].ws_110m[0]=8.785;
metmast_TH[849].ws_110m[0]=8.774;
metmast_TH[850].ws_110m[0]=8.5908;
metmast_TH[851].ws_110m[0]=8.5361;
metmast_TH[852].ws_110m[0]=8.6619;
metmast_TH[853].ws_110m[0]=8.6482;
metmast_TH[854].ws_110m[0]=8.867;
metmast_TH[855].ws_110m[0]=8.6647;
metmast_TH[856].ws_110m[0]=8.5115;
metmast_TH[857].ws_110m[0]=8.476;
metmast_TH[858].ws_110m[0]=8.301;
metmast_TH[859].ws_110m[0]=8.1369;
metmast_TH[860].ws_110m[0]=8.0002;
metmast_TH[861].ws_110m[0]=8.0467;
metmast_TH[862].ws_110m[0]=7.951;
metmast_TH[863].ws_110m[0]=8.1916;
metmast_TH[864].ws_110m[0]=8.3639;
metmast_TH[865].ws_110m[0]=8.2627;
metmast_TH[866].ws_110m[0]=8.2354;
metmast_TH[867].ws_110m[0]=8.0385;
metmast_TH[868].ws_110m[0]=8.2299;
metmast_TH[869].ws_110m[0]=8.1889;
metmast_TH[870].ws_110m[0]=8.1479;
metmast_TH[871].ws_110m[0]=8.2436;
metmast_TH[872].ws_110m[0]=8.1041;
metmast_TH[873].ws_110m[0]=8.0877;
metmast_TH[874].ws_110m[0]=8.1205;
metmast_TH[875].ws_110m[0]=8.0494;
metmast_TH[876].ws_110m[0]=8.0795;
metmast_TH[877].ws_110m[0]=8.0768;
metmast_TH[878].ws_110m[0]=8.1752;
metmast_TH[879].ws_110m[0]=8.6783;
metmast_TH[880].ws_110m[0]=8.208;
metmast_TH[881].ws_110m[0]=8.5525;
metmast_TH[882].ws_110m[0]=8.3858;
metmast_TH[883].ws_110m[0]=8.5061;
metmast_TH[884].ws_110m[0]=8.3502;
metmast_TH[885].ws_110m[0]=8.3092;
metmast_TH[886].ws_110m[0]=8.4514;
metmast_TH[887].ws_110m[0]=8.3475;
metmast_TH[888].ws_110m[0]=8.4131;
metmast_TH[889].ws_110m[0]=8.3037;
metmast_TH[890].ws_110m[0]=8.0166;
metmast_TH[891].ws_110m[0]=7.9674;
metmast_TH[892].ws_110m[0]=8.3092;
metmast_TH[893].ws_110m[0]=7.9865;
metmast_TH[894].ws_110m[0]=7.8334;
metmast_TH[895].ws_110m[0]=7.8362;
metmast_TH[896].ws_110m[0]=8.3256;
metmast_TH[897].ws_110m[0]=7.8963;
metmast_TH[898].ws_110m[0]=8.1807;
metmast_TH[899].ws_110m[0]=8.0494;
metmast_TH[900].ws_110m[0]=8.0658;
metmast_TH[901].ws_110m[0]=8.2299;
metmast_TH[902].ws_110m[0]=7.9647;
metmast_TH[903].ws_110m[0]=8.1233;
metmast_TH[904].ws_110m[0]=8.0959;
metmast_TH[905].ws_110m[0]=7.8608;
metmast_TH[906].ws_110m[0]=8.4486;
metmast_TH[907].ws_110m[0]=8.3037;
metmast_TH[908].ws_110m[0]=8.3721;
metmast_TH[909].ws_110m[0]=8.3338;
metmast_TH[910].ws_110m[0]=8.7822;
metmast_TH[911].ws_110m[0]=8.5553;
metmast_TH[912].ws_110m[0]=8.4678;
metmast_TH[913].ws_110m[0]=8.6783;
metmast_TH[914].ws_110m[0]=8.6701;
metmast_TH[915].ws_110m[0]=8.908;
metmast_TH[916].ws_110m[0]=9.0365;
metmast_TH[917].ws_110m[0]=8.6209;
metmast_TH[918].ws_110m[0]=8.9326;
metmast_TH[919].ws_110m[0]=8.599;
metmast_TH[920].ws_110m[0]=8.64;
metmast_TH[921].ws_110m[0]=8.3693;
metmast_TH[922].ws_110m[0]=8.7549;
metmast_TH[923].ws_110m[0]=8.4268;
metmast_TH[924].ws_110m[0]=8.6674;
metmast_TH[925].ws_110m[0]=8.6701;
metmast_TH[926].ws_110m[0]=8.5197;
metmast_TH[927].ws_110m[0]=8.9736;
metmast_TH[928].ws_110m[0]=8.6154;
metmast_TH[929].ws_110m[0]=8.9709;
metmast_TH[930].ws_110m[0]=8.5115;
metmast_TH[931].ws_110m[0]=8.5854;
metmast_TH[932].ws_110m[0]=8.4486;
metmast_TH[933].ws_110m[0]=8.6537;
metmast_TH[934].ws_110m[0]=8.476;
metmast_TH[935].ws_110m[0]=8.7193;
metmast_TH[936].ws_110m[0]=8.5963;
metmast_TH[937].ws_110m[0]=8.7084;
metmast_TH[938].ws_110m[0]=8.6482;
metmast_TH[939].ws_110m[0]=8.6811;
metmast_TH[940].ws_110m[0]=8.4951;
metmast_TH[941].ws_110m[0]=8.6756;
metmast_TH[942].ws_110m[0]=8.5607;
metmast_TH[943].ws_110m[0]=8.61;
metmast_TH[944].ws_110m[0]=8.5361;
metmast_TH[945].ws_110m[0]=8.7193;
metmast_TH[946].ws_110m[0]=8.4186;
metmast_TH[947].ws_110m[0]=8.651;
metmast_TH[948].ws_110m[0]=8.4568;
metmast_TH[949].ws_110m[0]=8.6701;
metmast_TH[950].ws_110m[0]=8.6264;
metmast_TH[951].ws_110m[0]=8.6674;
metmast_TH[952].ws_110m[0]=8.7959;
metmast_TH[953].ws_110m[0]=8.4842;
metmast_TH[954].ws_110m[0]=8.7084;
metmast_TH[955].ws_110m[0]=8.5225;
metmast_TH[956].ws_110m[0]=8.5635;
metmast_TH[957].ws_110m[0]=8.4404;
metmast_TH[958].ws_110m[0]=8.5635;
metmast_TH[959].ws_110m[0]=8.3885;
metmast_TH[960].ws_110m[0]=8.5525;
metmast_TH[961].ws_110m[0]=8.26;
metmast_TH[962].ws_110m[0]=8.3611;
metmast_TH[963].ws_110m[0]=8.3338;
metmast_TH[964].ws_110m[0]=8.4979;
metmast_TH[965].ws_110m[0]=8.2572;
metmast_TH[966].ws_110m[0]=8.2654;
metmast_TH[967].ws_110m[0]=8.5635;
metmast_TH[968].ws_110m[0]=8.4213;
metmast_TH[969].ws_110m[0]=8.6209;
metmast_TH[970].ws_110m[0]=8.6729;
metmast_TH[971].ws_110m[0]=8.8643;
metmast_TH[972].ws_110m[0]=8.7768;
metmast_TH[973].ws_110m[0]=8.9107;
metmast_TH[974].ws_110m[0]=8.7986;
metmast_TH[975].ws_110m[0]=8.9353;
metmast_TH[976].ws_110m[0]=8.7522;
metmast_TH[977].ws_110m[0]=8.9736;
metmast_TH[978].ws_110m[0]=8.8287;
metmast_TH[979].ws_110m[0]=8.8588;
metmast_TH[980].ws_110m[0]=8.96;
metmast_TH[981].ws_110m[0]=8.8451;
metmast_TH[982].ws_110m[0]=8.7768;
metmast_TH[983].ws_110m[0]=8.7658;
metmast_TH[984].ws_110m[0]=8.6592;
metmast_TH[985].ws_110m[0]=8.7248;
metmast_TH[986].ws_110m[0]=8.7822;
metmast_TH[987].ws_110m[0]=8.8068;
metmast_TH[988].ws_110m[0]=8.6236;
metmast_TH[989].ws_110m[0]=8.6838;
metmast_TH[990].ws_110m[0]=8.7002;
metmast_TH[991].ws_110m[0]=8.8123;
metmast_TH[992].ws_110m[0]=8.9627;
metmast_TH[993].ws_110m[0]=8.6072;
metmast_TH[994].ws_110m[0]=8.8916;
metmast_TH[995].ws_110m[0]=8.4705;
metmast_TH[996].ws_110m[0]=8.7412;
metmast_TH[997].ws_110m[0]=8.3584;
metmast_TH[998].ws_110m[0]=8.5307;
metmast_TH[999].ws_110m[0]=8.4459;
metmast_TH[0].ws_60m[0]=11.038;
metmast_TH[1].ws_60m[0]=11.1775;
metmast_TH[2].ws_60m[0]=10.9997;
metmast_TH[3].ws_60m[0]=10.7591;
metmast_TH[4].ws_60m[0]=10.6798;
metmast_TH[5].ws_60m[0]=10.6087;
metmast_TH[6].ws_60m[0]=10.7373;
metmast_TH[7].ws_60m[0]=10.9369;
metmast_TH[8].ws_60m[0]=10.9314;
metmast_TH[9].ws_60m[0]=11.0763;
metmast_TH[10].ws_60m[0]=10.7619;
metmast_TH[11].ws_60m[0]=10.6115;
metmast_TH[12].ws_60m[0]=10.8302;
metmast_TH[13].ws_60m[0]=10.5568;
metmast_TH[14].ws_60m[0]=10.9697;
metmast_TH[15].ws_60m[0]=10.7318;
metmast_TH[16].ws_60m[0]=10.8302;
metmast_TH[17].ws_60m[0]=11.079;
metmast_TH[18].ws_60m[0]=10.8083;
metmast_TH[19].ws_60m[0]=10.7236;
metmast_TH[20].ws_60m[0]=10.5322;
metmast_TH[21].ws_60m[0]=10.5814;
metmast_TH[22].ws_60m[0]=10.349;
metmast_TH[23].ws_60m[0]=10.2041;
metmast_TH[24].ws_60m[0]=10.2451;
metmast_TH[25].ws_60m[0]=10.2013;
metmast_TH[26].ws_60m[0]=10.2068;
metmast_TH[27].ws_60m[0]=10.2888;
metmast_TH[28].ws_60m[0]=9.9279;
metmast_TH[29].ws_60m[0]=9.8486;
metmast_TH[30].ws_60m[0]=9.9115;
metmast_TH[31].ws_60m[0]=9.6244;
metmast_TH[32].ws_60m[0]=9.8295;
metmast_TH[33].ws_60m[0]=10.0974;
metmast_TH[34].ws_60m[0]=10.1029;
metmast_TH[35].ws_60m[0]=9.7802;
metmast_TH[36].ws_60m[0]=10.338;
metmast_TH[37].ws_60m[0]=10.1986;
metmast_TH[38].ws_60m[0]=10.2041;
metmast_TH[39].ws_60m[0]=10.1576;
metmast_TH[40].ws_60m[0]=10.3025;
metmast_TH[41].ws_60m[0]=10.0974;
metmast_TH[42].ws_60m[0]=10.1166;
metmast_TH[43].ws_60m[0]=9.9525;
metmast_TH[44].ws_60m[0]=9.9744;
metmast_TH[45].ws_60m[0]=10.122;
metmast_TH[46].ws_60m[0]=10.2697;
metmast_TH[47].ws_60m[0]=10.1631;
metmast_TH[48].ws_60m[0]=10.5376;
metmast_TH[49].ws_60m[0]=10.6087;
metmast_TH[50].ws_60m[0]=10.3408;
metmast_TH[51].ws_60m[0]=10.2232;
metmast_TH[52].ws_60m[0]=10.0783;
metmast_TH[53].ws_60m[0]=10.3627;
metmast_TH[54].ws_60m[0]=10.431;
metmast_TH[55].ws_60m[0]=10.1959;
metmast_TH[56].ws_60m[0]=10.0045;
metmast_TH[57].ws_60m[0]=9.9142;
metmast_TH[58].ws_60m[0]=9.9252;
metmast_TH[59].ws_60m[0]=9.5834;
metmast_TH[60].ws_60m[0]=9.3373;
metmast_TH[61].ws_60m[0]=9.6381;
metmast_TH[62].ws_60m[0]=9.3783;
metmast_TH[63].ws_60m[0]=9.6435;
metmast_TH[64].ws_60m[0]=9.515;
metmast_TH[65].ws_60m[0]=9.3564;
metmast_TH[66].ws_60m[0]=9.7338;
metmast_TH[67].ws_60m[0]=9.6299;
metmast_TH[68].ws_60m[0]=9.4193;
metmast_TH[69].ws_60m[0]=9.7611;
metmast_TH[70].ws_60m[0]=9.7365;
metmast_TH[71].ws_60m[0]=9.6162;
metmast_TH[72].ws_60m[0]=9.7447;
metmast_TH[73].ws_60m[0]=9.8541;
metmast_TH[74].ws_60m[0]=9.7447;
metmast_TH[75].ws_60m[0]=9.7283;
metmast_TH[76].ws_60m[0]=9.8814;
metmast_TH[77].ws_60m[0]=9.7556;
metmast_TH[78].ws_60m[0]=9.8185;
metmast_TH[79].ws_60m[0]=10.133;
metmast_TH[80].ws_60m[0]=9.824;
metmast_TH[81].ws_60m[0]=9.7474;
metmast_TH[82].ws_60m[0]=9.8677;
metmast_TH[83].ws_60m[0]=10.1384;
metmast_TH[84].ws_60m[0]=9.947;
metmast_TH[85].ws_60m[0]=10.2259;
metmast_TH[86].ws_60m[0]=10.3654;
metmast_TH[87].ws_60m[0]=10.1494;
metmast_TH[88].ws_60m[0]=10.2943;
metmast_TH[89].ws_60m[0]=10.0564;
metmast_TH[90].ws_60m[0]=10.647;
metmast_TH[91].ws_60m[0]=10.3627;
metmast_TH[92].ws_60m[0]=10.1685;
metmast_TH[93].ws_60m[0]=10.1931;
metmast_TH[94].ws_60m[0]=10.4966;
metmast_TH[95].ws_60m[0]=10.9232;
metmast_TH[96].ws_60m[0]=10.4802;
metmast_TH[97].ws_60m[0]=10.2943;
metmast_TH[98].ws_60m[0]=10.1439;
metmast_TH[99].ws_60m[0]=10.3927;
metmast_TH[100].ws_60m[0]=10.513;
metmast_TH[101].ws_60m[0]=10.5869;
metmast_TH[102].ws_60m[0]=10.5459;
metmast_TH[103].ws_60m[0]=10.956;
metmast_TH[104].ws_60m[0]=11.0025;
metmast_TH[105].ws_60m[0]=10.9232;
metmast_TH[106].ws_60m[0]=11.038;
metmast_TH[107].ws_60m[0]=10.8658;
metmast_TH[108].ws_60m[0]=11.1857;
metmast_TH[109].ws_60m[0]=10.9478;
metmast_TH[110].ws_60m[0]=10.7755;
metmast_TH[111].ws_60m[0]=10.7345;
metmast_TH[112].ws_60m[0]=10.7099;
metmast_TH[113].ws_60m[0]=10.4884;
metmast_TH[114].ws_60m[0]=10.5513;
metmast_TH[115].ws_60m[0]=10.3189;
metmast_TH[116].ws_60m[0]=10.5267;
metmast_TH[117].ws_60m[0]=10.2423;
metmast_TH[118].ws_60m[0]=10.0783;
metmast_TH[119].ws_60m[0]=10.3982;
metmast_TH[120].ws_60m[0]=10.5349;
metmast_TH[121].ws_60m[0]=10.6005;
metmast_TH[122].ws_60m[0]=10.2888;
metmast_TH[123].ws_60m[0]=10.0646;
metmast_TH[124].ws_60m[0]=10.3763;
metmast_TH[125].ws_60m[0]=10.297;
metmast_TH[126].ws_60m[0]=10.2232;
metmast_TH[127].ws_60m[0]=10.0728;
metmast_TH[128].ws_60m[0]=10.2396;
metmast_TH[129].ws_60m[0]=10.3845;
metmast_TH[130].ws_60m[0]=10.4201;
metmast_TH[131].ws_60m[0]=10.3462;
metmast_TH[132].ws_60m[0]=10.0947;
metmast_TH[133].ws_60m[0]=9.8869;
metmast_TH[134].ws_60m[0]=10.0345;
metmast_TH[135].ws_60m[0]=9.8103;
metmast_TH[136].ws_60m[0]=9.8951;
metmast_TH[137].ws_60m[0]=9.6873;
metmast_TH[138].ws_60m[0]=9.6928;
metmast_TH[139].ws_60m[0]=9.5806;
metmast_TH[140].ws_60m[0]=9.6107;
metmast_TH[141].ws_60m[0]=9.4275;
metmast_TH[142].ws_60m[0]=9.4549;
metmast_TH[143].ws_60m[0]=9.69;
metmast_TH[144].ws_60m[0]=9.4029;
metmast_TH[145].ws_60m[0]=9.5834;
metmast_TH[146].ws_60m[0]=9.4959;
metmast_TH[147].ws_60m[0]=9.3537;
metmast_TH[148].ws_60m[0]=9.4685;
metmast_TH[149].ws_60m[0]=9.4193;
metmast_TH[150].ws_60m[0]=9.4002;
metmast_TH[151].ws_60m[0]=9.9279;
metmast_TH[152].ws_60m[0]=9.526;
metmast_TH[153].ws_60m[0]=9.9771;
metmast_TH[154].ws_60m[0]=9.5861;
metmast_TH[155].ws_60m[0]=9.2553;
metmast_TH[156].ws_60m[0]=9.2225;
metmast_TH[157].ws_60m[0]=8.9217;
metmast_TH[158].ws_60m[0]=9.0475;
metmast_TH[159].ws_60m[0]=8.815;
metmast_TH[160].ws_60m[0]=9.0775;
metmast_TH[161].ws_60m[0]=9.2525;
metmast_TH[162].ws_60m[0]=9.1568;
metmast_TH[163].ws_60m[0]=9.0967;
metmast_TH[164].ws_60m[0]=8.774;
metmast_TH[165].ws_60m[0]=8.7932;
metmast_TH[166].ws_60m[0]=8.5279;
metmast_TH[167].ws_60m[0]=8.6127;
metmast_TH[168].ws_60m[0]=8.4186;
metmast_TH[169].ws_60m[0]=8.4733;
metmast_TH[170].ws_60m[0]=8.3885;
metmast_TH[171].ws_60m[0]=8.2545;
metmast_TH[172].ws_60m[0]=8.2436;
metmast_TH[173].ws_60m[0]=8.1752;
metmast_TH[174].ws_60m[0]=8.5416;
metmast_TH[175].ws_60m[0]=8.4623;
metmast_TH[176].ws_60m[0]=8.7029;
metmast_TH[177].ws_60m[0]=8.4268;
metmast_TH[178].ws_60m[0]=8.3365;
metmast_TH[179].ws_60m[0]=8.1506;
metmast_TH[180].ws_60m[0]=8.3229;
metmast_TH[181].ws_60m[0]=8.383;
metmast_TH[182].ws_60m[0]=8.1397;
metmast_TH[183].ws_60m[0]=8.5279;
metmast_TH[184].ws_60m[0]=8.0713;
metmast_TH[185].ws_60m[0]=8.0822;
metmast_TH[186].ws_60m[0]=7.9865;
metmast_TH[187].ws_60m[0]=8.0139;
metmast_TH[188].ws_60m[0]=7.9811;
metmast_TH[189].ws_60m[0]=7.9209;
metmast_TH[190].ws_60m[0]=7.8608;
metmast_TH[191].ws_60m[0]=8.1916;
metmast_TH[192].ws_60m[0]=8.5279;
metmast_TH[193].ws_60m[0]=8.2162;
metmast_TH[194].ws_60m[0]=8.2983;
metmast_TH[195].ws_60m[0]=8.208;
metmast_TH[196].ws_60m[0]=8.2299;
metmast_TH[197].ws_60m[0]=8.0658;
metmast_TH[198].ws_60m[0]=8.1178;
metmast_TH[199].ws_60m[0]=8.085;
metmast_TH[200].ws_60m[0]=7.9182;
metmast_TH[201].ws_60m[0]=8.2436;
metmast_TH[202].ws_60m[0]=8.2709;
metmast_TH[203].ws_60m[0]=8.301;
metmast_TH[204].ws_60m[0]=8.4158;
metmast_TH[205].ws_60m[0]=8.2928;
metmast_TH[206].ws_60m[0]=8.4295;
metmast_TH[207].ws_60m[0]=8.5525;
metmast_TH[208].ws_60m[0]=8.7193;
metmast_TH[209].ws_60m[0]=8.867;
metmast_TH[210].ws_60m[0]=8.9326;
metmast_TH[211].ws_60m[0]=8.9572;
metmast_TH[212].ws_60m[0]=9.3373;
metmast_TH[213].ws_60m[0]=9.4193;
metmast_TH[214].ws_60m[0]=9.526;
metmast_TH[215].ws_60m[0]=9.5834;
metmast_TH[216].ws_60m[0]=9.5123;
metmast_TH[217].ws_60m[0]=9.206;
metmast_TH[218].ws_60m[0]=9.1705;
metmast_TH[219].ws_60m[0]=8.8916;
metmast_TH[220].ws_60m[0]=9.0803;
metmast_TH[221].ws_60m[0]=9.381;
metmast_TH[222].ws_60m[0]=9.4959;
metmast_TH[223].ws_60m[0]=9.0748;
metmast_TH[224].ws_60m[0]=8.9873;
metmast_TH[225].ws_60m[0]=8.9709;
metmast_TH[226].ws_60m[0]=8.5826;
metmast_TH[227].ws_60m[0]=8.8178;
metmast_TH[228].ws_60m[0]=8.465;
metmast_TH[229].ws_60m[0]=8.4104;
metmast_TH[230].ws_60m[0]=8.1014;
metmast_TH[231].ws_60m[0]=8.0194;
metmast_TH[232].ws_60m[0]=8.0904;
metmast_TH[233].ws_60m[0]=7.6967;
metmast_TH[234].ws_60m[0]=8.0713;
metmast_TH[235].ws_60m[0]=7.6557;
metmast_TH[236].ws_60m[0]=8.003;
metmast_TH[237].ws_60m[0]=8.2846;
metmast_TH[238].ws_60m[0]=8.4733;
metmast_TH[239].ws_60m[0]=8.5307;
metmast_TH[240].ws_60m[0]=8.7768;
metmast_TH[241].ws_60m[0]=8.9381;
metmast_TH[242].ws_60m[0]=8.9682;
metmast_TH[243].ws_60m[0]=8.9572;
metmast_TH[244].ws_60m[0]=8.6537;
metmast_TH[245].ws_60m[0]=8.5635;
metmast_TH[246].ws_60m[0]=8.342;
metmast_TH[247].ws_60m[0]=8.4432;
metmast_TH[248].ws_60m[0]=8.435;
metmast_TH[249].ws_60m[0]=8.5225;
metmast_TH[250].ws_60m[0]=8.4815;
metmast_TH[251].ws_60m[0]=8.465;
metmast_TH[252].ws_60m[0]=8.465;
metmast_TH[253].ws_60m[0]=8.0932;
metmast_TH[254].ws_60m[0]=8.1834;
metmast_TH[255].ws_60m[0]=7.9537;
metmast_TH[256].ws_60m[0]=8.1834;
metmast_TH[257].ws_60m[0]=8.7111;
metmast_TH[258].ws_60m[0]=8.8834;
metmast_TH[259].ws_60m[0]=8.9928;
metmast_TH[260].ws_60m[0]=9.3017;
metmast_TH[261].ws_60m[0]=9.3182;
metmast_TH[262].ws_60m[0]=9.9853;
metmast_TH[263].ws_60m[0]=10.0564;
metmast_TH[264].ws_60m[0]=10.308;
metmast_TH[265].ws_60m[0]=10.2013;
metmast_TH[266].ws_60m[0]=10.5705;
metmast_TH[267].ws_60m[0]=10.04;
metmast_TH[268].ws_60m[0]=9.7885;
metmast_TH[269].ws_60m[0]=10.1849;
metmast_TH[270].ws_60m[0]=10.0482;
metmast_TH[271].ws_60m[0]=10.1548;
metmast_TH[272].ws_60m[0]=10.2013;
metmast_TH[273].ws_60m[0]=10.1795;
metmast_TH[274].ws_60m[0]=10.5705;
metmast_TH[275].ws_60m[0]=10.7208;
metmast_TH[276].ws_60m[0]=10.4912;
metmast_TH[277].ws_60m[0]=10.74;
metmast_TH[278].ws_60m[0]=10.6935;
metmast_TH[279].ws_60m[0]=10.5486;
metmast_TH[280].ws_60m[0]=10.4939;
metmast_TH[281].ws_60m[0]=10.6306;
metmast_TH[282].ws_60m[0]=10.565;
metmast_TH[283].ws_60m[0]=10.5404;
metmast_TH[284].ws_60m[0]=10.8712;
metmast_TH[285].ws_60m[0]=10.8876;
metmast_TH[286].ws_60m[0]=10.8958;
metmast_TH[287].ws_60m[0]=10.7591;
metmast_TH[288].ws_60m[0]=10.4912;
metmast_TH[289].ws_60m[0]=10.6334;
metmast_TH[290].ws_60m[0]=10.5349;
metmast_TH[291].ws_60m[0]=10.6716;
metmast_TH[292].ws_60m[0]=10.688;
metmast_TH[293].ws_60m[0]=10.7755;
metmast_TH[294].ws_60m[0]=11.1037;
metmast_TH[295].ws_60m[0]=10.7291;
metmast_TH[296].ws_60m[0]=10.4447;
metmast_TH[297].ws_60m[0]=10.5814;
metmast_TH[298].ws_60m[0]=10.4337;
metmast_TH[299].ws_60m[0]=11.0326;
metmast_TH[300].ws_60m[0]=11.1228;
metmast_TH[301].ws_60m[0]=11.2404;
metmast_TH[302].ws_60m[0]=11.4454;
metmast_TH[303].ws_60m[0]=11.3853;
metmast_TH[304].ws_60m[0]=11.2103;
metmast_TH[305].ws_60m[0]=11.1665;
metmast_TH[306].ws_60m[0]=11.0353;
metmast_TH[307].ws_60m[0]=10.8029;
metmast_TH[308].ws_60m[0]=10.8056;
metmast_TH[309].ws_60m[0]=10.915;
metmast_TH[310].ws_60m[0]=11.2048;
metmast_TH[311].ws_60m[0]=10.8603;
metmast_TH[312].ws_60m[0]=10.9478;
metmast_TH[313].ws_60m[0]=10.9669;
metmast_TH[314].ws_60m[0]=11.2814;
metmast_TH[315].ws_60m[0]=11.1255;
metmast_TH[316].ws_60m[0]=11.4345;
metmast_TH[317].ws_60m[0]=11.2349;
metmast_TH[318].ws_60m[0]=10.9341;
metmast_TH[319].ws_60m[0]=11.1228;
metmast_TH[320].ws_60m[0]=11.2431;
metmast_TH[321].ws_60m[0]=11.0244;
metmast_TH[322].ws_60m[0]=11.172;
metmast_TH[323].ws_60m[0]=10.9095;
metmast_TH[324].ws_60m[0]=10.7345;
metmast_TH[325].ws_60m[0]=10.9669;
metmast_TH[326].ws_60m[0]=11.3607;
metmast_TH[327].ws_60m[0]=11.4865;
metmast_TH[328].ws_60m[0]=11.2677;
metmast_TH[329].ws_60m[0]=11.7134;
metmast_TH[330].ws_60m[0]=12.1536;
metmast_TH[331].ws_60m[0]=12.2439;
metmast_TH[332].ws_60m[0]=12.1755;
metmast_TH[333].ws_60m[0]=12.0716;
metmast_TH[334].ws_60m[0]=11.9923;
metmast_TH[335].ws_60m[0]=11.8501;
metmast_TH[336].ws_60m[0]=11.79;
metmast_TH[337].ws_60m[0]=12.0196;
metmast_TH[338].ws_60m[0]=11.6532;
metmast_TH[339].ws_60m[0]=11.8556;
metmast_TH[340].ws_60m[0]=11.7927;
metmast_TH[341].ws_60m[0]=11.6286;
metmast_TH[342].ws_60m[0]=11.6013;
metmast_TH[343].ws_60m[0]=11.5411;
metmast_TH[344].ws_60m[0]=11.5083;
metmast_TH[345].ws_60m[0]=11.6232;
metmast_TH[346].ws_60m[0]=11.6122;
metmast_TH[347].ws_60m[0]=11.5767;
metmast_TH[348].ws_60m[0]=11.1173;
metmast_TH[349].ws_60m[0]=11.1392;
metmast_TH[350].ws_60m[0]=10.8193;
metmast_TH[351].ws_60m[0]=10.6416;
metmast_TH[352].ws_60m[0]=10.6251;
metmast_TH[353].ws_60m[0]=10.4037;
metmast_TH[354].ws_60m[0]=10.2505;
metmast_TH[355].ws_60m[0]=10.081;
metmast_TH[356].ws_60m[0]=9.4959;
metmast_TH[357].ws_60m[0]=9.8513;
metmast_TH[358].ws_60m[0]=10.1166;
metmast_TH[359].ws_60m[0]=9.8185;
metmast_TH[360].ws_60m[0]=9.5779;
metmast_TH[361].ws_60m[0]=9.6408;
metmast_TH[362].ws_60m[0]=9.1732;
metmast_TH[363].ws_60m[0]=9.3236;
metmast_TH[364].ws_60m[0]=9.4986;
metmast_TH[365].ws_60m[0]=9.6353;
metmast_TH[366].ws_60m[0]=10.4146;
metmast_TH[367].ws_60m[0]=10.4064;
metmast_TH[368].ws_60m[0]=10.2615;
metmast_TH[369].ws_60m[0]=10.4857;
metmast_TH[370].ws_60m[0]=10.7509;
metmast_TH[371].ws_60m[0]=11.1994;
metmast_TH[372].ws_60m[0]=11.3306;
metmast_TH[373].ws_60m[0]=11.5193;
metmast_TH[374].ws_60m[0]=12.088;
metmast_TH[375].ws_60m[0]=12.2274;
metmast_TH[376].ws_60m[0]=12.3778;
metmast_TH[377].ws_60m[0]=11.8556;
metmast_TH[378].ws_60m[0]=12.2192;
metmast_TH[379].ws_60m[0]=11.6587;
metmast_TH[380].ws_60m[0]=11.6915;
metmast_TH[381].ws_60m[0]=11.2212;
metmast_TH[382].ws_60m[0]=11.2212;
metmast_TH[383].ws_60m[0]=11.4728;
metmast_TH[384].ws_60m[0]=11.522;
metmast_TH[385].ws_60m[0]=11.4755;
metmast_TH[386].ws_60m[0]=11.7927;
metmast_TH[387].ws_60m[0]=11.8228;
metmast_TH[388].ws_60m[0]=11.8173;
metmast_TH[389].ws_60m[0]=12.0743;
metmast_TH[390].ws_60m[0]=11.7626;
metmast_TH[391].ws_60m[0]=11.4017;
metmast_TH[392].ws_60m[0]=11.3962;
metmast_TH[393].ws_60m[0]=11.1911;
metmast_TH[394].ws_60m[0]=11.0736;
metmast_TH[395].ws_60m[0]=11.2458;
metmast_TH[396].ws_60m[0]=10.7837;
metmast_TH[397].ws_60m[0]=11.2868;
metmast_TH[398].ws_60m[0]=11.8529;
metmast_TH[399].ws_60m[0]=11.9267;
metmast_TH[400].ws_60m[0]=11.9759;
metmast_TH[401].ws_60m[0]=12.1564;
metmast_TH[402].ws_60m[0]=12.0689;
metmast_TH[403].ws_60m[0]=11.8255;
metmast_TH[404].ws_60m[0]=12.0032;
metmast_TH[405].ws_60m[0]=12.0798;
metmast_TH[406].ws_60m[0]=11.7982;
metmast_TH[407].ws_60m[0]=11.6751;
metmast_TH[408].ws_60m[0]=11.429;
metmast_TH[409].ws_60m[0]=11.4345;
metmast_TH[410].ws_60m[0]=11.4372;
metmast_TH[411].ws_60m[0]=11.0872;
metmast_TH[412].ws_60m[0]=10.9013;
metmast_TH[413].ws_60m[0]=10.8384;
metmast_TH[414].ws_60m[0]=10.513;
metmast_TH[415].ws_60m[0]=10.3654;
metmast_TH[416].ws_60m[0]=10.5978;
metmast_TH[417].ws_60m[0]=10.1412;
metmast_TH[418].ws_60m[0]=10.0756;
metmast_TH[419].ws_60m[0]=9.8267;
metmast_TH[420].ws_60m[0]=9.6025;
metmast_TH[421].ws_60m[0]=9.6135;
metmast_TH[422].ws_60m[0]=9.4221;
metmast_TH[423].ws_60m[0]=9.4303;
metmast_TH[424].ws_60m[0]=9.5396;
metmast_TH[425].ws_60m[0]=9.474;
metmast_TH[426].ws_60m[0]=9.5396;
metmast_TH[427].ws_60m[0]=9.6162;
metmast_TH[428].ws_60m[0]=9.5041;
metmast_TH[429].ws_60m[0]=9.6408;
metmast_TH[430].ws_60m[0]=9.4467;
metmast_TH[431].ws_60m[0]=9.7119;
metmast_TH[432].ws_60m[0]=9.4849;
metmast_TH[433].ws_60m[0]=9.4412;
metmast_TH[434].ws_60m[0]=9.4056;
metmast_TH[435].ws_60m[0]=9.3127;
metmast_TH[436].ws_60m[0]=9.4002;
metmast_TH[437].ws_60m[0]=9.7994;
metmast_TH[438].ws_60m[0]=9.6845;
metmast_TH[439].ws_60m[0]=9.6681;
metmast_TH[440].ws_60m[0]=9.6189;
metmast_TH[441].ws_60m[0]=9.5424;
metmast_TH[442].ws_60m[0]=9.7256;
metmast_TH[443].ws_60m[0]=10.0045;
metmast_TH[444].ws_60m[0]=9.906;
metmast_TH[445].ws_60m[0]=10.2533;
metmast_TH[446].ws_60m[0]=10.6142;
metmast_TH[447].ws_60m[0]=10.9697;
metmast_TH[448].ws_60m[0]=10.6689;
metmast_TH[449].ws_60m[0]=10.3627;
metmast_TH[450].ws_60m[0]=10.6087;
metmast_TH[451].ws_60m[0]=10.4255;
metmast_TH[452].ws_60m[0]=10.472;
metmast_TH[453].ws_60m[0]=10.5212;
metmast_TH[454].ws_60m[0]=10.3654;
metmast_TH[455].ws_60m[0]=10.2232;
metmast_TH[456].ws_60m[0]=10.0345;
metmast_TH[457].ws_60m[0]=10.3927;
metmast_TH[458].ws_60m[0]=10.0482;
metmast_TH[459].ws_60m[0]=10.04;
metmast_TH[460].ws_60m[0]=10.308;
metmast_TH[461].ws_60m[0]=10.3298;
metmast_TH[462].ws_60m[0]=10.7099;
metmast_TH[463].ws_60m[0]=10.5595;
metmast_TH[464].ws_60m[0]=10.1056;
metmast_TH[465].ws_60m[0]=10.3709;
metmast_TH[466].ws_60m[0]=10.6443;
metmast_TH[467].ws_60m[0]=10.7537;
metmast_TH[468].ws_60m[0]=10.4228;
metmast_TH[469].ws_60m[0]=10.2834;
metmast_TH[470].ws_60m[0]=10.1466;
metmast_TH[471].ws_60m[0]=10.349;
metmast_TH[472].ws_60m[0]=10.7318;
metmast_TH[473].ws_60m[0]=10.3982;
metmast_TH[474].ws_60m[0]=10.0154;
metmast_TH[475].ws_60m[0]=10.3462;
metmast_TH[476].ws_60m[0]=10.1849;
metmast_TH[477].ws_60m[0]=10.3353;
metmast_TH[478].ws_60m[0]=10.4775;
metmast_TH[479].ws_60m[0]=10.4064;
metmast_TH[480].ws_60m[0]=10.2697;
metmast_TH[481].ws_60m[0]=10.4064;
metmast_TH[482].ws_60m[0]=10.6716;
metmast_TH[483].ws_60m[0]=10.338;
metmast_TH[484].ws_60m[0]=10.4474;
metmast_TH[485].ws_60m[0]=10.4146;
metmast_TH[486].ws_60m[0]=10.3736;
metmast_TH[487].ws_60m[0]=10.174;
metmast_TH[488].ws_60m[0]=10.1111;
metmast_TH[489].ws_60m[0]=10.2095;
metmast_TH[490].ws_60m[0]=10.0509;
metmast_TH[491].ws_60m[0]=9.8595;
metmast_TH[492].ws_60m[0]=9.7338;
metmast_TH[493].ws_60m[0]=9.6736;
metmast_TH[494].ws_60m[0]=9.4549;
metmast_TH[495].ws_60m[0]=9.4549;
metmast_TH[496].ws_60m[0]=9.3318;
metmast_TH[497].ws_60m[0]=9.1131;
metmast_TH[498].ws_60m[0]=8.908;
metmast_TH[499].ws_60m[0]=8.7877;
metmast_TH[500].ws_60m[0]=8.5854;
metmast_TH[501].ws_60m[0]=8.9873;
metmast_TH[502].ws_60m[0]=8.8068;
metmast_TH[503].ws_60m[0]=9.2553;
metmast_TH[504].ws_60m[0]=9.2689;
metmast_TH[505].ws_60m[0]=9.2334;
metmast_TH[506].ws_60m[0]=10.0127;
metmast_TH[507].ws_60m[0]=9.6709;
metmast_TH[508].ws_60m[0]=9.7912;
metmast_TH[509].ws_60m[0]=9.8705;
metmast_TH[510].ws_60m[0]=9.8513;
metmast_TH[511].ws_60m[0]=9.8896;
metmast_TH[512].ws_60m[0]=10.0181;
metmast_TH[513].ws_60m[0]=10.4611;
metmast_TH[514].ws_60m[0]=10.4611;
metmast_TH[515].ws_60m[0]=10.4119;
metmast_TH[516].ws_60m[0]=10.7017;
metmast_TH[517].ws_60m[0]=10.4228;
metmast_TH[518].ws_60m[0]=10.3271;
metmast_TH[519].ws_60m[0]=9.8896;
metmast_TH[520].ws_60m[0]=10.1084;
metmast_TH[521].ws_60m[0]=10.0619;
metmast_TH[522].ws_60m[0]=10.0865;
metmast_TH[523].ws_60m[0]=10.1713;
metmast_TH[524].ws_60m[0]=10.2752;
metmast_TH[525].ws_60m[0]=10.3736;
metmast_TH[526].ws_60m[0]=10.3736;
metmast_TH[527].ws_60m[0]=9.8896;
metmast_TH[528].ws_60m[0]=10.3627;
metmast_TH[529].ws_60m[0]=10.3627;
metmast_TH[530].ws_60m[0]=10.1548;
metmast_TH[531].ws_60m[0]=10.2341;
metmast_TH[532].ws_60m[0]=10.2341;
metmast_TH[533].ws_60m[0]=10.4037;
metmast_TH[534].ws_60m[0]=10.565;
metmast_TH[535].ws_60m[0]=10.4584;
metmast_TH[536].ws_60m[0]=10.4064;
metmast_TH[537].ws_60m[0]=10.4146;
metmast_TH[538].ws_60m[0]=10.8685;
metmast_TH[539].ws_60m[0]=10.8685;
metmast_TH[540].ws_60m[0]=10.6169;
metmast_TH[541].ws_60m[0]=10.3873;
metmast_TH[542].ws_60m[0]=10.2752;
metmast_TH[543].ws_60m[0]=10.0427;
metmast_TH[544].ws_60m[0]=10.1357;
metmast_TH[545].ws_60m[0]=10.0865;
metmast_TH[546].ws_60m[0]=10.0373;
metmast_TH[547].ws_60m[0]=9.9033;
metmast_TH[548].ws_60m[0]=10.0181;
metmast_TH[549].ws_60m[0]=9.9115;
metmast_TH[550].ws_60m[0]=10.0701;
metmast_TH[551].ws_60m[0]=9.6791;
metmast_TH[552].ws_60m[0]=9.6791;
metmast_TH[553].ws_60m[0]=9.7037;
metmast_TH[554].ws_60m[0]=9.7037;
metmast_TH[555].ws_60m[0]=9.7365;
metmast_TH[556].ws_60m[0]=9.5888;
metmast_TH[557].ws_60m[0]=9.5888;
metmast_TH[558].ws_60m[0]=9.8377;
metmast_TH[559].ws_60m[0]=9.6681;
metmast_TH[560].ws_60m[0]=9.6681;
metmast_TH[561].ws_60m[0]=9.6681;
metmast_TH[562].ws_60m[0]=9.4767;
metmast_TH[563].ws_60m[0]=9.701;
metmast_TH[564].ws_60m[0]=9.701;
metmast_TH[565].ws_60m[0]=9.8869;
metmast_TH[566].ws_60m[0]=9.6928;
metmast_TH[567].ws_60m[0]=9.6928;
metmast_TH[568].ws_60m[0]=9.7392;
metmast_TH[569].ws_60m[0]=9.6928;
metmast_TH[570].ws_60m[0]=9.6928;
metmast_TH[571].ws_60m[0]=9.6928;
metmast_TH[572].ws_60m[0]=9.5396;
metmast_TH[573].ws_60m[0]=9.5396;
metmast_TH[574].ws_60m[0]=9.7502;
metmast_TH[575].ws_60m[0]=9.6545;
metmast_TH[576].ws_60m[0]=9.4357;
metmast_TH[577].ws_60m[0]=9.6736;
metmast_TH[578].ws_60m[0]=9.5096;
metmast_TH[579].ws_60m[0]=9.5834;
metmast_TH[580].ws_60m[0]=9.783;
metmast_TH[581].ws_60m[0]=9.9881;
metmast_TH[582].ws_60m[0]=9.5314;
metmast_TH[583].ws_60m[0]=9.5314;
metmast_TH[584].ws_60m[0]=9.5314;
metmast_TH[585].ws_60m[0]=9.5724;
metmast_TH[586].ws_60m[0]=9.5724;
metmast_TH[587].ws_60m[0]=9.5724;
metmast_TH[588].ws_60m[0]=9.5724;
metmast_TH[589].ws_60m[0]=9.5724;
metmast_TH[590].ws_60m[0]=9.999;
metmast_TH[591].ws_60m[0]=9.999;
metmast_TH[592].ws_60m[0]=9.999;
metmast_TH[593].ws_60m[0]=9.5478;
metmast_TH[594].ws_60m[0]=9.608;
metmast_TH[595].ws_60m[0]=9.608;
metmast_TH[596].ws_60m[0]=9.5861;
metmast_TH[597].ws_60m[0]=9.3428;
metmast_TH[598].ws_60m[0]=9.4822;
metmast_TH[599].ws_60m[0]=9.5642;
metmast_TH[600].ws_60m[0]=9.6107;
metmast_TH[601].ws_60m[0]=9.5752;
metmast_TH[602].ws_60m[0]=9.5752;
metmast_TH[603].ws_60m[0]=9.5916;
metmast_TH[604].ws_60m[0]=9.7775;
metmast_TH[605].ws_60m[0]=9.7775;
metmast_TH[606].ws_60m[0]=9.5424;
metmast_TH[607].ws_60m[0]=9.6545;
metmast_TH[608].ws_60m[0]=9.6408;
metmast_TH[609].ws_60m[0]=10.174;
metmast_TH[610].ws_60m[0]=10.2314;
metmast_TH[611].ws_60m[0]=10.2314;
metmast_TH[612].ws_60m[0]=10.3654;
metmast_TH[613].ws_60m[0]=10.6224;
metmast_TH[614].ws_60m[0]=10.6224;
metmast_TH[615].ws_60m[0]=10.3599;
metmast_TH[616].ws_60m[0]=10.2259;
metmast_TH[617].ws_60m[0]=10.1384;
metmast_TH[618].ws_60m[0]=10.1384;
metmast_TH[619].ws_60m[0]=10.0482;
metmast_TH[620].ws_60m[0]=10.0482;
metmast_TH[621].ws_60m[0]=10.3599;
metmast_TH[622].ws_60m[0]=10.6115;
metmast_TH[623].ws_60m[0]=10.4802;
metmast_TH[624].ws_60m[0]=10.4802;
metmast_TH[625].ws_60m[0]=10.5759;
metmast_TH[626].ws_60m[0]=10.0892;
metmast_TH[627].ws_60m[0]=10.0892;
metmast_TH[628].ws_60m[0]=9.6517;
metmast_TH[629].ws_60m[0]=9.3264;
metmast_TH[630].ws_60m[0]=9.3264;
metmast_TH[631].ws_60m[0]=9.2443;
metmast_TH[632].ws_60m[0]=9.3209;
metmast_TH[633].ws_60m[0]=9.3209;
metmast_TH[634].ws_60m[0]=9.3209;
metmast_TH[635].ws_60m[0]=9.2252;
metmast_TH[636].ws_60m[0]=9.2252;
metmast_TH[637].ws_60m[0]=9.3318;
metmast_TH[638].ws_60m[0]=9.2908;
metmast_TH[639].ws_60m[0]=9.2908;
metmast_TH[640].ws_60m[0]=9.1623;
metmast_TH[641].ws_60m[0]=8.9846;
metmast_TH[642].ws_60m[0]=8.9846;
metmast_TH[643].ws_60m[0]=8.6975;
metmast_TH[644].ws_60m[0]=8.6975;
metmast_TH[645].ws_60m[0]=8.6975;
metmast_TH[646].ws_60m[0]=8.5553;
metmast_TH[647].ws_60m[0]=8.5553;
metmast_TH[648].ws_60m[0]=8.5553;
metmast_TH[649].ws_60m[0]=8.8561;
metmast_TH[650].ws_60m[0]=9.1869;
metmast_TH[651].ws_60m[0]=9.8295;
metmast_TH[652].ws_60m[0]=9.8295;
metmast_TH[653].ws_60m[0]=9.7885;
metmast_TH[654].ws_60m[0]=9.9224;
metmast_TH[655].ws_60m[0]=9.9224;
metmast_TH[656].ws_60m[0]=9.701;
metmast_TH[657].ws_60m[0]=9.701;
metmast_TH[658].ws_60m[0]=9.6709;
metmast_TH[659].ws_60m[0]=9.6709;
metmast_TH[660].ws_60m[0]=9.6299;
metmast_TH[661].ws_60m[0]=9.6299;
metmast_TH[662].ws_60m[0]=9.6299;
metmast_TH[663].ws_60m[0]=9.2033;
metmast_TH[664].ws_60m[0]=9.2033;
metmast_TH[665].ws_60m[0]=9.2033;
metmast_TH[666].ws_60m[0]=9.2033;
metmast_TH[667].ws_60m[0]=9.2498;
metmast_TH[668].ws_60m[0]=9.1295;
metmast_TH[669].ws_60m[0]=9.1678;
metmast_TH[670].ws_60m[0]=8.785;
metmast_TH[671].ws_60m[0]=9.0666;
metmast_TH[672].ws_60m[0]=9.2006;
metmast_TH[673].ws_60m[0]=8.8369;
metmast_TH[674].ws_60m[0]=8.8369;
metmast_TH[675].ws_60m[0]=8.815;
metmast_TH[676].ws_60m[0]=8.9353;
metmast_TH[677].ws_60m[0]=8.7986;
metmast_TH[678].ws_60m[0]=8.7221;
metmast_TH[679].ws_60m[0]=8.8725;
metmast_TH[680].ws_60m[0]=9.031;
metmast_TH[681].ws_60m[0]=8.7713;
metmast_TH[682].ws_60m[0]=8.6291;
metmast_TH[683].ws_60m[0]=8.5143;
metmast_TH[684].ws_60m[0]=8.6127;
metmast_TH[685].ws_60m[0]=8.9217;
metmast_TH[686].ws_60m[0]=8.9135;
metmast_TH[687].ws_60m[0]=9.0529;
metmast_TH[688].ws_60m[0]=9.1787;
metmast_TH[689].ws_60m[0]=9.0584;
metmast_TH[690].ws_60m[0]=9.0775;
metmast_TH[691].ws_60m[0]=8.7986;
metmast_TH[692].ws_60m[0]=8.8834;
metmast_TH[693].ws_60m[0]=8.7713;
metmast_TH[694].ws_60m[0]=8.6455;
metmast_TH[695].ws_60m[0]=8.6373;
metmast_TH[696].ws_60m[0]=8.1944;
metmast_TH[697].ws_60m[0]=8.2736;
metmast_TH[698].ws_60m[0]=8.2682;
metmast_TH[699].ws_60m[0]=8.3092;
metmast_TH[700].ws_60m[0]=8.4076;
metmast_TH[701].ws_60m[0]=8.6072;
metmast_TH[702].ws_60m[0]=8.5498;
metmast_TH[703].ws_60m[0]=8.517;
metmast_TH[704].ws_60m[0]=8.4951;
metmast_TH[705].ws_60m[0]=8.517;
metmast_TH[706].ws_60m[0]=8.785;
metmast_TH[707].ws_60m[0]=8.7193;
metmast_TH[708].ws_60m[0]=8.8369;
metmast_TH[709].ws_60m[0]=8.9764;
metmast_TH[710].ws_60m[0]=9.3701;
metmast_TH[711].ws_60m[0]=9.4521;
metmast_TH[712].ws_60m[0]=9.8513;
metmast_TH[713].ws_60m[0]=9.6681;
metmast_TH[714].ws_60m[0]=9.7611;
metmast_TH[715].ws_60m[0]=9.7611;
metmast_TH[716].ws_60m[0]=9.6463;
metmast_TH[717].ws_60m[0]=9.6463;
metmast_TH[718].ws_60m[0]=9.6463;
metmast_TH[719].ws_60m[0]=9.1814;
metmast_TH[720].ws_60m[0]=9.0748;
metmast_TH[721].ws_60m[0]=9.042;
metmast_TH[722].ws_60m[0]=8.9271;
metmast_TH[723].ws_60m[0]=8.9244;
metmast_TH[724].ws_60m[0]=8.7303;
metmast_TH[725].ws_60m[0]=8.7139;
metmast_TH[726].ws_60m[0]=8.6619;
metmast_TH[727].ws_60m[0]=8.5361;
metmast_TH[728].ws_60m[0]=8.476;
metmast_TH[729].ws_60m[0]=8.1069;
metmast_TH[730].ws_60m[0]=8.1178;
metmast_TH[731].ws_60m[0]=8.074;
metmast_TH[732].ws_60m[0]=7.9893;
metmast_TH[733].ws_60m[0]=7.7158;
metmast_TH[734].ws_60m[0]=7.8526;
metmast_TH[735].ws_60m[0]=8.3393;
metmast_TH[736].ws_60m[0]=7.9264;
metmast_TH[737].ws_60m[0]=7.9264;
metmast_TH[738].ws_60m[0]=7.8936;
metmast_TH[739].ws_60m[0]=7.7897;
metmast_TH[740].ws_60m[0]=8.2545;
metmast_TH[741].ws_60m[0]=8.2545;
metmast_TH[742].ws_60m[0]=8.8479;
metmast_TH[743].ws_60m[0]=9.0228;
metmast_TH[744].ws_60m[0]=9.1869;
metmast_TH[745].ws_60m[0]=8.8041;
metmast_TH[746].ws_60m[0]=9.1514;
metmast_TH[747].ws_60m[0]=9.206;
metmast_TH[748].ws_60m[0]=9.1732;
metmast_TH[749].ws_60m[0]=9.5451;
metmast_TH[750].ws_60m[0]=9.3346;
metmast_TH[751].ws_60m[0]=9.5588;
metmast_TH[752].ws_60m[0]=9.4986;
metmast_TH[753].ws_60m[0]=9.433;
metmast_TH[754].ws_60m[0]=9.0639;
metmast_TH[755].ws_60m[0]=9.0639;
metmast_TH[756].ws_60m[0]=8.9846;
metmast_TH[757].ws_60m[0]=9.4303;
metmast_TH[758].ws_60m[0]=9.4303;
metmast_TH[759].ws_60m[0]=9.2471;
metmast_TH[760].ws_60m[0]=9.6217;
metmast_TH[761].ws_60m[0]=9.6681;
metmast_TH[762].ws_60m[0]=9.5697;
metmast_TH[763].ws_60m[0]=9.6353;
metmast_TH[764].ws_60m[0]=10.338;
metmast_TH[765].ws_60m[0]=10.338;
metmast_TH[766].ws_60m[0]=10.1193;
metmast_TH[767].ws_60m[0]=9.9853;
metmast_TH[768].ws_60m[0]=9.9443;
metmast_TH[769].ws_60m[0]=10.1084;
metmast_TH[770].ws_60m[0]=10.2615;
metmast_TH[771].ws_60m[0]=10.4748;
metmast_TH[772].ws_60m[0]=10.6087;
metmast_TH[773].ws_60m[0]=10.6087;
metmast_TH[774].ws_60m[0]=10.4337;
metmast_TH[775].ws_60m[0]=10.0892;
metmast_TH[776].ws_60m[0]=10.1713;
metmast_TH[777].ws_60m[0]=10.2752;
metmast_TH[778].ws_60m[0]=10.1631;
metmast_TH[779].ws_60m[0]=9.9771;
metmast_TH[780].ws_60m[0]=9.5861;
metmast_TH[781].ws_60m[0]=9.6381;
metmast_TH[782].ws_60m[0]=9.3318;
metmast_TH[783].ws_60m[0]=9.6736;
metmast_TH[784].ws_60m[0]=9.206;
metmast_TH[785].ws_60m[0]=9.5424;
metmast_TH[786].ws_60m[0]=9.5369;
metmast_TH[787].ws_60m[0]=9.515;
metmast_TH[788].ws_60m[0]=9.5998;
metmast_TH[789].ws_60m[0]=9.3564;
metmast_TH[790].ws_60m[0]=9.2744;
metmast_TH[791].ws_60m[0]=9.5533;
metmast_TH[792].ws_60m[0]=9.824;
metmast_TH[793].ws_60m[0]=9.433;
metmast_TH[794].ws_60m[0]=9.5615;
metmast_TH[795].ws_60m[0]=9.2471;
metmast_TH[796].ws_60m[0]=9.2033;
metmast_TH[797].ws_60m[0]=9.2607;
metmast_TH[798].ws_60m[0]=9.526;
metmast_TH[799].ws_60m[0]=9.433;
metmast_TH[800].ws_60m[0]=9.3264;
metmast_TH[801].ws_60m[0]=9.351;
metmast_TH[802].ws_60m[0]=9.9306;
metmast_TH[803].ws_60m[0]=9.8787;
metmast_TH[804].ws_60m[0]=10.0728;
metmast_TH[805].ws_60m[0]=10.2177;
metmast_TH[806].ws_60m[0]=10.0318;
metmast_TH[807].ws_60m[0]=10.0947;
metmast_TH[808].ws_60m[0]=10.1384;
metmast_TH[809].ws_60m[0]=10.092;
metmast_TH[810].ws_60m[0]=10.1877;
metmast_TH[811].ws_60m[0]=9.8842;
metmast_TH[812].ws_60m[0]=9.906;
metmast_TH[813].ws_60m[0]=9.8705;
metmast_TH[814].ws_60m[0]=9.3974;
metmast_TH[815].ws_60m[0]=9.5451;
metmast_TH[816].ws_60m[0]=9.5916;
metmast_TH[817].ws_60m[0]=9.34;
metmast_TH[818].ws_60m[0]=9.3045;
metmast_TH[819].ws_60m[0]=9.2252;
metmast_TH[820].ws_60m[0]=9.0748;
metmast_TH[821].ws_60m[0]=8.9353;
metmast_TH[822].ws_60m[0]=8.6893;
metmast_TH[823].ws_60m[0]=8.6264;
metmast_TH[824].ws_60m[0]=8.394;
metmast_TH[825].ws_60m[0]=8.4404;
metmast_TH[826].ws_60m[0]=8.5936;
metmast_TH[827].ws_60m[0]=8.7932;
metmast_TH[828].ws_60m[0]=8.7111;
metmast_TH[829].ws_60m[0]=8.9135;
metmast_TH[830].ws_60m[0]=8.8287;
metmast_TH[831].ws_60m[0]=9.0283;
metmast_TH[832].ws_60m[0]=8.9135;
metmast_TH[833].ws_60m[0]=9.1377;
metmast_TH[834].ws_60m[0]=9.0146;
metmast_TH[835].ws_60m[0]=9.2225;
metmast_TH[836].ws_60m[0]=9.1978;
metmast_TH[837].ws_60m[0]=9.5178;
metmast_TH[838].ws_60m[0]=9.5013;
metmast_TH[839].ws_60m[0]=9.7502;
metmast_TH[840].ws_60m[0]=9.6244;
metmast_TH[841].ws_60m[0]=9.6107;
metmast_TH[842].ws_60m[0]=9.7502;
metmast_TH[843].ws_60m[0]=9.8021;
metmast_TH[844].ws_60m[0]=9.958;
metmast_TH[845].ws_60m[0]=9.9033;
metmast_TH[846].ws_60m[0]=9.7228;
metmast_TH[847].ws_60m[0]=10.092;
metmast_TH[848].ws_60m[0]=9.9744;
metmast_TH[849].ws_60m[0]=10.0509;
metmast_TH[850].ws_60m[0]=9.8158;
metmast_TH[851].ws_60m[0]=9.9334;
metmast_TH[852].ws_60m[0]=9.4849;
metmast_TH[853].ws_60m[0]=9.4904;
metmast_TH[854].ws_60m[0]=9.5615;
metmast_TH[855].ws_60m[0]=9.392;
metmast_TH[856].ws_60m[0]=9.4521;
metmast_TH[857].ws_60m[0]=9.433;
metmast_TH[858].ws_60m[0]=9.4904;
metmast_TH[859].ws_60m[0]=9.5533;
metmast_TH[860].ws_60m[0]=9.5478;
metmast_TH[861].ws_60m[0]=9.4385;
metmast_TH[862].ws_60m[0]=9.597;
metmast_TH[863].ws_60m[0]=9.3373;
metmast_TH[864].ws_60m[0]=9.3592;
metmast_TH[865].ws_60m[0]=9.0939;
metmast_TH[866].ws_60m[0]=8.8096;
metmast_TH[867].ws_60m[0]=9.2635;
metmast_TH[868].ws_60m[0]=9.0393;
metmast_TH[869].ws_60m[0]=9.3728;
metmast_TH[870].ws_60m[0]=9.1459;
metmast_TH[871].ws_60m[0]=9.4275;
metmast_TH[872].ws_60m[0]=9.4959;
metmast_TH[873].ws_60m[0]=9.3865;
metmast_TH[874].ws_60m[0]=9.2088;
metmast_TH[875].ws_60m[0]=9.5205;
metmast_TH[876].ws_60m[0]=9.3045;
metmast_TH[877].ws_60m[0]=9.2607;
metmast_TH[878].ws_60m[0]=8.9326;
metmast_TH[879].ws_60m[0]=9.1896;
metmast_TH[880].ws_60m[0]=8.7713;
metmast_TH[881].ws_60m[0]=8.8561;
metmast_TH[882].ws_60m[0]=8.4213;
metmast_TH[883].ws_60m[0]=8.5553;
metmast_TH[884].ws_60m[0]=8.3994;
metmast_TH[885].ws_60m[0]=8.5635;
metmast_TH[886].ws_60m[0]=8.4787;
metmast_TH[887].ws_60m[0]=8.5443;
metmast_TH[888].ws_60m[0]=8.3229;
metmast_TH[889].ws_60m[0]=8.4596;
metmast_TH[890].ws_60m[0]=8.6318;
metmast_TH[891].ws_60m[0]=8.949;
metmast_TH[892].ws_60m[0]=8.8232;
metmast_TH[893].ws_60m[0]=8.99;
metmast_TH[894].ws_60m[0]=8.5389;
metmast_TH[895].ws_60m[0]=8.5936;
metmast_TH[896].ws_60m[0]=8.3666;
metmast_TH[897].ws_60m[0]=8.1725;
metmast_TH[898].ws_60m[0]=8.0002;
metmast_TH[899].ws_60m[0]=7.8908;
metmast_TH[900].ws_60m[0]=8.0494;
metmast_TH[901].ws_60m[0]=8.0057;
metmast_TH[902].ws_60m[0]=8.0248;
metmast_TH[903].ws_60m[0]=8.2791;
metmast_TH[904].ws_60m[0]=8.1397;
metmast_TH[905].ws_60m[0]=8.4733;
metmast_TH[906].ws_60m[0]=8.4268;
metmast_TH[907].ws_60m[0]=8.1998;
metmast_TH[908].ws_60m[0]=8.2764;
metmast_TH[909].ws_60m[0]=7.7514;
metmast_TH[910].ws_60m[0]=7.8307;
metmast_TH[911].ws_60m[0]=7.5682;
metmast_TH[912].ws_60m[0]=7.5491;
metmast_TH[913].ws_60m[0]=7.4889;
metmast_TH[914].ws_60m[0]=7.6994;
metmast_TH[915].ws_60m[0]=7.6338;
metmast_TH[916].ws_60m[0]=7.7213;
metmast_TH[917].ws_60m[0]=7.8198;
metmast_TH[918].ws_60m[0]=7.7241;
metmast_TH[919].ws_60m[0]=7.8143;
metmast_TH[920].ws_60m[0]=7.7815;
metmast_TH[921].ws_60m[0]=7.7158;
metmast_TH[922].ws_60m[0]=7.5436;
metmast_TH[923].ws_60m[0]=7.6612;
metmast_TH[924].ws_60m[0]=7.8334;
metmast_TH[925].ws_60m[0]=7.5327;
metmast_TH[926].ws_60m[0]=7.6666;
metmast_TH[927].ws_60m[0]=7.6393;
metmast_TH[928].ws_60m[0]=7.5436;
metmast_TH[929].ws_60m[0]=7.6393;
metmast_TH[930].ws_60m[0]=7.7678;
metmast_TH[931].ws_60m[0]=7.7377;
metmast_TH[932].ws_60m[0]=7.601;
metmast_TH[933].ws_60m[0]=7.8416;
metmast_TH[934].ws_60m[0]=7.8006;
metmast_TH[935].ws_60m[0]=7.8334;
metmast_TH[936].ws_60m[0]=7.91;
metmast_TH[937].ws_60m[0]=7.7049;
metmast_TH[938].ws_60m[0]=7.4506;
metmast_TH[939].ws_60m[0]=7.6147;
metmast_TH[940].ws_60m[0]=7.8881;
metmast_TH[941].ws_60m[0]=7.4889;
metmast_TH[942].ws_60m[0]=7.6967;
metmast_TH[943].ws_60m[0]=7.5791;
metmast_TH[944].ws_60m[0]=7.5108;
metmast_TH[945].ws_60m[0]=7.4807;
metmast_TH[946].ws_60m[0]=7.6858;
metmast_TH[947].ws_60m[0]=7.5791;
metmast_TH[948].ws_60m[0]=7.3768;
metmast_TH[949].ws_60m[0]=7.5737;
metmast_TH[950].ws_60m[0]=7.5463;
metmast_TH[951].ws_60m[0]=7.601;
metmast_TH[952].ws_60m[0]=7.9729;
metmast_TH[953].ws_60m[0]=7.9127;
metmast_TH[954].ws_60m[0]=7.6639;
metmast_TH[955].ws_60m[0]=7.7951;
metmast_TH[956].ws_60m[0]=7.828;
metmast_TH[957].ws_60m[0]=7.6694;
metmast_TH[958].ws_60m[0]=8.0904;
metmast_TH[959].ws_60m[0]=8.0112;
metmast_TH[960].ws_60m[0]=8.1725;
metmast_TH[961].ws_60m[0]=8.4486;
metmast_TH[962].ws_60m[0]=8.5143;
metmast_TH[963].ws_60m[0]=8.8096;
metmast_TH[964].ws_60m[0]=8.6482;
metmast_TH[965].ws_60m[0]=8.8068;
metmast_TH[966].ws_60m[0]=8.8232;
metmast_TH[967].ws_60m[0]=8.9818;
metmast_TH[968].ws_60m[0]=8.8697;
metmast_TH[969].ws_60m[0]=8.6373;
metmast_TH[970].ws_60m[0]=8.3721;
metmast_TH[971].ws_60m[0]=8.5471;
metmast_TH[972].ws_60m[0]=8.4541;
metmast_TH[973].ws_60m[0]=8.7795;
metmast_TH[974].ws_60m[0]=8.8342;
metmast_TH[975].ws_60m[0]=9.2088;
metmast_TH[976].ws_60m[0]=9.2553;
metmast_TH[977].ws_60m[0]=9.1924;
metmast_TH[978].ws_60m[0]=9.392;
metmast_TH[979].ws_60m[0]=9.0365;
metmast_TH[980].ws_60m[0]=9.3209;
metmast_TH[981].ws_60m[0]=9.3182;
metmast_TH[982].ws_60m[0]=9.6135;
metmast_TH[983].ws_60m[0]=9.392;
metmast_TH[984].ws_60m[0]=9.0365;
metmast_TH[985].ws_60m[0]=9.3674;
metmast_TH[986].ws_60m[0]=9.1103;
metmast_TH[987].ws_60m[0]=9.3209;
metmast_TH[988].ws_60m[0]=9.2853;
metmast_TH[989].ws_60m[0]=9.2881;
metmast_TH[990].ws_60m[0]=9.1814;
metmast_TH[991].ws_60m[0]=9.4084;
metmast_TH[992].ws_60m[0]=9.3318;
metmast_TH[993].ws_60m[0]=9.556;
metmast_TH[994].ws_60m[0]=9.3455;
metmast_TH[995].ws_60m[0]=9.556;
metmast_TH[996].ws_60m[0]=9.5041;
metmast_TH[997].ws_60m[0]=9.2443;
metmast_TH[998].ws_60m[0]=9.2525;
metmast_TH[999].ws_60m[0]=8.9791;
metmast_TH[0].wd_110m[0]=237.4756;
metmast_TH[1].wd_110m[0]=237.08;
metmast_TH[2].wd_110m[0]=237.058;
metmast_TH[3].wd_110m[0]=237.0361;
metmast_TH[4].wd_110m[0]=237.0141;
metmast_TH[5].wd_110m[0]=237.3657;
metmast_TH[6].wd_110m[0]=238.4424;
metmast_TH[7].wd_110m[0]=238.5523;
metmast_TH[8].wd_110m[0]=238.5523;
metmast_TH[9].wd_110m[0]=238.8819;
metmast_TH[10].wd_110m[0]=241.321;
metmast_TH[11].wd_110m[0]=241.2331;
metmast_TH[12].wd_110m[0]=241.1672;
metmast_TH[13].wd_110m[0]=241.1892;
metmast_TH[14].wd_110m[0]=236.8383;
metmast_TH[15].wd_110m[0]=236.4867;
metmast_TH[16].wd_110m[0]=236.5746;
metmast_TH[17].wd_110m[0]=236.5307;
metmast_TH[18].wd_110m[0]=237.4316;
metmast_TH[19].wd_110m[0]=237.5415;
metmast_TH[20].wd_110m[0]=237.4756;
metmast_TH[21].wd_110m[0]=237.4756;
metmast_TH[22].wd_110m[0]=235.9813;
metmast_TH[23].wd_110m[0]=235.1463;
metmast_TH[24].wd_110m[0]=235.0804;
metmast_TH[25].wd_110m[0]=235.0804;
metmast_TH[26].wd_110m[0]=235.1463;
metmast_TH[27].wd_110m[0]=235.8275;
metmast_TH[28].wd_110m[0]=235.7836;
metmast_TH[29].wd_110m[0]=235.7616;
metmast_TH[30].wd_110m[0]=235.6957;
metmast_TH[31].wd_110m[0]=233.696;
metmast_TH[32].wd_110m[0]=233.7839;
metmast_TH[33].wd_110m[0]=233.7839;
metmast_TH[34].wd_110m[0]=233.7839;
metmast_TH[35].wd_110m[0]=235.6078;
metmast_TH[36].wd_110m[0]=235.7396;
metmast_TH[37].wd_110m[0]=235.7836;
metmast_TH[38].wd_110m[0]=235.7836;
metmast_TH[39].wd_110m[0]=236.3988;
metmast_TH[40].wd_110m[0]=236.4428;
metmast_TH[41].wd_110m[0]=236.4208;
metmast_TH[42].wd_110m[0]=236.4208;
metmast_TH[43].wd_110m[0]=236.3109;
metmast_TH[44].wd_110m[0]=236.3329;
metmast_TH[45].wd_110m[0]=236.3329;
metmast_TH[46].wd_110m[0]=236.3329;
metmast_TH[47].wd_110m[0]=237.2558;
metmast_TH[48].wd_110m[0]=238.4864;
metmast_TH[49].wd_110m[0]=238.5742;
metmast_TH[50].wd_110m[0]=238.5523;
metmast_TH[51].wd_110m[0]=238.5523;
metmast_TH[52].wd_110m[0]=238.0688;
metmast_TH[53].wd_110m[0]=238.0688;
metmast_TH[54].wd_110m[0]=238.0688;
metmast_TH[55].wd_110m[0]=238.0469;
metmast_TH[56].wd_110m[0]=236.1571;
metmast_TH[57].wd_110m[0]=236.1351;
metmast_TH[58].wd_110m[0]=236.1571;
metmast_TH[59].wd_110m[0]=236.1351;
metmast_TH[60].wd_110m[0]=233.3664;
metmast_TH[61].wd_110m[0]=233.0588;
metmast_TH[62].wd_110m[0]=233.1247;
metmast_TH[63].wd_110m[0]=233.1247;
metmast_TH[64].wd_110m[0]=235.0584;
metmast_TH[65].wd_110m[0]=234.9705;
metmast_TH[66].wd_110m[0]=234.9485;
metmast_TH[67].wd_110m[0]=234.9925;
metmast_TH[68].wd_110m[0]=236.7724;
metmast_TH[69].wd_110m[0]=236.8603;
metmast_TH[70].wd_110m[0]=236.9042;
metmast_TH[71].wd_110m[0]=236.9262;
metmast_TH[72].wd_110m[0]=236.9702;
metmast_TH[73].wd_110m[0]=238.7061;
metmast_TH[74].wd_110m[0]=238.6622;
metmast_TH[75].wd_110m[0]=238.6841;
metmast_TH[76].wd_110m[0]=238.6182;
metmast_TH[77].wd_110m[0]=237.0141;
metmast_TH[78].wd_110m[0]=236.8823;
metmast_TH[79].wd_110m[0]=236.8823;
metmast_TH[80].wd_110m[0]=236.8823;
metmast_TH[81].wd_110m[0]=237.3437;
metmast_TH[82].wd_110m[0]=237.2558;
metmast_TH[83].wd_110m[0]=237.2778;
metmast_TH[84].wd_110m[0]=237.2778;
metmast_TH[85].wd_110m[0]=238.3325;
metmast_TH[86].wd_110m[0]=238.5083;
metmast_TH[87].wd_110m[0]=238.5303;
metmast_TH[88].wd_110m[0]=238.5523;
metmast_TH[89].wd_110m[0]=237.5195;
metmast_TH[90].wd_110m[0]=237.124;
metmast_TH[91].wd_110m[0]=237.0141;
metmast_TH[92].wd_110m[0]=237.0361;
metmast_TH[93].wd_110m[0]=236.7944;
metmast_TH[94].wd_110m[0]=234.7288;
metmast_TH[95].wd_110m[0]=234.8387;
metmast_TH[96].wd_110m[0]=234.8606;
metmast_TH[97].wd_110m[0]=235.2562;
metmast_TH[98].wd_110m[0]=242.0242;
metmast_TH[99].wd_110m[0]=241.7825;
metmast_TH[100].wd_110m[0]=241.6067;
metmast_TH[101].wd_110m[0]=241.6946;
metmast_TH[102].wd_110m[0]=240.4201;
metmast_TH[103].wd_110m[0]=240.3541;
metmast_TH[104].wd_110m[0]=240.3102;
metmast_TH[105].wd_110m[0]=240.3322;
metmast_TH[106].wd_110m[0]=235.388;
metmast_TH[107].wd_110m[0]=235.0145;
metmast_TH[108].wd_110m[0]=235.1024;
metmast_TH[109].wd_110m[0]=235.1243;
metmast_TH[110].wd_110m[0]=235.2562;
metmast_TH[111].wd_110m[0]=235.388;
metmast_TH[112].wd_110m[0]=235.41;
metmast_TH[113].wd_110m[0]=235.41;
metmast_TH[114].wd_110m[0]=235.366;
metmast_TH[115].wd_110m[0]=234.0256;
metmast_TH[116].wd_110m[0]=234.0476;
metmast_TH[117].wd_110m[0]=234.0256;
metmast_TH[118].wd_110m[0]=234.0476;
metmast_TH[119].wd_110m[0]=233.718;
metmast_TH[120].wd_110m[0]=233.718;
metmast_TH[121].wd_110m[0]=233.7619;
metmast_TH[122].wd_110m[0]=233.718;
metmast_TH[123].wd_110m[0]=237.7832;
metmast_TH[124].wd_110m[0]=237.981;
metmast_TH[125].wd_110m[0]=237.981;
metmast_TH[126].wd_110m[0]=237.959;
metmast_TH[127].wd_110m[0]=234.9046;
metmast_TH[128].wd_110m[0]=234.9046;
metmast_TH[129].wd_110m[0]=234.9485;
metmast_TH[130].wd_110m[0]=234.9485;
metmast_TH[131].wd_110m[0]=235.0145;
metmast_TH[132].wd_110m[0]=235.0145;
metmast_TH[133].wd_110m[0]=234.9925;
metmast_TH[134].wd_110m[0]=234.9925;
metmast_TH[135].wd_110m[0]=234.9046;
metmast_TH[136].wd_110m[0]=234.6629;
metmast_TH[137].wd_110m[0]=234.6189;
metmast_TH[138].wd_110m[0]=234.6189;
metmast_TH[139].wd_110m[0]=234.6629;
metmast_TH[140].wd_110m[0]=235.2562;
metmast_TH[141].wd_110m[0]=235.2782;
metmast_TH[142].wd_110m[0]=235.2782;
metmast_TH[143].wd_110m[0]=235.2782;
metmast_TH[144].wd_110m[0]=235.1903;
metmast_TH[145].wd_110m[0]=235.1903;
metmast_TH[146].wd_110m[0]=235.1903;
metmast_TH[147].wd_110m[0]=235.1903;
metmast_TH[148].wd_110m[0]=234.8826;
metmast_TH[149].wd_110m[0]=234.9266;
metmast_TH[150].wd_110m[0]=234.9485;
metmast_TH[151].wd_110m[0]=234.9266;
metmast_TH[152].wd_110m[0]=230.8174;
metmast_TH[153].wd_110m[0]=230.6636;
metmast_TH[154].wd_110m[0]=230.5318;
metmast_TH[155].wd_110m[0]=230.5098;
metmast_TH[156].wd_110m[0]=231.2789;
metmast_TH[157].wd_110m[0]=234.3992;
metmast_TH[158].wd_110m[0]=234.4871;
metmast_TH[159].wd_110m[0]=234.575;
metmast_TH[160].wd_110m[0]=235.2342;
metmast_TH[161].wd_110m[0]=239.3653;
metmast_TH[162].wd_110m[0]=239.1016;
metmast_TH[163].wd_110m[0]=239.1676;
metmast_TH[164].wd_110m[0]=239.1456;
metmast_TH[165].wd_110m[0]=236.7504;
metmast_TH[166].wd_110m[0]=236.6186;
metmast_TH[167].wd_110m[0]=236.5526;
metmast_TH[168].wd_110m[0]=236.5307;
metmast_TH[169].wd_110m[0]=240.5519;
metmast_TH[170].wd_110m[0]=240.8376;
metmast_TH[171].wd_110m[0]=240.7717;
metmast_TH[172].wd_110m[0]=240.7497;
metmast_TH[173].wd_110m[0]=238.7061;
metmast_TH[174].wd_110m[0]=237.6733;
metmast_TH[175].wd_110m[0]=237.6294;
metmast_TH[176].wd_110m[0]=237.5854;
metmast_TH[177].wd_110m[0]=239.0137;
metmast_TH[178].wd_110m[0]=240.5519;
metmast_TH[179].wd_110m[0]=240.508;
metmast_TH[180].wd_110m[0]=240.5299;
metmast_TH[181].wd_110m[0]=240.4201;
metmast_TH[182].wd_110m[0]=237.4975;
metmast_TH[183].wd_110m[0]=237.6074;
metmast_TH[184].wd_110m[0]=237.6074;
metmast_TH[185].wd_110m[0]=237.6733;
metmast_TH[186].wd_110m[0]=239.2115;
metmast_TH[187].wd_110m[0]=239.3214;
metmast_TH[188].wd_110m[0]=239.2994;
metmast_TH[189].wd_110m[0]=239.3214;
metmast_TH[190].wd_110m[0]=236.7504;
metmast_TH[191].wd_110m[0]=237.1679;
metmast_TH[192].wd_110m[0]=237.102;
metmast_TH[193].wd_110m[0]=237.124;
metmast_TH[194].wd_110m[0]=236.7724;
metmast_TH[195].wd_110m[0]=236.4648;
metmast_TH[196].wd_110m[0]=236.4648;
metmast_TH[197].wd_110m[0]=236.4428;
metmast_TH[198].wd_110m[0]=236.5746;
metmast_TH[199].wd_110m[0]=244.0238;
metmast_TH[200].wd_110m[0]=243.848;
metmast_TH[201].wd_110m[0]=243.848;
metmast_TH[202].wd_110m[0]=243.848;
metmast_TH[203].wd_110m[0]=246.5069;
metmast_TH[204].wd_110m[0]=246.5288;
metmast_TH[205].wd_110m[0]=246.5508;
metmast_TH[206].wd_110m[0]=246.4409;
metmast_TH[207].wd_110m[0]=245.54;
metmast_TH[208].wd_110m[0]=245.4961;
metmast_TH[209].wd_110m[0]=245.4961;
metmast_TH[210].wd_110m[0]=245.518;
metmast_TH[211].wd_110m[0]=244.3534;
metmast_TH[212].wd_110m[0]=244.3754;
metmast_TH[213].wd_110m[0]=244.3754;
metmast_TH[214].wd_110m[0]=244.3754;
metmast_TH[215].wd_110m[0]=244.5951;
metmast_TH[216].wd_110m[0]=244.5951;
metmast_TH[217].wd_110m[0]=244.5732;
metmast_TH[218].wd_110m[0]=244.5732;
metmast_TH[219].wd_110m[0]=244.5512;
metmast_TH[220].wd_110m[0]=244.4193;
metmast_TH[221].wd_110m[0]=244.4413;
metmast_TH[222].wd_110m[0]=244.4193;
metmast_TH[223].wd_110m[0]=244.4413;
metmast_TH[224].wd_110m[0]=243.4525;
metmast_TH[225].wd_110m[0]=243.3866;
metmast_TH[226].wd_110m[0]=243.4305;
metmast_TH[227].wd_110m[0]=243.3866;
metmast_TH[228].wd_110m[0]=244.3754;
metmast_TH[229].wd_110m[0]=244.4633;
metmast_TH[230].wd_110m[0]=244.4633;
metmast_TH[231].wd_110m[0]=244.4853;
metmast_TH[232].wd_110m[0]=245.1665;
metmast_TH[233].wd_110m[0]=245.0566;
metmast_TH[234].wd_110m[0]=245.0566;
metmast_TH[235].wd_110m[0]=245.0566;
metmast_TH[236].wd_110m[0]=247.1441;
metmast_TH[237].wd_110m[0]=247.254;
metmast_TH[238].wd_110m[0]=247.276;
metmast_TH[239].wd_110m[0]=247.232;
metmast_TH[240].wd_110m[0]=247.21;
metmast_TH[241].wd_110m[0]=247.1881;
metmast_TH[242].wd_110m[0]=247.1881;
metmast_TH[243].wd_110m[0]=247.1661;
metmast_TH[244].wd_110m[0]=246.0015;
metmast_TH[245].wd_110m[0]=245.0786;
metmast_TH[246].wd_110m[0]=245.0346;
metmast_TH[247].wd_110m[0]=245.0346;
metmast_TH[248].wd_110m[0]=245.0346;
metmast_TH[249].wd_110m[0]=245.0566;
metmast_TH[250].wd_110m[0]=245.0786;
metmast_TH[251].wd_110m[0]=245.0566;
metmast_TH[252].wd_110m[0]=245.0566;
metmast_TH[253].wd_110m[0]=246.8145;
metmast_TH[254].wd_110m[0]=246.9244;
metmast_TH[255].wd_110m[0]=246.9244;
metmast_TH[256].wd_110m[0]=246.9463;
metmast_TH[257].wd_110m[0]=246.7925;
metmast_TH[258].wd_110m[0]=246.8365;
metmast_TH[259].wd_110m[0]=246.8145;
metmast_TH[260].wd_110m[0]=246.8145;
metmast_TH[261].wd_110m[0]=246.2432;
metmast_TH[262].wd_110m[0]=245.4961;
metmast_TH[263].wd_110m[0]=245.4741;
metmast_TH[264].wd_110m[0]=245.4521;
metmast_TH[265].wd_110m[0]=245.6279;
metmast_TH[266].wd_110m[0]=246.1333;
metmast_TH[267].wd_110m[0]=246.1553;
metmast_TH[268].wd_110m[0]=246.1333;
metmast_TH[269].wd_110m[0]=245.1884;
metmast_TH[270].wd_110m[0]=242.2439;
metmast_TH[271].wd_110m[0]=242.2659;
metmast_TH[272].wd_110m[0]=242.2659;
metmast_TH[273].wd_110m[0]=242.3098;
metmast_TH[274].wd_110m[0]=245.518;
metmast_TH[275].wd_110m[0]=245.584;
metmast_TH[276].wd_110m[0]=245.6499;
metmast_TH[277].wd_110m[0]=245.6499;
metmast_TH[278].wd_110m[0]=243.7821;
metmast_TH[279].wd_110m[0]=243.6503;
metmast_TH[280].wd_110m[0]=243.6503;
metmast_TH[281].wd_110m[0]=243.6722;
metmast_TH[282].wd_110m[0]=243.6942;
metmast_TH[283].wd_110m[0]=240.9914;
metmast_TH[284].wd_110m[0]=240.9474;
metmast_TH[285].wd_110m[0]=240.9255;
metmast_TH[286].wd_110m[0]=240.9694;
metmast_TH[287].wd_110m[0]=239.9147;
metmast_TH[288].wd_110m[0]=240.0026;
metmast_TH[289].wd_110m[0]=239.9586;
metmast_TH[290].wd_110m[0]=239.9806;
metmast_TH[291].wd_110m[0]=242.178;
metmast_TH[292].wd_110m[0]=242.2439;
metmast_TH[293].wd_110m[0]=242.2659;
metmast_TH[294].wd_110m[0]=242.3098;
metmast_TH[295].wd_110m[0]=243.2327;
metmast_TH[296].wd_110m[0]=243.2767;
metmast_TH[297].wd_110m[0]=243.3206;
metmast_TH[298].wd_110m[0]=243.2987;
metmast_TH[299].wd_110m[0]=242.9691;
metmast_TH[300].wd_110m[0]=243.013;
metmast_TH[301].wd_110m[0]=243.013;
metmast_TH[302].wd_110m[0]=243.035;
metmast_TH[303].wd_110m[0]=240.7277;
metmast_TH[304].wd_110m[0]=240.9255;
metmast_TH[305].wd_110m[0]=240.9694;
metmast_TH[306].wd_110m[0]=241.0134;
metmast_TH[307].wd_110m[0]=238.4424;
metmast_TH[308].wd_110m[0]=237.937;
metmast_TH[309].wd_110m[0]=237.8271;
metmast_TH[310].wd_110m[0]=237.7832;
metmast_TH[311].wd_110m[0]=237.7612;
metmast_TH[312].wd_110m[0]=237.6294;
metmast_TH[313].wd_110m[0]=237.6294;
metmast_TH[314].wd_110m[0]=237.6294;
metmast_TH[315].wd_110m[0]=237.6074;
metmast_TH[316].wd_110m[0]=237.1679;
metmast_TH[317].wd_110m[0]=237.1459;
metmast_TH[318].wd_110m[0]=237.1459;
metmast_TH[319].wd_110m[0]=237.1459;
metmast_TH[320].wd_110m[0]=235.7836;
metmast_TH[321].wd_110m[0]=235.2122;
metmast_TH[322].wd_110m[0]=235.2342;
metmast_TH[323].wd_110m[0]=235.2342;
metmast_TH[324].wd_110m[0]=232.5094;
metmast_TH[325].wd_110m[0]=232.5753;
metmast_TH[326].wd_110m[0]=232.5753;
metmast_TH[327].wd_110m[0]=232.5753;
metmast_TH[328].wd_110m[0]=233.0148;
metmast_TH[329].wd_110m[0]=238.4424;
metmast_TH[330].wd_110m[0]=238.5523;
metmast_TH[331].wd_110m[0]=238.5742;
metmast_TH[332].wd_110m[0]=238.4864;
metmast_TH[333].wd_110m[0]=238.2666;
metmast_TH[334].wd_110m[0]=238.2227;
metmast_TH[335].wd_110m[0]=238.2227;
metmast_TH[336].wd_110m[0]=238.1787;
metmast_TH[337].wd_110m[0]=235.9154;
metmast_TH[338].wd_110m[0]=235.9813;
metmast_TH[339].wd_110m[0]=236.0033;
metmast_TH[340].wd_110m[0]=235.9813;
metmast_TH[341].wd_110m[0]=242.156;
metmast_TH[342].wd_110m[0]=242.134;
metmast_TH[343].wd_110m[0]=242.134;
metmast_TH[344].wd_110m[0]=242.0901;
metmast_TH[345].wd_110m[0]=239.6949;
metmast_TH[346].wd_110m[0]=239.5851;
metmast_TH[347].wd_110m[0]=239.5411;
metmast_TH[348].wd_110m[0]=239.4972;
metmast_TH[349].wd_110m[0]=239.4972;
metmast_TH[350].wd_110m[0]=238.5523;
metmast_TH[351].wd_110m[0]=238.6182;
metmast_TH[352].wd_110m[0]=238.5962;
metmast_TH[353].wd_110m[0]=238.5083;
metmast_TH[354].wd_110m[0]=237.5634;
metmast_TH[355].wd_110m[0]=237.7173;
metmast_TH[356].wd_110m[0]=237.6953;
metmast_TH[357].wd_110m[0]=237.6953;
metmast_TH[358].wd_110m[0]=235.7176;
metmast_TH[359].wd_110m[0]=235.6297;
metmast_TH[360].wd_110m[0]=235.5858;
metmast_TH[361].wd_110m[0]=235.6078;
metmast_TH[362].wd_110m[0]=237.981;
metmast_TH[363].wd_110m[0]=238.1348;
metmast_TH[364].wd_110m[0]=238.1348;
metmast_TH[365].wd_110m[0]=238.1348;
metmast_TH[366].wd_110m[0]=234.575;
metmast_TH[367].wd_110m[0]=234.8387;
metmast_TH[368].wd_110m[0]=234.9046;
metmast_TH[369].wd_110m[0]=234.9266;
metmast_TH[370].wd_110m[0]=237.2998;
metmast_TH[371].wd_110m[0]=238.0249;
metmast_TH[372].wd_110m[0]=238.1128;
metmast_TH[373].wd_110m[0]=238.1568;
metmast_TH[374].wd_110m[0]=238.2446;
metmast_TH[375].wd_110m[0]=240.2443;
metmast_TH[376].wd_110m[0]=240.1124;
metmast_TH[377].wd_110m[0]=240.1564;
metmast_TH[378].wd_110m[0]=240.0905;
metmast_TH[379].wd_110m[0]=237.6294;
metmast_TH[380].wd_110m[0]=237.4536;
metmast_TH[381].wd_110m[0]=237.3877;
metmast_TH[382].wd_110m[0]=237.3657;
metmast_TH[383].wd_110m[0]=240.7717;
metmast_TH[384].wd_110m[0]=240.3102;
metmast_TH[385].wd_110m[0]=240.3102;
metmast_TH[386].wd_110m[0]=240.3541;
metmast_TH[387].wd_110m[0]=245.2324;
metmast_TH[388].wd_110m[0]=244.5951;
metmast_TH[389].wd_110m[0]=244.683;
metmast_TH[390].wd_110m[0]=244.6171;
metmast_TH[391].wd_110m[0]=243.035;
metmast_TH[392].wd_110m[0]=243.1668;
metmast_TH[393].wd_110m[0]=243.1888;
metmast_TH[394].wd_110m[0]=243.1888;
metmast_TH[395].wd_110m[0]=243.1668;
metmast_TH[396].wd_110m[0]=236.3549;
metmast_TH[397].wd_110m[0]=236.1351;
metmast_TH[398].wd_110m[0]=236.0912;
metmast_TH[399].wd_110m[0]=236.267;
metmast_TH[400].wd_110m[0]=241.3869;
metmast_TH[401].wd_110m[0]=241.321;
metmast_TH[402].wd_110m[0]=241.321;
metmast_TH[403].wd_110m[0]=241.299;
metmast_TH[404].wd_110m[0]=242.6395;
metmast_TH[405].wd_110m[0]=242.6175;
metmast_TH[406].wd_110m[0]=242.6175;
metmast_TH[407].wd_110m[0]=242.6395;
metmast_TH[408].wd_110m[0]=242.1121;
metmast_TH[409].wd_110m[0]=241.4968;
metmast_TH[410].wd_110m[0]=241.4968;
metmast_TH[411].wd_110m[0]=241.4968;
metmast_TH[412].wd_110m[0]=239.629;
metmast_TH[413].wd_110m[0]=239.651;
metmast_TH[414].wd_110m[0]=239.5851;
metmast_TH[415].wd_110m[0]=239.5631;
metmast_TH[416].wd_110m[0]=237.7392;
metmast_TH[417].wd_110m[0]=232.9489;
metmast_TH[418].wd_110m[0]=233.0588;
metmast_TH[419].wd_110m[0]=233.1027;
metmast_TH[420].wd_110m[0]=233.1247;
metmast_TH[421].wd_110m[0]=237.981;
metmast_TH[422].wd_110m[0]=238.1348;
metmast_TH[423].wd_110m[0]=238.1128;
metmast_TH[424].wd_110m[0]=238.8379;
metmast_TH[425].wd_110m[0]=243.013;
metmast_TH[426].wd_110m[0]=243.1229;
metmast_TH[427].wd_110m[0]=243.1668;
metmast_TH[428].wd_110m[0]=243.1449;
metmast_TH[429].wd_110m[0]=240.9694;
metmast_TH[430].wd_110m[0]=241.1013;
metmast_TH[431].wd_110m[0]=241.1452;
metmast_TH[432].wd_110m[0]=241.1672;
metmast_TH[433].wd_110m[0]=241.1232;
metmast_TH[434].wd_110m[0]=239.0797;
metmast_TH[435].wd_110m[0]=239.2115;
metmast_TH[436].wd_110m[0]=239.1676;
metmast_TH[437].wd_110m[0]=239.5411;
metmast_TH[438].wd_110m[0]=241.9583;
metmast_TH[439].wd_110m[0]=242.0242;
metmast_TH[440].wd_110m[0]=242.0681;
metmast_TH[441].wd_110m[0]=242.0681;
metmast_TH[442].wd_110m[0]=241.6946;
metmast_TH[443].wd_110m[0]=241.6946;
metmast_TH[444].wd_110m[0]=241.6726;
metmast_TH[445].wd_110m[0]=241.6726;
metmast_TH[446].wd_110m[0]=239.673;
metmast_TH[447].wd_110m[0]=239.8707;
metmast_TH[448].wd_110m[0]=239.8048;
metmast_TH[449].wd_110m[0]=239.8487;
metmast_TH[450].wd_110m[0]=239.8487;
metmast_TH[451].wd_110m[0]=239.8268;
metmast_TH[452].wd_110m[0]=239.8707;
metmast_TH[453].wd_110m[0]=239.8487;
metmast_TH[454].wd_110m[0]=238.1568;
metmast_TH[455].wd_110m[0]=237.937;
metmast_TH[456].wd_110m[0]=237.915;
metmast_TH[457].wd_110m[0]=237.8711;
metmast_TH[458].wd_110m[0]=237.937;
metmast_TH[459].wd_110m[0]=238.5962;
metmast_TH[460].wd_110m[0]=238.5083;
metmast_TH[461].wd_110m[0]=238.5303;
metmast_TH[462].wd_110m[0]=238.5303;
metmast_TH[463].wd_110m[0]=240.1564;
metmast_TH[464].wd_110m[0]=240.2663;
metmast_TH[465].wd_110m[0]=240.2663;
metmast_TH[466].wd_110m[0]=240.2882;
metmast_TH[467].wd_110m[0]=240.6618;
metmast_TH[468].wd_110m[0]=240.5959;
metmast_TH[469].wd_110m[0]=240.6178;
metmast_TH[470].wd_110m[0]=240.5959;
metmast_TH[471].wd_110m[0]=234.6189;
metmast_TH[472].wd_110m[0]=235.3221;
metmast_TH[473].wd_110m[0]=235.3441;
metmast_TH[474].wd_110m[0]=235.3441;
metmast_TH[475].wd_110m[0]=237.8711;
metmast_TH[476].wd_110m[0]=239.8268;
metmast_TH[477].wd_110m[0]=239.6949;
metmast_TH[478].wd_110m[0]=239.8268;
metmast_TH[479].wd_110m[0]=238.8819;
metmast_TH[480].wd_110m[0]=238.9039;
metmast_TH[481].wd_110m[0]=238.9258;
metmast_TH[482].wd_110m[0]=238.9039;
metmast_TH[483].wd_110m[0]=238.9039;
metmast_TH[484].wd_110m[0]=240.5739;
metmast_TH[485].wd_110m[0]=240.5739;
metmast_TH[486].wd_110m[0]=240.5959;
metmast_TH[487].wd_110m[0]=240.5519;
metmast_TH[488].wd_110m[0]=238.5303;
metmast_TH[489].wd_110m[0]=238.3985;
metmast_TH[490].wd_110m[0]=238.3985;
metmast_TH[491].wd_110m[0]=238.4204;
metmast_TH[492].wd_110m[0]=238.4864;
metmast_TH[493].wd_110m[0]=240.6838;
metmast_TH[494].wd_110m[0]=240.7936;
metmast_TH[495].wd_110m[0]=240.7936;
metmast_TH[496].wd_110m[0]=239.9806;
metmast_TH[497].wd_110m[0]=239.8707;
metmast_TH[498].wd_110m[0]=239.9147;
metmast_TH[499].wd_110m[0]=239.9147;
metmast_TH[500].wd_110m[0]=242.2439;
metmast_TH[501].wd_110m[0]=242.9471;
metmast_TH[502].wd_110m[0]=243.1668;
metmast_TH[503].wd_110m[0]=243.0569;
metmast_TH[504].wd_110m[0]=242.991;
metmast_TH[505].wd_110m[0]=242.2219;
metmast_TH[506].wd_110m[0]=242.2879;
metmast_TH[507].wd_110m[0]=242.3098;
metmast_TH[508].wd_110m[0]=242.3098;
metmast_TH[509].wd_110m[0]=238.9258;
metmast_TH[510].wd_110m[0]=239.0577;
metmast_TH[511].wd_110m[0]=239.0797;
metmast_TH[512].wd_110m[0]=239.0577;
metmast_TH[513].wd_110m[0]=238.1568;
metmast_TH[514].wd_110m[0]=238.1568;
metmast_TH[515].wd_110m[0]=238.0908;
metmast_TH[516].wd_110m[0]=238.0688;
metmast_TH[517].wd_110m[0]=238.0908;
metmast_TH[518].wd_110m[0]=239.7169;
metmast_TH[519].wd_110m[0]=239.7169;
metmast_TH[520].wd_110m[0]=239.673;
metmast_TH[521].wd_110m[0]=239.629;
metmast_TH[522].wd_110m[0]=239.0137;
metmast_TH[523].wd_110m[0]=238.9698;
metmast_TH[524].wd_110m[0]=238.9698;
metmast_TH[525].wd_110m[0]=238.9039;
metmast_TH[526].wd_110m[0]=238.9039;
metmast_TH[527].wd_110m[0]=237.4536;
metmast_TH[528].wd_110m[0]=237.4096;
metmast_TH[529].wd_110m[0]=237.4096;
metmast_TH[530].wd_110m[0]=239.1016;
metmast_TH[531].wd_110m[0]=238.8599;
metmast_TH[532].wd_110m[0]=238.8599;
metmast_TH[533].wd_110m[0]=238.8599;
metmast_TH[534].wd_110m[0]=241.1892;
metmast_TH[535].wd_110m[0]=241.321;
metmast_TH[536].wd_110m[0]=241.3869;
metmast_TH[537].wd_110m[0]=241.343;
metmast_TH[538].wd_110m[0]=239.3873;
metmast_TH[539].wd_110m[0]=239.3873;
metmast_TH[540].wd_110m[0]=239.3653;
metmast_TH[541].wd_110m[0]=239.3653;
metmast_TH[542].wd_110m[0]=238.0688;
metmast_TH[543].wd_110m[0]=238.0029;
metmast_TH[544].wd_110m[0]=238.1128;
metmast_TH[545].wd_110m[0]=238.1128;
metmast_TH[546].wd_110m[0]=238.0688;
metmast_TH[547].wd_110m[0]=240.3322;
metmast_TH[548].wd_110m[0]=240.2443;
metmast_TH[549].wd_110m[0]=240.2223;
metmast_TH[550].wd_110m[0]=240.2003;
metmast_TH[551].wd_110m[0]=238.5962;
metmast_TH[552].wd_110m[0]=238.5962;
metmast_TH[553].wd_110m[0]=238.6622;
metmast_TH[554].wd_110m[0]=238.6622;
metmast_TH[555].wd_110m[0]=238.6841;
metmast_TH[556].wd_110m[0]=239.4312;
metmast_TH[557].wd_110m[0]=239.4312;
metmast_TH[558].wd_110m[0]=239.4532;
metmast_TH[559].wd_110m[0]=238.9258;
metmast_TH[560].wd_110m[0]=238.9258;
metmast_TH[561].wd_110m[0]=238.9258;
metmast_TH[562].wd_110m[0]=238.9918;
metmast_TH[563].wd_110m[0]=238.1348;
metmast_TH[564].wd_110m[0]=238.1348;
metmast_TH[565].wd_110m[0]=238.0908;
metmast_TH[566].wd_110m[0]=238.1348;
metmast_TH[567].wd_110m[0]=238.1348;
metmast_TH[568].wd_110m[0]=239.9366;
metmast_TH[569].wd_110m[0]=239.8268;
metmast_TH[570].wd_110m[0]=239.8268;
metmast_TH[571].wd_110m[0]=239.8268;
metmast_TH[572].wd_110m[0]=237.6074;
metmast_TH[573].wd_110m[0]=237.6074;
metmast_TH[574].wd_110m[0]=237.7392;
metmast_TH[575].wd_110m[0]=237.7173;
metmast_TH[576].wd_110m[0]=237.2119;
metmast_TH[577].wd_110m[0]=237.1899;
metmast_TH[578].wd_110m[0]=237.1899;
metmast_TH[579].wd_110m[0]=237.1899;
metmast_TH[580].wd_110m[0]=237.1899;
metmast_TH[581].wd_110m[0]=237.2558;
metmast_TH[582].wd_110m[0]=237.2558;
metmast_TH[583].wd_110m[0]=237.2558;
metmast_TH[584].wd_110m[0]=237.2558;
metmast_TH[585].wd_110m[0]=235.8934;
metmast_TH[586].wd_110m[0]=235.8934;
metmast_TH[587].wd_110m[0]=235.8934;
metmast_TH[588].wd_110m[0]=235.8934;
metmast_TH[589].wd_110m[0]=235.8934;
metmast_TH[590].wd_110m[0]=235.432;
metmast_TH[591].wd_110m[0]=235.432;
metmast_TH[592].wd_110m[0]=235.432;
metmast_TH[593].wd_110m[0]=234.4871;
metmast_TH[594].wd_110m[0]=233.3225;
metmast_TH[595].wd_110m[0]=233.3225;
metmast_TH[596].wd_110m[0]=233.3884;
metmast_TH[597].wd_110m[0]=233.8279;
metmast_TH[598].wd_110m[0]=233.8498;
metmast_TH[599].wd_110m[0]=233.8059;
metmast_TH[600].wd_110m[0]=233.8059;
metmast_TH[601].wd_110m[0]=234.0476;
metmast_TH[602].wd_110m[0]=234.0476;
metmast_TH[603].wd_110m[0]=234.0476;
metmast_TH[604].wd_110m[0]=234.0696;
metmast_TH[605].wd_110m[0]=234.0696;
metmast_TH[606].wd_110m[0]=235.1683;
metmast_TH[607].wd_110m[0]=235.0804;
metmast_TH[608].wd_110m[0]=235.0804;
metmast_TH[609].wd_110m[0]=235.1024;
metmast_TH[610].wd_110m[0]=235.4539;
metmast_TH[611].wd_110m[0]=235.4539;
metmast_TH[612].wd_110m[0]=235.4979;
metmast_TH[613].wd_110m[0]=235.432;
metmast_TH[614].wd_110m[0]=235.432;
metmast_TH[615].wd_110m[0]=235.1463;
metmast_TH[616].wd_110m[0]=235.1903;
metmast_TH[617].wd_110m[0]=235.1903;
metmast_TH[618].wd_110m[0]=235.1903;
metmast_TH[619].wd_110m[0]=236.1351;
metmast_TH[620].wd_110m[0]=236.1351;
metmast_TH[621].wd_110m[0]=236.2011;
metmast_TH[622].wd_110m[0]=230.5977;
metmast_TH[623].wd_110m[0]=227.2796;
metmast_TH[624].wd_110m[0]=227.2796;
metmast_TH[625].wd_110m[0]=227.3236;
metmast_TH[626].wd_110m[0]=229.5649;
metmast_TH[627].wd_110m[0]=229.5649;
metmast_TH[628].wd_110m[0]=229.4111;
metmast_TH[629].wd_110m[0]=229.4331;
metmast_TH[630].wd_110m[0]=229.4331;
metmast_TH[631].wd_110m[0]=235.6297;
metmast_TH[632].wd_110m[0]=235.7176;
metmast_TH[633].wd_110m[0]=235.7176;
metmast_TH[634].wd_110m[0]=235.7176;
metmast_TH[635].wd_110m[0]=236.5307;
metmast_TH[636].wd_110m[0]=236.5307;
metmast_TH[637].wd_110m[0]=236.5526;
metmast_TH[638].wd_110m[0]=236.5307;
metmast_TH[639].wd_110m[0]=236.5307;
metmast_TH[640].wd_110m[0]=232.4435;
metmast_TH[641].wd_110m[0]=232.4435;
metmast_TH[642].wd_110m[0]=232.4435;
metmast_TH[643].wd_110m[0]=232.4435;
metmast_TH[644].wd_110m[0]=232.4435;
metmast_TH[645].wd_110m[0]=232.4435;
metmast_TH[646].wd_110m[0]=228.8837;
metmast_TH[647].wd_110m[0]=228.8837;
metmast_TH[648].wd_110m[0]=228.8837;
metmast_TH[649].wd_110m[0]=227.5653;
metmast_TH[650].wd_110m[0]=227.5873;
metmast_TH[651].wd_110m[0]=227.7411;
metmast_TH[652].wd_110m[0]=227.7411;
metmast_TH[653].wd_110m[0]=228.5102;
metmast_TH[654].wd_110m[0]=228.5102;
metmast_TH[655].wd_110m[0]=228.5102;
metmast_TH[656].wd_110m[0]=229.5429;
metmast_TH[657].wd_110m[0]=229.5429;
metmast_TH[658].wd_110m[0]=229.455;
metmast_TH[659].wd_110m[0]=229.455;
metmast_TH[660].wd_110m[0]=229.3891;
metmast_TH[661].wd_110m[0]=229.3891;
metmast_TH[662].wd_110m[0]=229.3891;
metmast_TH[663].wd_110m[0]=229.3671;
metmast_TH[664].wd_110m[0]=229.3671;
metmast_TH[665].wd_110m[0]=229.3671;
metmast_TH[666].wd_110m[0]=229.3671;
metmast_TH[667].wd_110m[0]=228.642;
metmast_TH[668].wd_110m[0]=228.664;
metmast_TH[669].wd_110m[0]=229.3452;
metmast_TH[670].wd_110m[0]=229.3452;
metmast_TH[671].wd_110m[0]=229.3012;
metmast_TH[672].wd_110m[0]=229.3232;
metmast_TH[673].wd_110m[0]=229.6748;
metmast_TH[674].wd_110m[0]=229.6748;
metmast_TH[675].wd_110m[0]=229.6308;
metmast_TH[676].wd_110m[0]=229.6089;
metmast_TH[677].wd_110m[0]=227.807;
metmast_TH[678].wd_110m[0]=228.0707;
metmast_TH[679].wd_110m[0]=228.0487;
metmast_TH[680].wd_110m[0]=228.0267;
metmast_TH[681].wd_110m[0]=228.0927;
metmast_TH[682].wd_110m[0]=228.0927;
metmast_TH[683].wd_110m[0]=228.0927;
metmast_TH[684].wd_110m[0]=228.0707;
metmast_TH[685].wd_110m[0]=225.0163;
metmast_TH[686].wd_110m[0]=224.8625;
metmast_TH[687].wd_110m[0]=224.9284;
metmast_TH[688].wd_110m[0]=224.9064;
metmast_TH[689].wd_110m[0]=230.5318;
metmast_TH[690].wd_110m[0]=229.6968;
metmast_TH[691].wd_110m[0]=229.8066;
metmast_TH[692].wd_110m[0]=229.8506;
metmast_TH[693].wd_110m[0]=229.8945;
metmast_TH[694].wd_110m[0]=231.213;
metmast_TH[695].wd_110m[0]=231.1031;
metmast_TH[696].wd_110m[0]=231.147;
metmast_TH[697].wd_110m[0]=231.169;
metmast_TH[698].wd_110m[0]=231.4107;
metmast_TH[699].wd_110m[0]=231.4107;
metmast_TH[700].wd_110m[0]=231.3888;
metmast_TH[701].wd_110m[0]=231.4107;
metmast_TH[702].wd_110m[0]=233.74;
metmast_TH[703].wd_110m[0]=233.7619;
metmast_TH[704].wd_110m[0]=233.7619;
metmast_TH[705].wd_110m[0]=233.7619;
metmast_TH[706].wd_110m[0]=233.696;
metmast_TH[707].wd_110m[0]=233.696;
metmast_TH[708].wd_110m[0]=233.696;
metmast_TH[709].wd_110m[0]=233.696;
metmast_TH[710].wd_110m[0]=228.8398;
metmast_TH[711].wd_110m[0]=228.9496;
metmast_TH[712].wd_110m[0]=228.9716;
metmast_TH[713].wd_110m[0]=228.8837;
metmast_TH[714].wd_110m[0]=228.8617;
metmast_TH[715].wd_110m[0]=228.8617;
metmast_TH[716].wd_110m[0]=229.9385;
metmast_TH[717].wd_110m[0]=229.9385;
metmast_TH[718].wd_110m[0]=229.9385;
metmast_TH[719].wd_110m[0]=231.8282;
metmast_TH[720].wd_110m[0]=231.8722;
metmast_TH[721].wd_110m[0]=231.8502;
metmast_TH[722].wd_110m[0]=231.8502;
metmast_TH[723].wd_110m[0]=232.2677;
metmast_TH[724].wd_110m[0]=232.2897;
metmast_TH[725].wd_110m[0]=232.2677;
metmast_TH[726].wd_110m[0]=232.2677;
metmast_TH[727].wd_110m[0]=234.2014;
metmast_TH[728].wd_110m[0]=234.575;
metmast_TH[729].wd_110m[0]=234.575;
metmast_TH[730].wd_110m[0]=234.553;
metmast_TH[731].wd_110m[0]=234.553;
metmast_TH[732].wd_110m[0]=235.3221;
metmast_TH[733].wd_110m[0]=235.2562;
metmast_TH[734].wd_110m[0]=235.2782;
metmast_TH[735].wd_110m[0]=235.6737;
metmast_TH[736].wd_110m[0]=235.8934;
metmast_TH[737].wd_110m[0]=235.8934;
metmast_TH[738].wd_110m[0]=235.9154;
metmast_TH[739].wd_110m[0]=235.8934;
metmast_TH[740].wd_110m[0]=229.6308;
metmast_TH[741].wd_110m[0]=229.6308;
metmast_TH[742].wd_110m[0]=230.2022;
metmast_TH[743].wd_110m[0]=230.378;
metmast_TH[744].wd_110m[0]=233.8498;
metmast_TH[745].wd_110m[0]=233.8938;
metmast_TH[746].wd_110m[0]=233.8718;
metmast_TH[747].wd_110m[0]=233.8718;
metmast_TH[748].wd_110m[0]=237.4316;
metmast_TH[749].wd_110m[0]=237.6294;
metmast_TH[750].wd_110m[0]=237.6953;
metmast_TH[751].wd_110m[0]=237.7173;
metmast_TH[752].wd_110m[0]=233.4323;
metmast_TH[753].wd_110m[0]=233.8938;
metmast_TH[754].wd_110m[0]=234.0476;
metmast_TH[755].wd_110m[0]=234.0476;
metmast_TH[756].wd_110m[0]=233.9597;
metmast_TH[757].wd_110m[0]=237.4096;
metmast_TH[758].wd_110m[0]=237.2558;
metmast_TH[759].wd_110m[0]=237.2778;
metmast_TH[760].wd_110m[0]=237.2338;
metmast_TH[761].wd_110m[0]=234.0476;
metmast_TH[762].wd_110m[0]=234.0256;
metmast_TH[763].wd_110m[0]=234.0476;
metmast_TH[764].wd_110m[0]=234.0256;
metmast_TH[765].wd_110m[0]=234.0256;
metmast_TH[766].wd_110m[0]=224.7087;
metmast_TH[767].wd_110m[0]=224.9724;
metmast_TH[768].wd_110m[0]=224.9943;
metmast_TH[769].wd_110m[0]=225.6755;
metmast_TH[770].wd_110m[0]=226.0051;
metmast_TH[771].wd_110m[0]=225.9832;
metmast_TH[772].wd_110m[0]=225.9832;
metmast_TH[773].wd_110m[0]=230.6197;
metmast_TH[774].wd_110m[0]=230.5098;
metmast_TH[775].wd_110m[0]=230.5318;
metmast_TH[776].wd_110m[0]=230.4439;
metmast_TH[777].wd_110m[0]=236.4208;
metmast_TH[778].wd_110m[0]=235.6297;
metmast_TH[779].wd_110m[0]=236.0253;
metmast_TH[780].wd_110m[0]=236.0912;
metmast_TH[781].wd_110m[0]=236.1351;
metmast_TH[782].wd_110m[0]=235.1243;
metmast_TH[783].wd_110m[0]=235.1463;
metmast_TH[784].wd_110m[0]=235.1463;
metmast_TH[785].wd_110m[0]=235.1463;
metmast_TH[786].wd_110m[0]=226.972;
metmast_TH[787].wd_110m[0]=226.093;
metmast_TH[788].wd_110m[0]=226.2468;
metmast_TH[789].wd_110m[0]=226.3128;
metmast_TH[790].wd_110m[0]=226.115;
metmast_TH[791].wd_110m[0]=226.137;
metmast_TH[792].wd_110m[0]=226.137;
metmast_TH[793].wd_110m[0]=226.115;
metmast_TH[794].wd_110m[0]=226.2468;
metmast_TH[795].wd_110m[0]=230.7076;
metmast_TH[796].wd_110m[0]=230.8834;
metmast_TH[797].wd_110m[0]=230.8614;
metmast_TH[798].wd_110m[0]=230.5098;
metmast_TH[799].wd_110m[0]=229.4111;
metmast_TH[800].wd_110m[0]=229.3012;
metmast_TH[801].wd_110m[0]=229.3671;
metmast_TH[802].wd_110m[0]=232.2238;
metmast_TH[803].wd_110m[0]=233.6301;
metmast_TH[804].wd_110m[0]=233.3884;
metmast_TH[805].wd_110m[0]=233.4543;
metmast_TH[806].wd_110m[0]=232.7511;
metmast_TH[807].wd_110m[0]=227.3236;
metmast_TH[808].wd_110m[0]=227.1698;
metmast_TH[809].wd_110m[0]=227.3236;
metmast_TH[810].wd_110m[0]=227.2576;
metmast_TH[811].wd_110m[0]=228.9277;
metmast_TH[812].wd_110m[0]=229.0595;
metmast_TH[813].wd_110m[0]=229.1035;
metmast_TH[814].wd_110m[0]=229.1254;
metmast_TH[815].wd_110m[0]=227.2576;
metmast_TH[816].wd_110m[0]=227.5433;
metmast_TH[817].wd_110m[0]=227.5213;
metmast_TH[818].wd_110m[0]=227.4994;
metmast_TH[819].wd_110m[0]=227.4774;
metmast_TH[820].wd_110m[0]=234.553;
metmast_TH[821].wd_110m[0]=234.1355;
metmast_TH[822].wd_110m[0]=234.2454;
metmast_TH[823].wd_110m[0]=234.1795;
metmast_TH[824].wd_110m[0]=234.8606;
metmast_TH[825].wd_110m[0]=234.8606;
metmast_TH[826].wd_110m[0]=234.8387;
metmast_TH[827].wd_110m[0]=234.8826;
metmast_TH[828].wd_110m[0]=235.9594;
metmast_TH[829].wd_110m[0]=235.9594;
metmast_TH[830].wd_110m[0]=235.9154;
metmast_TH[831].wd_110m[0]=235.9813;
metmast_TH[832].wd_110m[0]=236.7284;
metmast_TH[833].wd_110m[0]=236.7724;
metmast_TH[834].wd_110m[0]=236.7504;
metmast_TH[835].wd_110m[0]=236.7504;
metmast_TH[836].wd_110m[0]=226.5984;
metmast_TH[837].wd_110m[0]=228.0707;
metmast_TH[838].wd_110m[0]=227.8729;
metmast_TH[839].wd_110m[0]=227.9169;
metmast_TH[840].wd_110m[0]=237.3217;
metmast_TH[841].wd_110m[0]=236.5966;
metmast_TH[842].wd_110m[0]=236.5746;
metmast_TH[843].wd_110m[0]=236.5526;
metmast_TH[844].wd_110m[0]=235.5638;
metmast_TH[845].wd_110m[0]=232.7292;
metmast_TH[846].wd_110m[0]=232.3996;
metmast_TH[847].wd_110m[0]=232.4215;
metmast_TH[848].wd_110m[0]=232.3996;
metmast_TH[849].wd_110m[0]=233.6521;
metmast_TH[850].wd_110m[0]=233.6521;
metmast_TH[851].wd_110m[0]=233.6301;
metmast_TH[852].wd_110m[0]=233.6521;
metmast_TH[853].wd_110m[0]=233.1687;
metmast_TH[854].wd_110m[0]=233.1687;
metmast_TH[855].wd_110m[0]=233.1687;
metmast_TH[856].wd_110m[0]=233.1687;
metmast_TH[857].wd_110m[0]=224.6867;
metmast_TH[858].wd_110m[0]=224.9943;
metmast_TH[859].wd_110m[0]=224.9504;
metmast_TH[860].wd_110m[0]=224.9943;
metmast_TH[861].wd_110m[0]=225.1042;
metmast_TH[862].wd_110m[0]=225.1481;
metmast_TH[863].wd_110m[0]=225.1262;
metmast_TH[864].wd_110m[0]=225.1262;
metmast_TH[865].wd_110m[0]=225.6096;
metmast_TH[866].wd_110m[0]=225.6316;
metmast_TH[867].wd_110m[0]=225.6535;
metmast_TH[868].wd_110m[0]=225.6535;
metmast_TH[869].wd_110m[0]=225.7634;
metmast_TH[870].wd_110m[0]=226.6204;
metmast_TH[871].wd_110m[0]=226.6424;
metmast_TH[872].wd_110m[0]=226.6644;
metmast_TH[873].wd_110m[0]=226.6424;
metmast_TH[874].wd_110m[0]=229.0375;
metmast_TH[875].wd_110m[0]=229.2133;
metmast_TH[876].wd_110m[0]=229.2793;
metmast_TH[877].wd_110m[0]=229.2793;
metmast_TH[878].wd_110m[0]=230.8394;
metmast_TH[879].wd_110m[0]=230.6856;
metmast_TH[880].wd_110m[0]=230.6197;
metmast_TH[881].wd_110m[0]=230.6416;
metmast_TH[882].wd_110m[0]=226.972;
metmast_TH[883].wd_110m[0]=227.6092;
metmast_TH[884].wd_110m[0]=227.4115;
metmast_TH[885].wd_110m[0]=227.4554;
metmast_TH[886].wd_110m[0]=226.9061;
metmast_TH[887].wd_110m[0]=223.8517;
metmast_TH[888].wd_110m[0]=223.7418;
metmast_TH[889].wd_110m[0]=223.6979;
metmast_TH[890].wd_110m[0]=226.4446;
metmast_TH[891].wd_110m[0]=228.62;
metmast_TH[892].wd_110m[0]=228.642;
metmast_TH[893].wd_110m[0]=228.7299;
metmast_TH[894].wd_110m[0]=228.8178;
metmast_TH[895].wd_110m[0]=230.0923;
metmast_TH[896].wd_110m[0]=230.0483;
metmast_TH[897].wd_110m[0]=230.0703;
metmast_TH[898].wd_110m[0]=230.0044;
metmast_TH[899].wd_110m[0]=231.3448;
metmast_TH[900].wd_110m[0]=231.4327;
metmast_TH[901].wd_110m[0]=231.4327;
metmast_TH[902].wd_110m[0]=231.4767;
metmast_TH[903].wd_110m[0]=227.8509;
metmast_TH[904].wd_110m[0]=228.1586;
metmast_TH[905].wd_110m[0]=228.1806;
metmast_TH[906].wd_110m[0]=228.1366;
metmast_TH[907].wd_110m[0]=229.3452;
metmast_TH[908].wd_110m[0]=230.9053;
metmast_TH[909].wd_110m[0]=230.9273;
metmast_TH[910].wd_110m[0]=230.9493;
metmast_TH[911].wd_110m[0]=229.6308;
metmast_TH[912].wd_110m[0]=226.2029;
metmast_TH[913].wd_110m[0]=226.2908;
metmast_TH[914].wd_110m[0]=226.2908;
metmast_TH[915].wd_110m[0]=226.3347;
metmast_TH[916].wd_110m[0]=228.4223;
metmast_TH[917].wd_110m[0]=228.4223;
metmast_TH[918].wd_110m[0]=228.4442;
metmast_TH[919].wd_110m[0]=228.4223;
metmast_TH[920].wd_110m[0]=227.3455;
metmast_TH[921].wd_110m[0]=227.2796;
metmast_TH[922].wd_110m[0]=227.2576;
metmast_TH[923].wd_110m[0]=227.3016;
metmast_TH[924].wd_110m[0]=228.4223;
metmast_TH[925].wd_110m[0]=228.4882;
metmast_TH[926].wd_110m[0]=228.4662;
metmast_TH[927].wd_110m[0]=228.4442;
metmast_TH[928].wd_110m[0]=229.7847;
metmast_TH[929].wd_110m[0]=229.7627;
metmast_TH[930].wd_110m[0]=229.8066;
metmast_TH[931].wd_110m[0]=229.8286;
metmast_TH[932].wd_110m[0]=229.8286;
metmast_TH[933].wd_110m[0]=227.4115;
metmast_TH[934].wd_110m[0]=227.5213;
metmast_TH[935].wd_110m[0]=227.5653;
metmast_TH[936].wd_110m[0]=227.5653;
metmast_TH[937].wd_110m[0]=229.455;
metmast_TH[938].wd_110m[0]=229.5649;
metmast_TH[939].wd_110m[0]=229.5869;
metmast_TH[940].wd_110m[0]=229.5649;
metmast_TH[941].wd_110m[0]=229.1914;
metmast_TH[942].wd_110m[0]=229.1474;
metmast_TH[943].wd_110m[0]=229.1474;
metmast_TH[944].wd_110m[0]=229.1474;
metmast_TH[945].wd_110m[0]=223.7198;
metmast_TH[946].wd_110m[0]=224.0934;
metmast_TH[947].wd_110m[0]=224.1593;
metmast_TH[948].wd_110m[0]=224.2472;
metmast_TH[949].wd_110m[0]=231.3448;
metmast_TH[950].wd_110m[0]=230.7076;
metmast_TH[951].wd_110m[0]=230.5977;
metmast_TH[952].wd_110m[0]=230.5318;
metmast_TH[953].wd_110m[0]=230.2022;
metmast_TH[954].wd_110m[0]=228.9716;
metmast_TH[955].wd_110m[0]=228.9277;
metmast_TH[956].wd_110m[0]=228.9057;
metmast_TH[957].wd_110m[0]=228.7738;
metmast_TH[958].wd_110m[0]=226.928;
metmast_TH[959].wd_110m[0]=226.972;
metmast_TH[960].wd_110m[0]=227.0159;
metmast_TH[961].wd_110m[0]=227.0159;
metmast_TH[962].wd_110m[0]=226.0491;
metmast_TH[963].wd_110m[0]=226.0051;
metmast_TH[964].wd_110m[0]=225.9832;
metmast_TH[965].wd_110m[0]=225.9832;
metmast_TH[966].wd_110m[0]=226.8621;
metmast_TH[967].wd_110m[0]=226.7522;
metmast_TH[968].wd_110m[0]=226.7522;
metmast_TH[969].wd_110m[0]=226.7742;
metmast_TH[970].wd_110m[0]=225.5437;
metmast_TH[971].wd_110m[0]=225.6755;
metmast_TH[972].wd_110m[0]=225.6755;
metmast_TH[973].wd_110m[0]=225.6755;
metmast_TH[974].wd_110m[0]=225.1701;
metmast_TH[975].wd_110m[0]=224.6867;
metmast_TH[976].wd_110m[0]=224.6427;
metmast_TH[977].wd_110m[0]=224.6427;
metmast_TH[978].wd_110m[0]=224.3791;
metmast_TH[979].wd_110m[0]=223.8297;
metmast_TH[980].wd_110m[0]=223.8297;
metmast_TH[981].wd_110m[0]=223.8077;
metmast_TH[982].wd_110m[0]=223.8297;
metmast_TH[983].wd_110m[0]=224.2692;
metmast_TH[984].wd_110m[0]=224.2692;
metmast_TH[985].wd_110m[0]=224.2472;
metmast_TH[986].wd_110m[0]=224.2692;
metmast_TH[987].wd_110m[0]=225.4997;
metmast_TH[988].wd_110m[0]=225.3679;
metmast_TH[989].wd_110m[0]=225.3459;
metmast_TH[990].wd_110m[0]=225.3459;
metmast_TH[991].wd_110m[0]=225.9392;
metmast_TH[992].wd_110m[0]=225.9612;
metmast_TH[993].wd_110m[0]=225.9832;
metmast_TH[994].wd_110m[0]=225.9832;
metmast_TH[995].wd_110m[0]=225.9172;
metmast_TH[996].wd_110m[0]=225.5656;
metmast_TH[997].wd_110m[0]=225.5876;
metmast_TH[998].wd_110m[0]=225.5656;
metmast_TH[999].wd_110m[0]=225.8953;
metmast_TH[0].wd_60m[0]=227.3006;
metmast_TH[1].wd_60m[0]=223.7001;
metmast_TH[2].wd_60m[0]=220.8567;
metmast_TH[3].wd_60m[0]=225.9965;
metmast_TH[4].wd_60m[0]=238.8932;
metmast_TH[5].wd_60m[0]=245.0515;
metmast_TH[6].wd_60m[0]=241.7484;
metmast_TH[7].wd_60m[0]=243.0628;
metmast_TH[8].wd_60m[0]=243.2203;
metmast_TH[9].wd_60m[0]=247.7168;
metmast_TH[10].wd_60m[0]=242.1549;
metmast_TH[11].wd_60m[0]=235.5973;
metmast_TH[12].wd_60m[0]=235.9894;
metmast_TH[13].wd_60m[0]=227.471;
metmast_TH[14].wd_60m[0]=227.4174;
metmast_TH[15].wd_60m[0]=230.823;
metmast_TH[16].wd_60m[0]=231.08;
metmast_TH[17].wd_60m[0]=221.8914;
metmast_TH[18].wd_60m[0]=225.1764;
metmast_TH[19].wd_60m[0]=233.6655;
metmast_TH[20].wd_60m[0]=241.889;
metmast_TH[21].wd_60m[0]=237.6329;
metmast_TH[22].wd_60m[0]=231.2527;
metmast_TH[23].wd_60m[0]=235.3003;
metmast_TH[24].wd_60m[0]=237.268;
metmast_TH[25].wd_60m[0]=228.9091;
metmast_TH[26].wd_60m[0]=225.2244;
metmast_TH[27].wd_60m[0]=230.8988;
metmast_TH[28].wd_60m[0]=236.4372;
metmast_TH[29].wd_60m[0]=237.9808;
metmast_TH[30].wd_60m[0]=240.328;
metmast_TH[31].wd_60m[0]=237.8048;
metmast_TH[32].wd_60m[0]=232.8287;
metmast_TH[33].wd_60m[0]=229.8486;
metmast_TH[34].wd_60m[0]=231.1389;
metmast_TH[35].wd_60m[0]=233.3588;
metmast_TH[36].wd_60m[0]=225.7724;
metmast_TH[37].wd_60m[0]=229.6916;
metmast_TH[38].wd_60m[0]=227.2468;
metmast_TH[39].wd_60m[0]=230.137;
metmast_TH[40].wd_60m[0]=228.3507;
metmast_TH[41].wd_60m[0]=227.9623;
metmast_TH[42].wd_60m[0]=227.439;
metmast_TH[43].wd_60m[0]=231.2739;
metmast_TH[44].wd_60m[0]=234.7383;
metmast_TH[45].wd_60m[0]=239.8224;
metmast_TH[46].wd_60m[0]=246.4147;
metmast_TH[47].wd_60m[0]=245.3446;
metmast_TH[48].wd_60m[0]=249.0834;
metmast_TH[49].wd_60m[0]=247.7393;
metmast_TH[50].wd_60m[0]=251.9258;
metmast_TH[51].wd_60m[0]=245.16;
metmast_TH[52].wd_60m[0]=245.7529;
metmast_TH[53].wd_60m[0]=241.4731;
metmast_TH[54].wd_60m[0]=241.9025;
metmast_TH[55].wd_60m[0]=238.8083;
metmast_TH[56].wd_60m[0]=237.7593;
metmast_TH[57].wd_60m[0]=239.7466;
metmast_TH[58].wd_60m[0]=237.8256;
metmast_TH[59].wd_60m[0]=240.8486;
metmast_TH[60].wd_60m[0]=239.9222;
metmast_TH[61].wd_60m[0]=241.7783;
metmast_TH[62].wd_60m[0]=239.7938;
metmast_TH[63].wd_60m[0]=234.9978;
metmast_TH[64].wd_60m[0]=234.8116;
metmast_TH[65].wd_60m[0]=238.7438;
metmast_TH[66].wd_60m[0]=236.4226;
metmast_TH[67].wd_60m[0]=234.8123;
metmast_TH[68].wd_60m[0]=236.0821;
metmast_TH[69].wd_60m[0]=232.2983;
metmast_TH[70].wd_60m[0]=235.3728;
metmast_TH[71].wd_60m[0]=232.325;
metmast_TH[72].wd_60m[0]=232.1862;
metmast_TH[73].wd_60m[0]=231.3973;
metmast_TH[74].wd_60m[0]=233.3099;
metmast_TH[75].wd_60m[0]=236.3897;
metmast_TH[76].wd_60m[0]=235.8694;
metmast_TH[77].wd_60m[0]=235.9292;
metmast_TH[78].wd_60m[0]=236.8521;
metmast_TH[79].wd_60m[0]=237.0952;
metmast_TH[80].wd_60m[0]=239.3124;
metmast_TH[81].wd_60m[0]=241.6478;
metmast_TH[82].wd_60m[0]=242.1823;
metmast_TH[83].wd_60m[0]=238.516;
metmast_TH[84].wd_60m[0]=236.3094;
metmast_TH[85].wd_60m[0]=238.1084;
metmast_TH[86].wd_60m[0]=238.1678;
metmast_TH[87].wd_60m[0]=228.5212;
metmast_TH[88].wd_60m[0]=230.4321;
metmast_TH[89].wd_60m[0]=230.4324;
metmast_TH[90].wd_60m[0]=237.5852;
metmast_TH[91].wd_60m[0]=236.8993;
metmast_TH[92].wd_60m[0]=230.8081;
metmast_TH[93].wd_60m[0]=232.5499;
metmast_TH[94].wd_60m[0]=235.8081;
metmast_TH[95].wd_60m[0]=226.4154;
metmast_TH[96].wd_60m[0]=233.0684;
metmast_TH[97].wd_60m[0]=234.0626;
metmast_TH[98].wd_60m[0]=236.3344;
metmast_TH[99].wd_60m[0]=234.7593;
metmast_TH[100].wd_60m[0]=238.1388;
metmast_TH[101].wd_60m[0]=241.1666;
metmast_TH[102].wd_60m[0]=242.6744;
metmast_TH[103].wd_60m[0]=244.7632;
metmast_TH[104].wd_60m[0]=240.9706;
metmast_TH[105].wd_60m[0]=241.5089;
metmast_TH[106].wd_60m[0]=243.6767;
metmast_TH[107].wd_60m[0]=241.5693;
metmast_TH[108].wd_60m[0]=242.6603;
metmast_TH[109].wd_60m[0]=242.4015;
metmast_TH[110].wd_60m[0]=240.8045;
metmast_TH[111].wd_60m[0]=240.7359;
metmast_TH[112].wd_60m[0]=237.2036;
metmast_TH[113].wd_60m[0]=240.9709;
metmast_TH[114].wd_60m[0]=238.4039;
metmast_TH[115].wd_60m[0]=238.2589;
metmast_TH[116].wd_60m[0]=237.6766;
metmast_TH[117].wd_60m[0]=235.1702;
metmast_TH[118].wd_60m[0]=236.279;
metmast_TH[119].wd_60m[0]=240.4406;
metmast_TH[120].wd_60m[0]=237.8878;
metmast_TH[121].wd_60m[0]=235.4318;
metmast_TH[122].wd_60m[0]=231.505;
metmast_TH[123].wd_60m[0]=231.5652;
metmast_TH[124].wd_60m[0]=229.0065;
metmast_TH[125].wd_60m[0]=227.1625;
metmast_TH[126].wd_60m[0]=225.4455;
metmast_TH[127].wd_60m[0]=226.3379;
metmast_TH[128].wd_60m[0]=227.6669;
metmast_TH[129].wd_60m[0]=228.6453;
metmast_TH[130].wd_60m[0]=229.0571;
metmast_TH[131].wd_60m[0]=230.3848;
metmast_TH[132].wd_60m[0]=231.9422;
metmast_TH[133].wd_60m[0]=235.0408;
metmast_TH[134].wd_60m[0]=235.204;
metmast_TH[135].wd_60m[0]=235.3412;
metmast_TH[136].wd_60m[0]=236.8332;
metmast_TH[137].wd_60m[0]=236.2276;
metmast_TH[138].wd_60m[0]=238.9669;
metmast_TH[139].wd_60m[0]=244.3558;
metmast_TH[140].wd_60m[0]=248.493;
metmast_TH[141].wd_60m[0]=247.8088;
metmast_TH[142].wd_60m[0]=241.0192;
metmast_TH[143].wd_60m[0]=241.9027;
metmast_TH[144].wd_60m[0]=231.8973;
metmast_TH[145].wd_60m[0]=228.2136;
metmast_TH[146].wd_60m[0]=230.1824;
metmast_TH[147].wd_60m[0]=230.1219;
metmast_TH[148].wd_60m[0]=230.2446;
metmast_TH[149].wd_60m[0]=225.7755;
metmast_TH[150].wd_60m[0]=224.9181;
metmast_TH[151].wd_60m[0]=228.1512;
metmast_TH[152].wd_60m[0]=227.6167;
metmast_TH[153].wd_60m[0]=226.8523;
metmast_TH[154].wd_60m[0]=224.3805;
metmast_TH[155].wd_60m[0]=225.864;
metmast_TH[156].wd_60m[0]=220.8791;
metmast_TH[157].wd_60m[0]=219.2463;
metmast_TH[158].wd_60m[0]=222.6719;
metmast_TH[159].wd_60m[0]=226.1979;
metmast_TH[160].wd_60m[0]=224.7666;
metmast_TH[161].wd_60m[0]=225.7325;
metmast_TH[162].wd_60m[0]=222.014;
metmast_TH[163].wd_60m[0]=220.6113;
metmast_TH[164].wd_60m[0]=230.5902;
metmast_TH[165].wd_60m[0]=234.8849;
metmast_TH[166].wd_60m[0]=233.8236;
metmast_TH[167].wd_60m[0]=227.4749;
metmast_TH[168].wd_60m[0]=226.7823;
metmast_TH[169].wd_60m[0]=226.1227;
metmast_TH[170].wd_60m[0]=229.2177;
metmast_TH[171].wd_60m[0]=235.4798;
metmast_TH[172].wd_60m[0]=235.049;
metmast_TH[173].wd_60m[0]=234.4847;
metmast_TH[174].wd_60m[0]=235.5462;
metmast_TH[175].wd_60m[0]=235.579;
metmast_TH[176].wd_60m[0]=232.4198;
metmast_TH[177].wd_60m[0]=234.4358;
metmast_TH[178].wd_60m[0]=235.117;
metmast_TH[179].wd_60m[0]=240.1191;
metmast_TH[180].wd_60m[0]=238.3264;
metmast_TH[181].wd_60m[0]=235.1047;
metmast_TH[182].wd_60m[0]=232.7303;
metmast_TH[183].wd_60m[0]=230.2265;
metmast_TH[184].wd_60m[0]=232.3005;
metmast_TH[185].wd_60m[0]=237.4046;
metmast_TH[186].wd_60m[0]=241.2964;
metmast_TH[187].wd_60m[0]=246.1884;
metmast_TH[188].wd_60m[0]=244.8048;
metmast_TH[189].wd_60m[0]=236.2216;
metmast_TH[190].wd_60m[0]=236.672;
metmast_TH[191].wd_60m[0]=234.2055;
metmast_TH[192].wd_60m[0]=227.1098;
metmast_TH[193].wd_60m[0]=224.3711;
metmast_TH[194].wd_60m[0]=231.3395;
metmast_TH[195].wd_60m[0]=231.8461;
metmast_TH[196].wd_60m[0]=232.7091;
metmast_TH[197].wd_60m[0]=235.2797;
metmast_TH[198].wd_60m[0]=243.498;
metmast_TH[199].wd_60m[0]=244.7808;
metmast_TH[200].wd_60m[0]=234.5759;
metmast_TH[201].wd_60m[0]=235.7399;
metmast_TH[202].wd_60m[0]=246.4886;
metmast_TH[203].wd_60m[0]=245.9396;
metmast_TH[204].wd_60m[0]=256.2273;
metmast_TH[205].wd_60m[0]=246.4049;
metmast_TH[206].wd_60m[0]=226.2353;
metmast_TH[207].wd_60m[0]=231.671;
metmast_TH[208].wd_60m[0]=244.6267;
metmast_TH[209].wd_60m[0]=249.5653;
metmast_TH[210].wd_60m[0]=230.8107;
metmast_TH[211].wd_60m[0]=223.8155;
metmast_TH[212].wd_60m[0]=221.8378;
metmast_TH[213].wd_60m[0]=240.7949;
metmast_TH[214].wd_60m[0]=242.9913;
metmast_TH[215].wd_60m[0]=236.4222;
metmast_TH[216].wd_60m[0]=230.6681;
metmast_TH[217].wd_60m[0]=238.9024;
metmast_TH[218].wd_60m[0]=243.2045;
metmast_TH[219].wd_60m[0]=231.2142;
metmast_TH[220].wd_60m[0]=230.1343;
metmast_TH[221].wd_60m[0]=239.3327;
metmast_TH[222].wd_60m[0]=230.4649;
metmast_TH[223].wd_60m[0]=232.7624;
metmast_TH[224].wd_60m[0]=240.6744;
metmast_TH[225].wd_60m[0]=236.8324;
metmast_TH[226].wd_60m[0]=232.0078;
metmast_TH[227].wd_60m[0]=229.6584;
metmast_TH[228].wd_60m[0]=224.3966;
metmast_TH[229].wd_60m[0]=221.8584;
metmast_TH[230].wd_60m[0]=222.2228;
metmast_TH[231].wd_60m[0]=224.8298;
metmast_TH[232].wd_60m[0]=220.4344;
metmast_TH[233].wd_60m[0]=217.4649;
metmast_TH[234].wd_60m[0]=219.2382;
metmast_TH[235].wd_60m[0]=221.2274;
metmast_TH[236].wd_60m[0]=221.669;
metmast_TH[237].wd_60m[0]=223.2943;
metmast_TH[238].wd_60m[0]=223.8545;
metmast_TH[239].wd_60m[0]=226.0212;
metmast_TH[240].wd_60m[0]=227.511;
metmast_TH[241].wd_60m[0]=223.7473;
metmast_TH[242].wd_60m[0]=221.1512;
metmast_TH[243].wd_60m[0]=225.4365;
metmast_TH[244].wd_60m[0]=225.9275;
metmast_TH[245].wd_60m[0]=224.9978;
metmast_TH[246].wd_60m[0]=220.1026;
metmast_TH[247].wd_60m[0]=221.3735;
metmast_TH[248].wd_60m[0]=222.4111;
metmast_TH[249].wd_60m[0]=228.1651;
metmast_TH[250].wd_60m[0]=224.0668;
metmast_TH[251].wd_60m[0]=224.6553;
metmast_TH[252].wd_60m[0]=228.1122;
metmast_TH[253].wd_60m[0]=230.577;
metmast_TH[254].wd_60m[0]=233.5164;
metmast_TH[255].wd_60m[0]=232.8695;
metmast_TH[256].wd_60m[0]=244.3006;
metmast_TH[257].wd_60m[0]=245.1156;
metmast_TH[258].wd_60m[0]=242.0007;
metmast_TH[259].wd_60m[0]=235.7047;
metmast_TH[260].wd_60m[0]=235.0805;
metmast_TH[261].wd_60m[0]=238.6915;
metmast_TH[262].wd_60m[0]=237.9996;
metmast_TH[263].wd_60m[0]=242.5634;
metmast_TH[264].wd_60m[0]=240.2967;
metmast_TH[265].wd_60m[0]=240.1728;
metmast_TH[266].wd_60m[0]=242.293;
metmast_TH[267].wd_60m[0]=242.9815;
metmast_TH[268].wd_60m[0]=239.5216;
metmast_TH[269].wd_60m[0]=244.3747;
metmast_TH[270].wd_60m[0]=249.2186;
metmast_TH[271].wd_60m[0]=246.1994;
metmast_TH[272].wd_60m[0]=237.5346;
metmast_TH[273].wd_60m[0]=240.1808;
metmast_TH[274].wd_60m[0]=239.634;
metmast_TH[275].wd_60m[0]=238.3836;
metmast_TH[276].wd_60m[0]=239.3374;
metmast_TH[277].wd_60m[0]=238.313;
metmast_TH[278].wd_60m[0]=238.9695;
metmast_TH[279].wd_60m[0]=235.9959;
metmast_TH[280].wd_60m[0]=235.9524;
metmast_TH[281].wd_60m[0]=234.312;
metmast_TH[282].wd_60m[0]=235.0714;
metmast_TH[283].wd_60m[0]=235.2382;
metmast_TH[284].wd_60m[0]=233.5974;
metmast_TH[285].wd_60m[0]=233.7049;
metmast_TH[286].wd_60m[0]=233.0563;
metmast_TH[287].wd_60m[0]=233.4409;
metmast_TH[288].wd_60m[0]=229.1961;
metmast_TH[289].wd_60m[0]=229.6172;
metmast_TH[290].wd_60m[0]=230.1609;
metmast_TH[291].wd_60m[0]=232.4723;
metmast_TH[292].wd_60m[0]=229.261;
metmast_TH[293].wd_60m[0]=227.2187;
metmast_TH[294].wd_60m[0]=221.5506;
metmast_TH[295].wd_60m[0]=225.352;
metmast_TH[296].wd_60m[0]=228.9818;
metmast_TH[297].wd_60m[0]=232.131;
metmast_TH[298].wd_60m[0]=234.3141;
metmast_TH[299].wd_60m[0]=233.7544;
metmast_TH[300].wd_60m[0]=235.5391;
metmast_TH[301].wd_60m[0]=232.5426;
metmast_TH[302].wd_60m[0]=234.4959;
metmast_TH[303].wd_60m[0]=237.1594;
metmast_TH[304].wd_60m[0]=236.1287;
metmast_TH[305].wd_60m[0]=236.417;
metmast_TH[306].wd_60m[0]=242.1129;
metmast_TH[307].wd_60m[0]=241.6414;
metmast_TH[308].wd_60m[0]=235.9654;
metmast_TH[309].wd_60m[0]=238.2973;
metmast_TH[310].wd_60m[0]=238.746;
metmast_TH[311].wd_60m[0]=237.0833;
metmast_TH[312].wd_60m[0]=237.6803;
metmast_TH[313].wd_60m[0]=235.3631;
metmast_TH[314].wd_60m[0]=238.2363;
metmast_TH[315].wd_60m[0]=232.4315;
metmast_TH[316].wd_60m[0]=232.8161;
metmast_TH[317].wd_60m[0]=239.1581;
metmast_TH[318].wd_60m[0]=241.3198;
metmast_TH[319].wd_60m[0]=241.8242;
metmast_TH[320].wd_60m[0]=238.8373;
metmast_TH[321].wd_60m[0]=239.3391;
metmast_TH[322].wd_60m[0]=226.9268;
metmast_TH[323].wd_60m[0]=219.0633;
metmast_TH[324].wd_60m[0]=215.9054;
metmast_TH[325].wd_60m[0]=213.9521;
metmast_TH[326].wd_60m[0]=216.0949;
metmast_TH[327].wd_60m[0]=217.7542;
metmast_TH[328].wd_60m[0]=234.3288;
metmast_TH[329].wd_60m[0]=236.0748;
metmast_TH[330].wd_60m[0]=227.0284;
metmast_TH[331].wd_60m[0]=227.9694;
metmast_TH[332].wd_60m[0]=237.1495;
metmast_TH[333].wd_60m[0]=232.4736;
metmast_TH[334].wd_60m[0]=233.7481;
metmast_TH[335].wd_60m[0]=230.1028;
metmast_TH[336].wd_60m[0]=232.4139;
metmast_TH[337].wd_60m[0]=233.1788;
metmast_TH[338].wd_60m[0]=246.0025;
metmast_TH[339].wd_60m[0]=236.2947;
metmast_TH[340].wd_60m[0]=239.191;
metmast_TH[341].wd_60m[0]=243.6313;
metmast_TH[342].wd_60m[0]=241.6149;
metmast_TH[343].wd_60m[0]=231.3467;
metmast_TH[344].wd_60m[0]=230.4159;
metmast_TH[345].wd_60m[0]=235.4051;
metmast_TH[346].wd_60m[0]=238.6174;
metmast_TH[347].wd_60m[0]=237.9164;
metmast_TH[348].wd_60m[0]=234.0228;
metmast_TH[349].wd_60m[0]=236.0005;
metmast_TH[350].wd_60m[0]=240.644;
metmast_TH[351].wd_60m[0]=232.0419;
metmast_TH[352].wd_60m[0]=231.5631;
metmast_TH[353].wd_60m[0]=229.1259;
metmast_TH[354].wd_60m[0]=233.0089;
metmast_TH[355].wd_60m[0]=234.6606;
metmast_TH[356].wd_60m[0]=240.0898;
metmast_TH[357].wd_60m[0]=245.9063;
metmast_TH[358].wd_60m[0]=243.9289;
metmast_TH[359].wd_60m[0]=237.9073;
metmast_TH[360].wd_60m[0]=234.9717;
metmast_TH[361].wd_60m[0]=235.8233;
metmast_TH[362].wd_60m[0]=228.15;
metmast_TH[363].wd_60m[0]=225.766;
metmast_TH[364].wd_60m[0]=232.3585;
metmast_TH[365].wd_60m[0]=227.8832;
metmast_TH[366].wd_60m[0]=226.4957;
metmast_TH[367].wd_60m[0]=224.6082;
metmast_TH[368].wd_60m[0]=221.9427;
metmast_TH[369].wd_60m[0]=226.6215;
metmast_TH[370].wd_60m[0]=228.0735;
metmast_TH[371].wd_60m[0]=225.6338;
metmast_TH[372].wd_60m[0]=227.3962;
metmast_TH[373].wd_60m[0]=229.2267;
metmast_TH[374].wd_60m[0]=230.6283;
metmast_TH[375].wd_60m[0]=233.7801;
metmast_TH[376].wd_60m[0]=240.3092;
metmast_TH[377].wd_60m[0]=241.881;
metmast_TH[378].wd_60m[0]=235.9082;
metmast_TH[379].wd_60m[0]=235.7552;
metmast_TH[380].wd_60m[0]=236.0966;
metmast_TH[381].wd_60m[0]=235.666;
metmast_TH[382].wd_60m[0]=237.6326;
metmast_TH[383].wd_60m[0]=237.6291;
metmast_TH[384].wd_60m[0]=239.0879;
metmast_TH[385].wd_60m[0]=240.2016;
metmast_TH[386].wd_60m[0]=238.1016;
metmast_TH[387].wd_60m[0]=234.988;
metmast_TH[388].wd_60m[0]=235.608;
metmast_TH[389].wd_60m[0]=235.0898;
metmast_TH[390].wd_60m[0]=230.7875;
metmast_TH[391].wd_60m[0]=236.3902;
metmast_TH[392].wd_60m[0]=237.0622;
metmast_TH[393].wd_60m[0]=236.6227;
metmast_TH[394].wd_60m[0]=236.2174;
metmast_TH[395].wd_60m[0]=232.7073;
metmast_TH[396].wd_60m[0]=231.9638;
metmast_TH[397].wd_60m[0]=234.3992;
metmast_TH[398].wd_60m[0]=235.4422;
metmast_TH[399].wd_60m[0]=237.8286;
metmast_TH[400].wd_60m[0]=238.4369;
metmast_TH[401].wd_60m[0]=237.5557;
metmast_TH[402].wd_60m[0]=234.5154;
metmast_TH[403].wd_60m[0]=231.2118;
metmast_TH[404].wd_60m[0]=233.3855;
metmast_TH[405].wd_60m[0]=235.7253;
metmast_TH[406].wd_60m[0]=229.1873;
metmast_TH[407].wd_60m[0]=226.6309;
metmast_TH[408].wd_60m[0]=229.3786;
metmast_TH[409].wd_60m[0]=228.2815;
metmast_TH[410].wd_60m[0]=225.3294;
metmast_TH[411].wd_60m[0]=225.5267;
metmast_TH[412].wd_60m[0]=225.8805;
metmast_TH[413].wd_60m[0]=228.627;
metmast_TH[414].wd_60m[0]=227.8478;
metmast_TH[415].wd_60m[0]=232.6577;
metmast_TH[416].wd_60m[0]=232.6085;
metmast_TH[417].wd_60m[0]=233.4652;
metmast_TH[418].wd_60m[0]=233.9976;
metmast_TH[419].wd_60m[0]=231.9437;
metmast_TH[420].wd_60m[0]=229.257;
metmast_TH[421].wd_60m[0]=230.0263;
metmast_TH[422].wd_60m[0]=229.2324;
metmast_TH[423].wd_60m[0]=228.6765;
metmast_TH[424].wd_60m[0]=230.5514;
metmast_TH[425].wd_60m[0]=229.7534;
metmast_TH[426].wd_60m[0]=227.9789;
metmast_TH[427].wd_60m[0]=234.1814;
metmast_TH[428].wd_60m[0]=229.8838;
metmast_TH[429].wd_60m[0]=227.3409;
metmast_TH[430].wd_60m[0]=230.4006;
metmast_TH[431].wd_60m[0]=221.398;
metmast_TH[432].wd_60m[0]=222.8942;
metmast_TH[433].wd_60m[0]=218.7513;
metmast_TH[434].wd_60m[0]=219.2953;
metmast_TH[435].wd_60m[0]=218.4296;
metmast_TH[436].wd_60m[0]=219.8821;
metmast_TH[437].wd_60m[0]=227.6434;
metmast_TH[438].wd_60m[0]=227.1413;
metmast_TH[439].wd_60m[0]=237.7202;
metmast_TH[440].wd_60m[0]=243.2229;
metmast_TH[441].wd_60m[0]=241.8041;
metmast_TH[442].wd_60m[0]=238.0875;
metmast_TH[443].wd_60m[0]=243.661;
metmast_TH[444].wd_60m[0]=245.2207;
metmast_TH[445].wd_60m[0]=237.6028;
metmast_TH[446].wd_60m[0]=236.6885;
metmast_TH[447].wd_60m[0]=241.1895;
metmast_TH[448].wd_60m[0]=246.4038;
metmast_TH[449].wd_60m[0]=243.7884;
metmast_TH[450].wd_60m[0]=242.311;
metmast_TH[451].wd_60m[0]=240.3846;
metmast_TH[452].wd_60m[0]=240.8742;
metmast_TH[453].wd_60m[0]=236.1624;
metmast_TH[454].wd_60m[0]=236.3594;
metmast_TH[455].wd_60m[0]=232.3969;
metmast_TH[456].wd_60m[0]=228.1719;
metmast_TH[457].wd_60m[0]=231.3031;
metmast_TH[458].wd_60m[0]=228.0182;
metmast_TH[459].wd_60m[0]=233.9033;
metmast_TH[460].wd_60m[0]=233.7057;
metmast_TH[461].wd_60m[0]=231.0984;
metmast_TH[462].wd_60m[0]=229.4497;
metmast_TH[463].wd_60m[0]=235.5598;
metmast_TH[464].wd_60m[0]=240.7513;
metmast_TH[465].wd_60m[0]=243.6838;
metmast_TH[466].wd_60m[0]=236.8561;
metmast_TH[467].wd_60m[0]=231.7992;
metmast_TH[468].wd_60m[0]=230.227;
metmast_TH[469].wd_60m[0]=231.6346;
metmast_TH[470].wd_60m[0]=234.9694;
metmast_TH[471].wd_60m[0]=234.1579;
metmast_TH[472].wd_60m[0]=232.6674;
metmast_TH[473].wd_60m[0]=232.3004;
metmast_TH[474].wd_60m[0]=227.2062;
metmast_TH[475].wd_60m[0]=228.4901;
metmast_TH[476].wd_60m[0]=235.8761;
metmast_TH[477].wd_60m[0]=233.5885;
metmast_TH[478].wd_60m[0]=227.5584;
metmast_TH[479].wd_60m[0]=224.0059;
metmast_TH[480].wd_60m[0]=231.44;
metmast_TH[481].wd_60m[0]=228.3952;
metmast_TH[482].wd_60m[0]=228.6134;
metmast_TH[483].wd_60m[0]=229.5015;
metmast_TH[484].wd_60m[0]=228.9444;
metmast_TH[485].wd_60m[0]=224.5447;
metmast_TH[486].wd_60m[0]=223.0468;
metmast_TH[487].wd_60m[0]=220.9581;
metmast_TH[488].wd_60m[0]=222.1081;
metmast_TH[489].wd_60m[0]=227.5409;
metmast_TH[490].wd_60m[0]=229.1293;
metmast_TH[491].wd_60m[0]=228.4485;
metmast_TH[492].wd_60m[0]=230.9697;
metmast_TH[493].wd_60m[0]=229.4123;
metmast_TH[494].wd_60m[0]=227.2291;
metmast_TH[495].wd_60m[0]=228.8137;
metmast_TH[496].wd_60m[0]=230.7808;
metmast_TH[497].wd_60m[0]=231.8051;
metmast_TH[498].wd_60m[0]=237.8347;
metmast_TH[499].wd_60m[0]=241.9051;
metmast_TH[500].wd_60m[0]=245.113;
metmast_TH[501].wd_60m[0]=241.1071;
metmast_TH[502].wd_60m[0]=244.3763;
metmast_TH[503].wd_60m[0]=241.5504;
metmast_TH[504].wd_60m[0]=236.5683;
metmast_TH[505].wd_60m[0]=238.7717;
metmast_TH[506].wd_60m[0]=236.181;
metmast_TH[507].wd_60m[0]=233.8067;
metmast_TH[508].wd_60m[0]=234.8322;
metmast_TH[509].wd_60m[0]=229.1301;
metmast_TH[510].wd_60m[0]=226.3929;
metmast_TH[511].wd_60m[0]=227.0139;
metmast_TH[512].wd_60m[0]=230.6067;
metmast_TH[513].wd_60m[0]=236.0025;
metmast_TH[514].wd_60m[0]=233.0166;
metmast_TH[515].wd_60m[0]=234.8644;
metmast_TH[516].wd_60m[0]=233.5228;
metmast_TH[517].wd_60m[0]=236.9633;
metmast_TH[518].wd_60m[0]=236.5835;
metmast_TH[519].wd_60m[0]=233.1201;
metmast_TH[520].wd_60m[0]=229.0904;
metmast_TH[521].wd_60m[0]=227.7273;
metmast_TH[522].wd_60m[0]=227.4569;
metmast_TH[523].wd_60m[0]=230.4461;
metmast_TH[524].wd_60m[0]=231.6566;
metmast_TH[525].wd_60m[0]=231.8584;
metmast_TH[526].wd_60m[0]=234.8023;
metmast_TH[527].wd_60m[0]=229.9168;
metmast_TH[528].wd_60m[0]=233.791;
metmast_TH[529].wd_60m[0]=234.7576;
metmast_TH[530].wd_60m[0]=234.169;
metmast_TH[531].wd_60m[0]=233.6853;
metmast_TH[532].wd_60m[0]=229.7381;
metmast_TH[533].wd_60m[0]=226.5294;
metmast_TH[534].wd_60m[0]=230.5579;
metmast_TH[535].wd_60m[0]=229.9567;
metmast_TH[536].wd_60m[0]=233.1541;
metmast_TH[537].wd_60m[0]=243.6085;
metmast_TH[538].wd_60m[0]=243.4631;
metmast_TH[539].wd_60m[0]=242.4083;
metmast_TH[540].wd_60m[0]=241.3176;
metmast_TH[541].wd_60m[0]=243.7524;
metmast_TH[542].wd_60m[0]=244.3176;
metmast_TH[543].wd_60m[0]=246.0745;
metmast_TH[544].wd_60m[0]=238.7148;
metmast_TH[545].wd_60m[0]=238.8477;
metmast_TH[546].wd_60m[0]=241.8056;
metmast_TH[547].wd_60m[0]=239.042;
metmast_TH[548].wd_60m[0]=233.4844;
metmast_TH[549].wd_60m[0]=242.989;
metmast_TH[550].wd_60m[0]=242.7174;
metmast_TH[551].wd_60m[0]=246.3203;
metmast_TH[552].wd_60m[0]=249.2944;
metmast_TH[553].wd_60m[0]=245.0322;
metmast_TH[554].wd_60m[0]=238.9632;
metmast_TH[555].wd_60m[0]=238.506;
metmast_TH[556].wd_60m[0]=235.8952;
metmast_TH[557].wd_60m[0]=235.6797;
metmast_TH[558].wd_60m[0]=240.4268;
metmast_TH[559].wd_60m[0]=240.2319;
metmast_TH[560].wd_60m[0]=237.5501;
metmast_TH[561].wd_60m[0]=232.0262;
metmast_TH[562].wd_60m[0]=230.6791;
metmast_TH[563].wd_60m[0]=232.0192;
metmast_TH[564].wd_60m[0]=234.7136;
metmast_TH[565].wd_60m[0]=240.1839;
metmast_TH[566].wd_60m[0]=239.6145;
metmast_TH[567].wd_60m[0]=240.819;
metmast_TH[568].wd_60m[0]=239.1475;
metmast_TH[569].wd_60m[0]=236.8996;
metmast_TH[570].wd_60m[0]=242.5674;
metmast_TH[571].wd_60m[0]=239.4858;
metmast_TH[572].wd_60m[0]=239.5712;
metmast_TH[573].wd_60m[0]=238.4491;
metmast_TH[574].wd_60m[0]=237.5823;
metmast_TH[575].wd_60m[0]=233.9425;
metmast_TH[576].wd_60m[0]=239.3229;
metmast_TH[577].wd_60m[0]=237.6062;
metmast_TH[578].wd_60m[0]=236.187;
metmast_TH[579].wd_60m[0]=233.777;
metmast_TH[580].wd_60m[0]=235.0558;
metmast_TH[581].wd_60m[0]=238.1672;
metmast_TH[582].wd_60m[0]=231.3891;
metmast_TH[583].wd_60m[0]=235.1545;
metmast_TH[584].wd_60m[0]=236.5945;
metmast_TH[585].wd_60m[0]=228.8836;
metmast_TH[586].wd_60m[0]=228.0872;
metmast_TH[587].wd_60m[0]=235.2249;
metmast_TH[588].wd_60m[0]=239.4653;
metmast_TH[589].wd_60m[0]=238.872;
metmast_TH[590].wd_60m[0]=238.6535;
metmast_TH[591].wd_60m[0]=236.0643;
metmast_TH[592].wd_60m[0]=230.6064;
metmast_TH[593].wd_60m[0]=230.0699;
metmast_TH[594].wd_60m[0]=227.8401;
metmast_TH[595].wd_60m[0]=228.0344;
metmast_TH[596].wd_60m[0]=230.6903;
metmast_TH[597].wd_60m[0]=236.1372;
metmast_TH[598].wd_60m[0]=233.3565;
metmast_TH[599].wd_60m[0]=233.2132;
metmast_TH[600].wd_60m[0]=231.9888;
metmast_TH[601].wd_60m[0]=233.477;
metmast_TH[602].wd_60m[0]=230.8809;
metmast_TH[603].wd_60m[0]=225.9921;
metmast_TH[604].wd_60m[0]=226.6693;
metmast_TH[605].wd_60m[0]=227.1148;
metmast_TH[606].wd_60m[0]=235.3271;
metmast_TH[607].wd_60m[0]=234.6807;
metmast_TH[608].wd_60m[0]=233.4941;
metmast_TH[609].wd_60m[0]=227.9835;
metmast_TH[610].wd_60m[0]=231.4637;
metmast_TH[611].wd_60m[0]=230.1024;
metmast_TH[612].wd_60m[0]=228.0458;
metmast_TH[613].wd_60m[0]=228.2782;
metmast_TH[614].wd_60m[0]=224.7493;
metmast_TH[615].wd_60m[0]=228.6505;
metmast_TH[616].wd_60m[0]=231.5679;
metmast_TH[617].wd_60m[0]=233.7436;
metmast_TH[618].wd_60m[0]=242.4243;
metmast_TH[619].wd_60m[0]=237.9001;
metmast_TH[620].wd_60m[0]=236.8632;
metmast_TH[621].wd_60m[0]=243.9608;
metmast_TH[622].wd_60m[0]=237.2959;
metmast_TH[623].wd_60m[0]=235.9819;
metmast_TH[624].wd_60m[0]=236.0948;
metmast_TH[625].wd_60m[0]=233.1667;
metmast_TH[626].wd_60m[0]=233.6911;
metmast_TH[627].wd_60m[0]=236.38;
metmast_TH[628].wd_60m[0]=243.207;
metmast_TH[629].wd_60m[0]=237.8252;
metmast_TH[630].wd_60m[0]=238.1388;
metmast_TH[631].wd_60m[0]=240.1113;
metmast_TH[632].wd_60m[0]=242.6576;
metmast_TH[633].wd_60m[0]=241.4562;
metmast_TH[634].wd_60m[0]=234.675;
metmast_TH[635].wd_60m[0]=244.677;
metmast_TH[636].wd_60m[0]=234.3457;
metmast_TH[637].wd_60m[0]=229.459;
metmast_TH[638].wd_60m[0]=220.9354;
metmast_TH[639].wd_60m[0]=220.9375;
metmast_TH[640].wd_60m[0]=223.8088;
metmast_TH[641].wd_60m[0]=229.6785;
metmast_TH[642].wd_60m[0]=232.1922;
metmast_TH[643].wd_60m[0]=235.8158;
metmast_TH[644].wd_60m[0]=235.6182;
metmast_TH[645].wd_60m[0]=238.5;
metmast_TH[646].wd_60m[0]=236.0742;
metmast_TH[647].wd_60m[0]=235.4505;
metmast_TH[648].wd_60m[0]=232.3039;
metmast_TH[649].wd_60m[0]=234.0527;
metmast_TH[650].wd_60m[0]=233.7766;
metmast_TH[651].wd_60m[0]=233.6039;
metmast_TH[652].wd_60m[0]=232.7333;
metmast_TH[653].wd_60m[0]=232.0345;
metmast_TH[654].wd_60m[0]=229.0428;
metmast_TH[655].wd_60m[0]=224.2659;
metmast_TH[656].wd_60m[0]=224.5693;
metmast_TH[657].wd_60m[0]=226.0767;
metmast_TH[658].wd_60m[0]=234.0176;
metmast_TH[659].wd_60m[0]=239.8783;
metmast_TH[660].wd_60m[0]=236.4865;
metmast_TH[661].wd_60m[0]=243.0644;
metmast_TH[662].wd_60m[0]=233.4232;
metmast_TH[663].wd_60m[0]=239.1902;
metmast_TH[664].wd_60m[0]=235.1373;
metmast_TH[665].wd_60m[0]=231.8773;
metmast_TH[666].wd_60m[0]=234.9303;
metmast_TH[667].wd_60m[0]=233.582;
metmast_TH[668].wd_60m[0]=231.5721;
metmast_TH[669].wd_60m[0]=227.9624;
metmast_TH[670].wd_60m[0]=232.0826;
metmast_TH[671].wd_60m[0]=228.6772;
metmast_TH[672].wd_60m[0]=227.0891;
metmast_TH[673].wd_60m[0]=229.98;
metmast_TH[674].wd_60m[0]=226.5298;
metmast_TH[675].wd_60m[0]=227.3769;
metmast_TH[676].wd_60m[0]=228.201;
metmast_TH[677].wd_60m[0]=228.996;
metmast_TH[678].wd_60m[0]=229.2329;
metmast_TH[679].wd_60m[0]=228.546;
metmast_TH[680].wd_60m[0]=227.7263;
metmast_TH[681].wd_60m[0]=225.9759;
metmast_TH[682].wd_60m[0]=226.3406;
metmast_TH[683].wd_60m[0]=224.893;
metmast_TH[684].wd_60m[0]=225.5504;
metmast_TH[685].wd_60m[0]=226.7683;
metmast_TH[686].wd_60m[0]=226.2158;
metmast_TH[687].wd_60m[0]=226.6895;
metmast_TH[688].wd_60m[0]=229.9844;
metmast_TH[689].wd_60m[0]=233.5353;
metmast_TH[690].wd_60m[0]=233.4414;
metmast_TH[691].wd_60m[0]=237.1164;
metmast_TH[692].wd_60m[0]=237.1722;
metmast_TH[693].wd_60m[0]=234.664;
metmast_TH[694].wd_60m[0]=232.5099;
metmast_TH[695].wd_60m[0]=232.2184;
metmast_TH[696].wd_60m[0]=229.4038;
metmast_TH[697].wd_60m[0]=228.006;
metmast_TH[698].wd_60m[0]=229.228;
metmast_TH[699].wd_60m[0]=229.6745;
metmast_TH[700].wd_60m[0]=230.2048;
metmast_TH[701].wd_60m[0]=232.8203;
metmast_TH[702].wd_60m[0]=229.9725;
metmast_TH[703].wd_60m[0]=230.1988;
metmast_TH[704].wd_60m[0]=232.7277;
metmast_TH[705].wd_60m[0]=234.6419;
metmast_TH[706].wd_60m[0]=237.3861;
metmast_TH[707].wd_60m[0]=239.084;
metmast_TH[708].wd_60m[0]=231.286;
metmast_TH[709].wd_60m[0]=229.6939;
metmast_TH[710].wd_60m[0]=229.4186;
metmast_TH[711].wd_60m[0]=230.8796;
metmast_TH[712].wd_60m[0]=225.7103;
metmast_TH[713].wd_60m[0]=228.8572;
metmast_TH[714].wd_60m[0]=234.2825;
metmast_TH[715].wd_60m[0]=237.9442;
metmast_TH[716].wd_60m[0]=244.7663;
metmast_TH[717].wd_60m[0]=244.5004;
metmast_TH[718].wd_60m[0]=240.0426;
metmast_TH[719].wd_60m[0]=234.3533;
metmast_TH[720].wd_60m[0]=232.3303;
metmast_TH[721].wd_60m[0]=230.8062;
metmast_TH[722].wd_60m[0]=235.267;
metmast_TH[723].wd_60m[0]=236.7814;
metmast_TH[724].wd_60m[0]=239.6182;
metmast_TH[725].wd_60m[0]=242.0801;
metmast_TH[726].wd_60m[0]=234.6253;
metmast_TH[727].wd_60m[0]=233.5582;
metmast_TH[728].wd_60m[0]=230.9009;
metmast_TH[729].wd_60m[0]=236.1877;
metmast_TH[730].wd_60m[0]=237.6432;
metmast_TH[731].wd_60m[0]=222.5835;
metmast_TH[732].wd_60m[0]=228.7792;
metmast_TH[733].wd_60m[0]=231.1103;
metmast_TH[734].wd_60m[0]=232.009;
metmast_TH[735].wd_60m[0]=234.4339;
metmast_TH[736].wd_60m[0]=239.1387;
metmast_TH[737].wd_60m[0]=238.1575;
metmast_TH[738].wd_60m[0]=246.143;
metmast_TH[739].wd_60m[0]=244.6651;
metmast_TH[740].wd_60m[0]=247.0704;
metmast_TH[741].wd_60m[0]=239.3023;
metmast_TH[742].wd_60m[0]=241.2593;
metmast_TH[743].wd_60m[0]=243.0169;
metmast_TH[744].wd_60m[0]=237.3508;
metmast_TH[745].wd_60m[0]=238.4277;
metmast_TH[746].wd_60m[0]=243.8737;
metmast_TH[747].wd_60m[0]=247.5363;
metmast_TH[748].wd_60m[0]=248.7607;
metmast_TH[749].wd_60m[0]=250.6325;
metmast_TH[750].wd_60m[0]=251.2268;
metmast_TH[751].wd_60m[0]=251.379;
metmast_TH[752].wd_60m[0]=253.2342;
metmast_TH[753].wd_60m[0]=251.6139;
metmast_TH[754].wd_60m[0]=250.9713;
metmast_TH[755].wd_60m[0]=250.6789;
metmast_TH[756].wd_60m[0]=245.288;
metmast_TH[757].wd_60m[0]=241.3973;
metmast_TH[758].wd_60m[0]=235.7449;
metmast_TH[759].wd_60m[0]=231.5966;
metmast_TH[760].wd_60m[0]=232.7917;
metmast_TH[761].wd_60m[0]=233.8787;
metmast_TH[762].wd_60m[0]=232.6936;
metmast_TH[763].wd_60m[0]=231.5104;
metmast_TH[764].wd_60m[0]=229.9035;
metmast_TH[765].wd_60m[0]=230.5042;
metmast_TH[766].wd_60m[0]=229.1719;
metmast_TH[767].wd_60m[0]=225.7504;
metmast_TH[768].wd_60m[0]=227.4688;
metmast_TH[769].wd_60m[0]=227.2837;
metmast_TH[770].wd_60m[0]=224.8468;
metmast_TH[771].wd_60m[0]=222.9764;
metmast_TH[772].wd_60m[0]=225.1448;
metmast_TH[773].wd_60m[0]=222.7993;
metmast_TH[774].wd_60m[0]=218.5983;
metmast_TH[775].wd_60m[0]=218.9908;
metmast_TH[776].wd_60m[0]=219.5092;
metmast_TH[777].wd_60m[0]=222.8892;
metmast_TH[778].wd_60m[0]=222.5707;
metmast_TH[779].wd_60m[0]=221.4329;
metmast_TH[780].wd_60m[0]=224.2702;
metmast_TH[781].wd_60m[0]=225.222;
metmast_TH[782].wd_60m[0]=230.7025;
metmast_TH[783].wd_60m[0]=226.0175;
metmast_TH[784].wd_60m[0]=220.5382;
metmast_TH[785].wd_60m[0]=222.733;
metmast_TH[786].wd_60m[0]=225.8036;
metmast_TH[787].wd_60m[0]=226.7665;
metmast_TH[788].wd_60m[0]=224.6537;
metmast_TH[789].wd_60m[0]=229.1352;
metmast_TH[790].wd_60m[0]=230.4228;
metmast_TH[791].wd_60m[0]=237.6558;
metmast_TH[792].wd_60m[0]=239.9003;
metmast_TH[793].wd_60m[0]=239.008;
metmast_TH[794].wd_60m[0]=234.0558;
metmast_TH[795].wd_60m[0]=232.9355;
metmast_TH[796].wd_60m[0]=230.9995;
metmast_TH[797].wd_60m[0]=228.5865;
metmast_TH[798].wd_60m[0]=227.5701;
metmast_TH[799].wd_60m[0]=226.1054;
metmast_TH[800].wd_60m[0]=223.9711;
metmast_TH[801].wd_60m[0]=224.966;
metmast_TH[802].wd_60m[0]=223.7099;
metmast_TH[803].wd_60m[0]=231.8841;
metmast_TH[804].wd_60m[0]=234.9459;
metmast_TH[805].wd_60m[0]=228.5311;
metmast_TH[806].wd_60m[0]=229.7002;
metmast_TH[807].wd_60m[0]=226.0246;
metmast_TH[808].wd_60m[0]=229.0157;
metmast_TH[809].wd_60m[0]=227.9065;
metmast_TH[810].wd_60m[0]=232.5515;
metmast_TH[811].wd_60m[0]=231.1072;
metmast_TH[812].wd_60m[0]=230.0042;
metmast_TH[813].wd_60m[0]=233.0849;
metmast_TH[814].wd_60m[0]=233.7307;
metmast_TH[815].wd_60m[0]=233.6876;
metmast_TH[816].wd_60m[0]=232.7324;
metmast_TH[817].wd_60m[0]=234.1878;
metmast_TH[818].wd_60m[0]=236.9937;
metmast_TH[819].wd_60m[0]=238.6604;
metmast_TH[820].wd_60m[0]=237.4634;
metmast_TH[821].wd_60m[0]=228.8131;
metmast_TH[822].wd_60m[0]=231.1163;
metmast_TH[823].wd_60m[0]=229.4262;
metmast_TH[824].wd_60m[0]=230.6414;
metmast_TH[825].wd_60m[0]=229.3137;
metmast_TH[826].wd_60m[0]=229.3512;
metmast_TH[827].wd_60m[0]=229.9444;
metmast_TH[828].wd_60m[0]=230.4535;
metmast_TH[829].wd_60m[0]=229.2021;
metmast_TH[830].wd_60m[0]=231.0202;
metmast_TH[831].wd_60m[0]=228.6857;
metmast_TH[832].wd_60m[0]=223.4365;
metmast_TH[833].wd_60m[0]=224.7339;
metmast_TH[834].wd_60m[0]=231.927;
metmast_TH[835].wd_60m[0]=233.6467;
metmast_TH[836].wd_60m[0]=229.5028;
metmast_TH[837].wd_60m[0]=226.9456;
metmast_TH[838].wd_60m[0]=219.8339;
metmast_TH[839].wd_60m[0]=214.926;
metmast_TH[840].wd_60m[0]=211.9974;
metmast_TH[841].wd_60m[0]=214.9388;
metmast_TH[842].wd_60m[0]=213.4467;
metmast_TH[843].wd_60m[0]=212.8315;
metmast_TH[844].wd_60m[0]=215.1701;
metmast_TH[845].wd_60m[0]=222.5011;
metmast_TH[846].wd_60m[0]=234.6327;
metmast_TH[847].wd_60m[0]=227.4769;
metmast_TH[848].wd_60m[0]=236.7724;
metmast_TH[849].wd_60m[0]=236.185;
metmast_TH[850].wd_60m[0]=239.5989;
metmast_TH[851].wd_60m[0]=243.6344;
metmast_TH[852].wd_60m[0]=235.8743;
metmast_TH[853].wd_60m[0]=243.9349;
metmast_TH[854].wd_60m[0]=236.9735;
metmast_TH[855].wd_60m[0]=235.176;
metmast_TH[856].wd_60m[0]=236.4557;
metmast_TH[857].wd_60m[0]=239.4257;
metmast_TH[858].wd_60m[0]=240.1014;
metmast_TH[859].wd_60m[0]=239.439;
metmast_TH[860].wd_60m[0]=238.2663;
metmast_TH[861].wd_60m[0]=227.0853;
metmast_TH[862].wd_60m[0]=228.0177;
metmast_TH[863].wd_60m[0]=226.8431;
metmast_TH[864].wd_60m[0]=231.4449;
metmast_TH[865].wd_60m[0]=229.3999;
metmast_TH[866].wd_60m[0]=227.8909;
metmast_TH[867].wd_60m[0]=231.9105;
metmast_TH[868].wd_60m[0]=234.1779;
metmast_TH[869].wd_60m[0]=224.1011;
metmast_TH[870].wd_60m[0]=236.5587;
metmast_TH[871].wd_60m[0]=238.6465;
metmast_TH[872].wd_60m[0]=241.6761;
metmast_TH[873].wd_60m[0]=240.9624;
metmast_TH[874].wd_60m[0]=241.2959;
metmast_TH[875].wd_60m[0]=233.5307;
metmast_TH[876].wd_60m[0]=238.0566;
metmast_TH[877].wd_60m[0]=236.5743;
metmast_TH[878].wd_60m[0]=234.2685;
metmast_TH[879].wd_60m[0]=234.5527;
metmast_TH[880].wd_60m[0]=233.8843;
metmast_TH[881].wd_60m[0]=233.9128;
metmast_TH[882].wd_60m[0]=237.081;
metmast_TH[883].wd_60m[0]=236.1076;
metmast_TH[884].wd_60m[0]=236.2768;
metmast_TH[885].wd_60m[0]=234.7462;
metmast_TH[886].wd_60m[0]=238.0457;
metmast_TH[887].wd_60m[0]=242.707;
metmast_TH[888].wd_60m[0]=240.1083;
metmast_TH[889].wd_60m[0]=245.386;
metmast_TH[890].wd_60m[0]=245.7674;
metmast_TH[891].wd_60m[0]=240.2846;
metmast_TH[892].wd_60m[0]=239.4518;
metmast_TH[893].wd_60m[0]=239.9398;
metmast_TH[894].wd_60m[0]=241.4457;
metmast_TH[895].wd_60m[0]=238.4184;
metmast_TH[896].wd_60m[0]=236.4302;
metmast_TH[897].wd_60m[0]=244.9466;
metmast_TH[898].wd_60m[0]=245.7714;
metmast_TH[899].wd_60m[0]=243.9035;
metmast_TH[900].wd_60m[0]=245.9212;
metmast_TH[901].wd_60m[0]=241.5634;
metmast_TH[902].wd_60m[0]=239.293;
metmast_TH[903].wd_60m[0]=237.2348;
metmast_TH[904].wd_60m[0]=236.6372;
metmast_TH[905].wd_60m[0]=238.6776;
metmast_TH[906].wd_60m[0]=235.9045;
metmast_TH[907].wd_60m[0]=233.9801;
metmast_TH[908].wd_60m[0]=237.0877;
metmast_TH[909].wd_60m[0]=235.1781;
metmast_TH[910].wd_60m[0]=236.7356;
metmast_TH[911].wd_60m[0]=237.7714;
metmast_TH[912].wd_60m[0]=235.4357;
metmast_TH[913].wd_60m[0]=231.1525;
metmast_TH[914].wd_60m[0]=231.5691;
metmast_TH[915].wd_60m[0]=230.5617;
metmast_TH[916].wd_60m[0]=228.622;
metmast_TH[917].wd_60m[0]=230.4163;
metmast_TH[918].wd_60m[0]=231.8012;
metmast_TH[919].wd_60m[0]=231.8438;
metmast_TH[920].wd_60m[0]=234.4252;
metmast_TH[921].wd_60m[0]=238.6238;
metmast_TH[922].wd_60m[0]=244.2925;
metmast_TH[923].wd_60m[0]=244.0301;
metmast_TH[924].wd_60m[0]=240.3484;
metmast_TH[925].wd_60m[0]=236.247;
metmast_TH[926].wd_60m[0]=234.8805;
metmast_TH[927].wd_60m[0]=235.0283;
metmast_TH[928].wd_60m[0]=233.4127;
metmast_TH[929].wd_60m[0]=237.5739;
metmast_TH[930].wd_60m[0]=235.4687;
metmast_TH[931].wd_60m[0]=235.1739;
metmast_TH[932].wd_60m[0]=234.4935;
metmast_TH[933].wd_60m[0]=235.4713;
metmast_TH[934].wd_60m[0]=235.7515;
metmast_TH[935].wd_60m[0]=237.2458;
metmast_TH[936].wd_60m[0]=238.2753;
metmast_TH[937].wd_60m[0]=242.5794;
metmast_TH[938].wd_60m[0]=242.4195;
metmast_TH[939].wd_60m[0]=241.7529;
metmast_TH[940].wd_60m[0]=235.0393;
metmast_TH[941].wd_60m[0]=240.4525;
metmast_TH[942].wd_60m[0]=243.393;
metmast_TH[943].wd_60m[0]=240.2229;
metmast_TH[944].wd_60m[0]=245.849;
metmast_TH[945].wd_60m[0]=238.9185;
metmast_TH[946].wd_60m[0]=247.2593;
metmast_TH[947].wd_60m[0]=240.9318;
metmast_TH[948].wd_60m[0]=239.1607;
metmast_TH[949].wd_60m[0]=234.3198;
metmast_TH[950].wd_60m[0]=234.2957;
metmast_TH[951].wd_60m[0]=235.5243;
metmast_TH[952].wd_60m[0]=239.2115;
metmast_TH[953].wd_60m[0]=242.315;
metmast_TH[954].wd_60m[0]=242.0852;
metmast_TH[955].wd_60m[0]=239.3689;
metmast_TH[956].wd_60m[0]=237.6379;
metmast_TH[957].wd_60m[0]=235.9483;
metmast_TH[958].wd_60m[0]=233.0906;
metmast_TH[959].wd_60m[0]=230.4416;
metmast_TH[960].wd_60m[0]=233.7468;
metmast_TH[961].wd_60m[0]=231.7515;
metmast_TH[962].wd_60m[0]=233.4175;
metmast_TH[963].wd_60m[0]=231.5695;
metmast_TH[964].wd_60m[0]=231.2197;
metmast_TH[965].wd_60m[0]=232.3656;
metmast_TH[966].wd_60m[0]=226.3154;
metmast_TH[967].wd_60m[0]=234.4287;
metmast_TH[968].wd_60m[0]=243.1871;
metmast_TH[969].wd_60m[0]=235.9652;
metmast_TH[970].wd_60m[0]=229.3946;
metmast_TH[971].wd_60m[0]=226.825;
metmast_TH[972].wd_60m[0]=223.8033;
metmast_TH[973].wd_60m[0]=225.7961;
metmast_TH[974].wd_60m[0]=224.6547;
metmast_TH[975].wd_60m[0]=228.2991;
metmast_TH[976].wd_60m[0]=222.4521;
metmast_TH[977].wd_60m[0]=218.31;
metmast_TH[978].wd_60m[0]=223.5306;
metmast_TH[979].wd_60m[0]=223.4322;
metmast_TH[980].wd_60m[0]=230.4275;
metmast_TH[981].wd_60m[0]=231.3305;
metmast_TH[982].wd_60m[0]=223.7751;
metmast_TH[983].wd_60m[0]=226.6808;
metmast_TH[984].wd_60m[0]=229.1956;
metmast_TH[985].wd_60m[0]=233.5564;
metmast_TH[986].wd_60m[0]=233.3621;
metmast_TH[987].wd_60m[0]=233.2722;
metmast_TH[988].wd_60m[0]=233.8061;
metmast_TH[989].wd_60m[0]=232.8941;
metmast_TH[990].wd_60m[0]=231.9146;
metmast_TH[991].wd_60m[0]=230.7811;
metmast_TH[992].wd_60m[0]=231.1754;
metmast_TH[993].wd_60m[0]=229.503;
metmast_TH[994].wd_60m[0]=223.4296;
metmast_TH[995].wd_60m[0]=221.3153;
metmast_TH[996].wd_60m[0]=225.0029;
metmast_TH[997].wd_60m[0]=231.7298;
metmast_TH[998].wd_60m[0]=235.6349;
metmast_TH[999].wd_60m[0]=239.8611;
metmast_TH[0].temperature[0]=7.9978;
metmast_TH[1].temperature[0]=7.9937;
metmast_TH[2].temperature[0]=7.9979;
metmast_TH[3].temperature[0]=7.9949;
metmast_TH[4].temperature[0]=8.0002;
metmast_TH[5].temperature[0]=7.994;
metmast_TH[6].temperature[0]=8.0054;
metmast_TH[7].temperature[0]=7.9962;
metmast_TH[8].temperature[0]=7.994;
metmast_TH[9].temperature[0]=8.0007;
metmast_TH[10].temperature[0]=7.9954;
metmast_TH[11].temperature[0]=8.0011;
metmast_TH[12].temperature[0]=8.0049;
metmast_TH[13].temperature[0]=7.9971;
metmast_TH[14].temperature[0]=7.9919;
metmast_TH[15].temperature[0]=8.0018;
metmast_TH[16].temperature[0]=7.9872;
metmast_TH[17].temperature[0]=7.9934;
metmast_TH[18].temperature[0]=8.0039;
metmast_TH[19].temperature[0]=8.0016;
metmast_TH[20].temperature[0]=8.0039;
metmast_TH[21].temperature[0]=7.9988;
metmast_TH[22].temperature[0]=7.9889;
metmast_TH[23].temperature[0]=7.9879;
metmast_TH[24].temperature[0]=7.9906;
metmast_TH[25].temperature[0]=7.9989;
metmast_TH[26].temperature[0]=8;
metmast_TH[27].temperature[0]=7.9977;
metmast_TH[28].temperature[0]=8.004;
metmast_TH[29].temperature[0]=7.99;
metmast_TH[30].temperature[0]=7.9989;
metmast_TH[31].temperature[0]=7.9956;
metmast_TH[32].temperature[0]=7.9901;
metmast_TH[33].temperature[0]=7.9949;
metmast_TH[34].temperature[0]=7.9941;
metmast_TH[35].temperature[0]=7.9897;
metmast_TH[36].temperature[0]=8.0022;
metmast_TH[37].temperature[0]=7.9974;
metmast_TH[38].temperature[0]=7.9962;
metmast_TH[39].temperature[0]=7.9946;
metmast_TH[40].temperature[0]=8.0054;
metmast_TH[41].temperature[0]=7.9928;
metmast_TH[42].temperature[0]=8.0059;
metmast_TH[43].temperature[0]=7.9906;
metmast_TH[44].temperature[0]=7.9912;
metmast_TH[45].temperature[0]=8.0049;
metmast_TH[46].temperature[0]=7.9972;
metmast_TH[47].temperature[0]=8.0087;
metmast_TH[48].temperature[0]=7.9993;
metmast_TH[49].temperature[0]=8.0085;
metmast_TH[50].temperature[0]=7.9968;
metmast_TH[51].temperature[0]=8.004;
metmast_TH[52].temperature[0]=8.0063;
metmast_TH[53].temperature[0]=8.0054;
metmast_TH[54].temperature[0]=8.0027;
metmast_TH[55].temperature[0]=8.002;
metmast_TH[56].temperature[0]=7.9993;
metmast_TH[57].temperature[0]=7.9994;
metmast_TH[58].temperature[0]=8.0022;
metmast_TH[59].temperature[0]=7.9989;
metmast_TH[60].temperature[0]=7.9966;
metmast_TH[61].temperature[0]=8.0116;
metmast_TH[62].temperature[0]=8.0056;
metmast_TH[63].temperature[0]=8.0079;
metmast_TH[64].temperature[0]=8.015;
metmast_TH[65].temperature[0]=8.0107;
metmast_TH[66].temperature[0]=8.0056;
metmast_TH[67].temperature[0]=8.0115;
metmast_TH[68].temperature[0]=8.0067;
metmast_TH[69].temperature[0]=8.0072;
metmast_TH[70].temperature[0]=8.0127;
metmast_TH[71].temperature[0]=8.0193;
metmast_TH[72].temperature[0]=8.0103;
metmast_TH[73].temperature[0]=8.0168;
metmast_TH[74].temperature[0]=8.0179;
metmast_TH[75].temperature[0]=8.0016;
metmast_TH[76].temperature[0]=7.9977;
metmast_TH[77].temperature[0]=8.0223;
metmast_TH[78].temperature[0]=8.0156;
metmast_TH[79].temperature[0]=8.022;
metmast_TH[80].temperature[0]=8.0188;
metmast_TH[81].temperature[0]=8.0164;
metmast_TH[82].temperature[0]=8.0192;
metmast_TH[83].temperature[0]=8.0193;
metmast_TH[84].temperature[0]=8.0073;
metmast_TH[85].temperature[0]=8.0197;
metmast_TH[86].temperature[0]=8.0074;
metmast_TH[87].temperature[0]=8.02;
metmast_TH[88].temperature[0]=8.017;
metmast_TH[89].temperature[0]=8.0148;
metmast_TH[90].temperature[0]=8.0195;
metmast_TH[91].temperature[0]=8.016;
metmast_TH[92].temperature[0]=8.0259;
metmast_TH[93].temperature[0]=8.016;
metmast_TH[94].temperature[0]=8.0278;
metmast_TH[95].temperature[0]=8.0172;
metmast_TH[96].temperature[0]=8.0144;
metmast_TH[97].temperature[0]=8.0215;
metmast_TH[98].temperature[0]=8.0206;
metmast_TH[99].temperature[0]=8.0219;
metmast_TH[100].temperature[0]=8.0245;
metmast_TH[101].temperature[0]=8.014;
metmast_TH[102].temperature[0]=8.0204;
metmast_TH[103].temperature[0]=8.0161;
metmast_TH[104].temperature[0]=8.023;
metmast_TH[105].temperature[0]=8.0294;
metmast_TH[106].temperature[0]=8.0228;
metmast_TH[107].temperature[0]=8.0272;
metmast_TH[108].temperature[0]=8.0186;
metmast_TH[109].temperature[0]=8.0211;
metmast_TH[110].temperature[0]=8.0294;
metmast_TH[111].temperature[0]=8.0171;
metmast_TH[112].temperature[0]=8.0255;
metmast_TH[113].temperature[0]=8.0148;
metmast_TH[114].temperature[0]=8.0254;
metmast_TH[115].temperature[0]=8.0203;
metmast_TH[116].temperature[0]=8.0204;
metmast_TH[117].temperature[0]=8.0205;
metmast_TH[118].temperature[0]=8.0183;
metmast_TH[119].temperature[0]=8.0173;
metmast_TH[120].temperature[0]=8.0211;
metmast_TH[121].temperature[0]=8.0112;
metmast_TH[122].temperature[0]=8.0153;
metmast_TH[123].temperature[0]=8.016;
metmast_TH[124].temperature[0]=8.0127;
metmast_TH[125].temperature[0]=8.0144;
metmast_TH[126].temperature[0]=8.0159;
metmast_TH[127].temperature[0]=8.0175;
metmast_TH[128].temperature[0]=8.0082;
metmast_TH[129].temperature[0]=8.0157;
metmast_TH[130].temperature[0]=8.0155;
metmast_TH[131].temperature[0]=8.016;
metmast_TH[132].temperature[0]=8.0177;
metmast_TH[133].temperature[0]=8.0157;
metmast_TH[134].temperature[0]=8.0199;
metmast_TH[135].temperature[0]=8.0192;
metmast_TH[136].temperature[0]=8.0184;
metmast_TH[137].temperature[0]=8.016;
metmast_TH[138].temperature[0]=8.0203;
metmast_TH[139].temperature[0]=8.0269;
metmast_TH[140].temperature[0]=8.0234;
metmast_TH[141].temperature[0]=8.014;
metmast_TH[142].temperature[0]=8.026;
metmast_TH[143].temperature[0]=8.0133;
metmast_TH[144].temperature[0]=8.0193;
metmast_TH[145].temperature[0]=8.024;
metmast_TH[146].temperature[0]=8.015;
metmast_TH[147].temperature[0]=8.0189;
metmast_TH[148].temperature[0]=8.0217;
metmast_TH[149].temperature[0]=8.0211;
metmast_TH[150].temperature[0]=8.0242;
metmast_TH[151].temperature[0]=8.021;
metmast_TH[152].temperature[0]=8.0173;
metmast_TH[153].temperature[0]=8.0245;
metmast_TH[154].temperature[0]=8.0219;
metmast_TH[155].temperature[0]=8.0222;
metmast_TH[156].temperature[0]=8.0221;
metmast_TH[157].temperature[0]=8.0203;
metmast_TH[158].temperature[0]=8.0303;
metmast_TH[159].temperature[0]=8.0225;
metmast_TH[160].temperature[0]=8.0175;
metmast_TH[161].temperature[0]=8.0231;
metmast_TH[162].temperature[0]=8.0255;
metmast_TH[163].temperature[0]=8.0233;
metmast_TH[164].temperature[0]=8.0234;
metmast_TH[165].temperature[0]=8.0205;
metmast_TH[166].temperature[0]=8.0209;
metmast_TH[167].temperature[0]=8.0208;
metmast_TH[168].temperature[0]=8.0267;
metmast_TH[169].temperature[0]=8.0256;
metmast_TH[170].temperature[0]=8.0208;
metmast_TH[171].temperature[0]=8.0269;
metmast_TH[172].temperature[0]=8.0294;
metmast_TH[173].temperature[0]=8.0286;
metmast_TH[174].temperature[0]=8.03;
metmast_TH[175].temperature[0]=8.0334;
metmast_TH[176].temperature[0]=8.028;
metmast_TH[177].temperature[0]=8.0189;
metmast_TH[178].temperature[0]=8.0349;
metmast_TH[179].temperature[0]=8.0297;
metmast_TH[180].temperature[0]=8.0292;
metmast_TH[181].temperature[0]=8.0339;
metmast_TH[182].temperature[0]=8.0282;
metmast_TH[183].temperature[0]=8.0344;
metmast_TH[184].temperature[0]=8.0315;
metmast_TH[185].temperature[0]=8.0377;
metmast_TH[186].temperature[0]=8.0291;
metmast_TH[187].temperature[0]=8.0386;
metmast_TH[188].temperature[0]=8.025;
metmast_TH[189].temperature[0]=8.0345;
metmast_TH[190].temperature[0]=8.0292;
metmast_TH[191].temperature[0]=8.0383;
metmast_TH[192].temperature[0]=8.0289;
metmast_TH[193].temperature[0]=8.0392;
metmast_TH[194].temperature[0]=8.0352;
metmast_TH[195].temperature[0]=8.0348;
metmast_TH[196].temperature[0]=8.0432;
metmast_TH[197].temperature[0]=8.0407;
metmast_TH[198].temperature[0]=8.0367;
metmast_TH[199].temperature[0]=8.0339;
metmast_TH[200].temperature[0]=8.0403;
metmast_TH[201].temperature[0]=8.0328;
metmast_TH[202].temperature[0]=8.0371;
metmast_TH[203].temperature[0]=8.0289;
metmast_TH[204].temperature[0]=8.0458;
metmast_TH[205].temperature[0]=8.0437;
metmast_TH[206].temperature[0]=8.0381;
metmast_TH[207].temperature[0]=8.0422;
metmast_TH[208].temperature[0]=8.0371;
metmast_TH[209].temperature[0]=8.0303;
metmast_TH[210].temperature[0]=8.0498;
metmast_TH[211].temperature[0]=8.0398;
metmast_TH[212].temperature[0]=8.0278;
metmast_TH[213].temperature[0]=8.0348;
metmast_TH[214].temperature[0]=8.0459;
metmast_TH[215].temperature[0]=8.0354;
metmast_TH[216].temperature[0]=8.0377;
metmast_TH[217].temperature[0]=8.032;
metmast_TH[218].temperature[0]=8.0397;
metmast_TH[219].temperature[0]=8.0399;
metmast_TH[220].temperature[0]=8.0443;
metmast_TH[221].temperature[0]=8.0479;
metmast_TH[222].temperature[0]=8.0592;
metmast_TH[223].temperature[0]=8.0449;
metmast_TH[224].temperature[0]=8.0453;
metmast_TH[225].temperature[0]=8.0466;
metmast_TH[226].temperature[0]=8.0559;
metmast_TH[227].temperature[0]=8.0348;
metmast_TH[228].temperature[0]=8.0531;
metmast_TH[229].temperature[0]=8.0543;
metmast_TH[230].temperature[0]=8.0513;
metmast_TH[231].temperature[0]=8.0518;
metmast_TH[232].temperature[0]=8.0531;
metmast_TH[233].temperature[0]=8.054;
metmast_TH[234].temperature[0]=8.0559;
metmast_TH[235].temperature[0]=8.0422;
metmast_TH[236].temperature[0]=8.052;
metmast_TH[237].temperature[0]=8.0504;
metmast_TH[238].temperature[0]=8.0579;
metmast_TH[239].temperature[0]=8.0585;
metmast_TH[240].temperature[0]=8.0564;
metmast_TH[241].temperature[0]=8.0513;
metmast_TH[242].temperature[0]=8.0607;
metmast_TH[243].temperature[0]=8.051;
metmast_TH[244].temperature[0]=8.0568;
metmast_TH[245].temperature[0]=8.0613;
metmast_TH[246].temperature[0]=8.0558;
metmast_TH[247].temperature[0]=8.0566;
metmast_TH[248].temperature[0]=8.0571;
metmast_TH[249].temperature[0]=8.0684;
metmast_TH[250].temperature[0]=8.0582;
metmast_TH[251].temperature[0]=8.0532;
metmast_TH[252].temperature[0]=8.0565;
metmast_TH[253].temperature[0]=8.0663;
metmast_TH[254].temperature[0]=8.0552;
metmast_TH[255].temperature[0]=8.0577;
metmast_TH[256].temperature[0]=8.0638;
metmast_TH[257].temperature[0]=8.0619;
metmast_TH[258].temperature[0]=8.0586;
metmast_TH[259].temperature[0]=8.0575;
metmast_TH[260].temperature[0]=8.059;
metmast_TH[261].temperature[0]=8.0592;
metmast_TH[262].temperature[0]=8.0582;
metmast_TH[263].temperature[0]=8.0608;
metmast_TH[264].temperature[0]=8.0498;
metmast_TH[265].temperature[0]=8.059;
metmast_TH[266].temperature[0]=8.0529;
metmast_TH[267].temperature[0]=8.0544;
metmast_TH[268].temperature[0]=8.0585;
metmast_TH[269].temperature[0]=8.0522;
metmast_TH[270].temperature[0]=8.0532;
metmast_TH[271].temperature[0]=8.0607;
metmast_TH[272].temperature[0]=8.0529;
metmast_TH[273].temperature[0]=8.0532;
metmast_TH[274].temperature[0]=8.0669;
metmast_TH[275].temperature[0]=8.059;
metmast_TH[276].temperature[0]=8.0588;
metmast_TH[277].temperature[0]=8.0493;
metmast_TH[278].temperature[0]=8.0575;
metmast_TH[279].temperature[0]=8.0645;
metmast_TH[280].temperature[0]=8.0601;
metmast_TH[281].temperature[0]=8.0571;
metmast_TH[282].temperature[0]=8.0548;
metmast_TH[283].temperature[0]=8.0616;
metmast_TH[284].temperature[0]=8.0641;
metmast_TH[285].temperature[0]=8.0563;
metmast_TH[286].temperature[0]=8.0586;
metmast_TH[287].temperature[0]=8.0575;
metmast_TH[288].temperature[0]=8.0649;
metmast_TH[289].temperature[0]=8.0575;
metmast_TH[290].temperature[0]=8.0537;
metmast_TH[291].temperature[0]=8.0673;
metmast_TH[292].temperature[0]=8.0598;
metmast_TH[293].temperature[0]=8.0658;
metmast_TH[294].temperature[0]=8.0607;
metmast_TH[295].temperature[0]=8.0623;
metmast_TH[296].temperature[0]=8.0597;
metmast_TH[297].temperature[0]=8.0643;
metmast_TH[298].temperature[0]=8.0612;
metmast_TH[299].temperature[0]=8.0626;
metmast_TH[300].temperature[0]=8.0614;
metmast_TH[301].temperature[0]=8.0712;
metmast_TH[302].temperature[0]=8.0574;
metmast_TH[303].temperature[0]=8.0589;
metmast_TH[304].temperature[0]=8.0627;
metmast_TH[305].temperature[0]=8.0676;
metmast_TH[306].temperature[0]=8.0601;
metmast_TH[307].temperature[0]=8.072;
metmast_TH[308].temperature[0]=8.0573;
metmast_TH[309].temperature[0]=8.0595;
metmast_TH[310].temperature[0]=8.0585;
metmast_TH[311].temperature[0]=8.0614;
metmast_TH[312].temperature[0]=8.0654;
metmast_TH[313].temperature[0]=8.0585;
metmast_TH[314].temperature[0]=8.0636;
metmast_TH[315].temperature[0]=8.0635;
metmast_TH[316].temperature[0]=8.0623;
metmast_TH[317].temperature[0]=8.0723;
metmast_TH[318].temperature[0]=8.0569;
metmast_TH[319].temperature[0]=8.0659;
metmast_TH[320].temperature[0]=8.0696;
metmast_TH[321].temperature[0]=8.0526;
metmast_TH[322].temperature[0]=8.069;
metmast_TH[323].temperature[0]=8.0609;
metmast_TH[324].temperature[0]=8.0674;
metmast_TH[325].temperature[0]=8.062;
metmast_TH[326].temperature[0]=8.0653;
metmast_TH[327].temperature[0]=8.0709;
metmast_TH[328].temperature[0]=8.0695;
metmast_TH[329].temperature[0]=8.0636;
metmast_TH[330].temperature[0]=8.0608;
metmast_TH[331].temperature[0]=8.0709;
metmast_TH[332].temperature[0]=8.0623;
metmast_TH[333].temperature[0]=8.0763;
metmast_TH[334].temperature[0]=8.0686;
metmast_TH[335].temperature[0]=8.0656;
metmast_TH[336].temperature[0]=8.0634;
metmast_TH[337].temperature[0]=8.0686;
metmast_TH[338].temperature[0]=8.0679;
metmast_TH[339].temperature[0]=8.0707;
metmast_TH[340].temperature[0]=8.067;
metmast_TH[341].temperature[0]=8.0769;
metmast_TH[342].temperature[0]=8.0779;
metmast_TH[343].temperature[0]=8.0736;
metmast_TH[344].temperature[0]=8.064;
metmast_TH[345].temperature[0]=8.0729;
metmast_TH[346].temperature[0]=8.0684;
metmast_TH[347].temperature[0]=8.0726;
metmast_TH[348].temperature[0]=8.0696;
metmast_TH[349].temperature[0]=8.0618;
metmast_TH[350].temperature[0]=8.0696;
metmast_TH[351].temperature[0]=8.0697;
metmast_TH[352].temperature[0]=8.0718;
metmast_TH[353].temperature[0]=8.0729;
metmast_TH[354].temperature[0]=8.0725;
metmast_TH[355].temperature[0]=8.0791;
metmast_TH[356].temperature[0]=8.0752;
metmast_TH[357].temperature[0]=8.0662;
metmast_TH[358].temperature[0]=8.069;
metmast_TH[359].temperature[0]=8.0621;
metmast_TH[360].temperature[0]=8.0713;
metmast_TH[361].temperature[0]=8.078;
metmast_TH[362].temperature[0]=8.0807;
metmast_TH[363].temperature[0]=8.0762;
metmast_TH[364].temperature[0]=8.0796;
metmast_TH[365].temperature[0]=8.0706;
metmast_TH[366].temperature[0]=8.0715;
metmast_TH[367].temperature[0]=8.0753;
metmast_TH[368].temperature[0]=8.0752;
metmast_TH[369].temperature[0]=8.0742;
metmast_TH[370].temperature[0]=8.0767;
metmast_TH[371].temperature[0]=8.0812;
metmast_TH[372].temperature[0]=8.0836;
metmast_TH[373].temperature[0]=8.0734;
metmast_TH[374].temperature[0]=8.0798;
metmast_TH[375].temperature[0]=8.072;
metmast_TH[376].temperature[0]=8.0845;
metmast_TH[377].temperature[0]=8.0785;
metmast_TH[378].temperature[0]=8.0848;
metmast_TH[379].temperature[0]=8.084;
metmast_TH[380].temperature[0]=8.0809;
metmast_TH[381].temperature[0]=8.0736;
metmast_TH[382].temperature[0]=8.0758;
metmast_TH[383].temperature[0]=8.0828;
metmast_TH[384].temperature[0]=8.0775;
metmast_TH[385].temperature[0]=8.071;
metmast_TH[386].temperature[0]=8.0772;
metmast_TH[387].temperature[0]=8.0779;
metmast_TH[388].temperature[0]=8.0786;
metmast_TH[389].temperature[0]=8.077;
metmast_TH[390].temperature[0]=8.0776;
metmast_TH[391].temperature[0]=8.0773;
metmast_TH[392].temperature[0]=8.0789;
metmast_TH[393].temperature[0]=8.0815;
metmast_TH[394].temperature[0]=8.0773;
metmast_TH[395].temperature[0]=8.078;
metmast_TH[396].temperature[0]=8.0768;
metmast_TH[397].temperature[0]=8.0752;
metmast_TH[398].temperature[0]=8.0737;
metmast_TH[399].temperature[0]=8.0775;
metmast_TH[400].temperature[0]=8.0703;
metmast_TH[401].temperature[0]=8.0692;
metmast_TH[402].temperature[0]=8.0748;
metmast_TH[403].temperature[0]=8.072;
metmast_TH[404].temperature[0]=8.0763;
metmast_TH[405].temperature[0]=8.0669;
metmast_TH[406].temperature[0]=8.0723;
metmast_TH[407].temperature[0]=8.0787;
metmast_TH[408].temperature[0]=8.0753;
metmast_TH[409].temperature[0]=8.0784;
metmast_TH[410].temperature[0]=8.0675;
metmast_TH[411].temperature[0]=8.0829;
metmast_TH[412].temperature[0]=8.0709;
metmast_TH[413].temperature[0]=8.0774;
metmast_TH[414].temperature[0]=8.0748;
metmast_TH[415].temperature[0]=8.073;
metmast_TH[416].temperature[0]=8.0757;
metmast_TH[417].temperature[0]=8.0784;
metmast_TH[418].temperature[0]=8.0807;
metmast_TH[419].temperature[0]=8.0804;
metmast_TH[420].temperature[0]=8.0739;
metmast_TH[421].temperature[0]=8.0829;
metmast_TH[422].temperature[0]=8.0773;
metmast_TH[423].temperature[0]=8.0896;
metmast_TH[424].temperature[0]=8.0792;
metmast_TH[425].temperature[0]=8.0803;
metmast_TH[426].temperature[0]=8.0773;
metmast_TH[427].temperature[0]=8.079;
metmast_TH[428].temperature[0]=8.0756;
metmast_TH[429].temperature[0]=8.0761;
metmast_TH[430].temperature[0]=8.0759;
metmast_TH[431].temperature[0]=8.0786;
metmast_TH[432].temperature[0]=8.0828;
metmast_TH[433].temperature[0]=8.077;
metmast_TH[434].temperature[0]=8.0732;
metmast_TH[435].temperature[0]=8.082;
metmast_TH[436].temperature[0]=8.0813;
metmast_TH[437].temperature[0]=8.08;
metmast_TH[438].temperature[0]=8.0745;
metmast_TH[439].temperature[0]=8.0774;
metmast_TH[440].temperature[0]=8.0768;
metmast_TH[441].temperature[0]=8.0866;
metmast_TH[442].temperature[0]=8.0756;
metmast_TH[443].temperature[0]=8.0697;
metmast_TH[444].temperature[0]=8.0691;
metmast_TH[445].temperature[0]=8.0645;
metmast_TH[446].temperature[0]=8.0647;
metmast_TH[447].temperature[0]=8.0701;
metmast_TH[448].temperature[0]=8.0698;
metmast_TH[449].temperature[0]=8.0681;
metmast_TH[450].temperature[0]=8.0691;
metmast_TH[451].temperature[0]=8.0782;
metmast_TH[452].temperature[0]=8.0695;
metmast_TH[453].temperature[0]=8.07;
metmast_TH[454].temperature[0]=8.0685;
metmast_TH[455].temperature[0]=8.0709;
metmast_TH[456].temperature[0]=8.067;
metmast_TH[457].temperature[0]=8.0731;
metmast_TH[458].temperature[0]=8.0697;
metmast_TH[459].temperature[0]=8.0745;
metmast_TH[460].temperature[0]=8.0698;
metmast_TH[461].temperature[0]=8.0669;
metmast_TH[462].temperature[0]=8.061;
metmast_TH[463].temperature[0]=8.0681;
metmast_TH[464].temperature[0]=8.0673;
metmast_TH[465].temperature[0]=8.0757;
metmast_TH[466].temperature[0]=8.0663;
metmast_TH[467].temperature[0]=8.0726;
metmast_TH[468].temperature[0]=8.0667;
metmast_TH[469].temperature[0]=8.0729;
metmast_TH[470].temperature[0]=8.0692;
metmast_TH[471].temperature[0]=8.077;
metmast_TH[472].temperature[0]=8.0697;
metmast_TH[473].temperature[0]=8.0627;
metmast_TH[474].temperature[0]=8.0684;
metmast_TH[475].temperature[0]=8.0681;
metmast_TH[476].temperature[0]=8.0709;
metmast_TH[477].temperature[0]=8.0685;
metmast_TH[478].temperature[0]=8.075;
metmast_TH[479].temperature[0]=8.0748;
metmast_TH[480].temperature[0]=8.074;
metmast_TH[481].temperature[0]=8.0627;
metmast_TH[482].temperature[0]=8.0584;
metmast_TH[483].temperature[0]=8.0646;
metmast_TH[484].temperature[0]=8.0731;
metmast_TH[485].temperature[0]=8.0742;
metmast_TH[486].temperature[0]=8.0668;
metmast_TH[487].temperature[0]=8.0645;
metmast_TH[488].temperature[0]=8.0696;
metmast_TH[489].temperature[0]=8.0746;
metmast_TH[490].temperature[0]=8.0624;
metmast_TH[491].temperature[0]=8.0721;
metmast_TH[492].temperature[0]=8.0697;
metmast_TH[493].temperature[0]=8.0681;
metmast_TH[494].temperature[0]=8.0636;
metmast_TH[495].temperature[0]=8.0669;
metmast_TH[496].temperature[0]=8.0668;
metmast_TH[497].temperature[0]=8.0668;
metmast_TH[498].temperature[0]=8.0641;
metmast_TH[499].temperature[0]=8.0654;
metmast_TH[500].temperature[0]=8.0726;
metmast_TH[501].temperature[0]=8.0625;
metmast_TH[502].temperature[0]=8.0682;
metmast_TH[503].temperature[0]=8.0682;
metmast_TH[504].temperature[0]=8.0713;
metmast_TH[505].temperature[0]=8.0689;
metmast_TH[506].temperature[0]=8.0584;
metmast_TH[507].temperature[0]=8.0758;
metmast_TH[508].temperature[0]=8.0648;
metmast_TH[509].temperature[0]=8.0663;
metmast_TH[510].temperature[0]=8.0663;
metmast_TH[511].temperature[0]=8.0718;
metmast_TH[512].temperature[0]=8.0565;
metmast_TH[513].temperature[0]=8.0706;
metmast_TH[514].temperature[0]=8.068;
metmast_TH[515].temperature[0]=8.0637;
metmast_TH[516].temperature[0]=8.0741;
metmast_TH[517].temperature[0]=8.0609;
metmast_TH[518].temperature[0]=8.0614;
metmast_TH[519].temperature[0]=8.0629;
metmast_TH[520].temperature[0]=8.0674;
metmast_TH[521].temperature[0]=8.0649;
metmast_TH[522].temperature[0]=8.0597;
metmast_TH[523].temperature[0]=8.0751;
metmast_TH[524].temperature[0]=8.0707;
metmast_TH[525].temperature[0]=8.0696;
metmast_TH[526].temperature[0]=8.0652;
metmast_TH[527].temperature[0]=8.0669;
metmast_TH[528].temperature[0]=8.0702;
metmast_TH[529].temperature[0]=8.0696;
metmast_TH[530].temperature[0]=8.0746;
metmast_TH[531].temperature[0]=8.0669;
metmast_TH[532].temperature[0]=8.0635;
metmast_TH[533].temperature[0]=8.077;
metmast_TH[534].temperature[0]=8.0775;
metmast_TH[535].temperature[0]=8.0824;
metmast_TH[536].temperature[0]=8.0747;
metmast_TH[537].temperature[0]=8.0715;
metmast_TH[538].temperature[0]=8.0741;
metmast_TH[539].temperature[0]=8.0774;
metmast_TH[540].temperature[0]=8.0789;
metmast_TH[541].temperature[0]=8.0756;
metmast_TH[542].temperature[0]=8.0719;
metmast_TH[543].temperature[0]=8.0781;
metmast_TH[544].temperature[0]=8.0701;
metmast_TH[545].temperature[0]=8.0685;
metmast_TH[546].temperature[0]=8.0723;
metmast_TH[547].temperature[0]=8.0752;
metmast_TH[548].temperature[0]=8.0681;
metmast_TH[549].temperature[0]=8.0645;
metmast_TH[550].temperature[0]=8.0774;
metmast_TH[551].temperature[0]=8.0751;
metmast_TH[552].temperature[0]=8.0707;
metmast_TH[553].temperature[0]=8.0748;
metmast_TH[554].temperature[0]=8.0767;
metmast_TH[555].temperature[0]=8.0803;
metmast_TH[556].temperature[0]=8.0748;
metmast_TH[557].temperature[0]=8.0718;
metmast_TH[558].temperature[0]=8.0794;
metmast_TH[559].temperature[0]=8.0732;
metmast_TH[560].temperature[0]=8.0637;
metmast_TH[561].temperature[0]=8.073;
metmast_TH[562].temperature[0]=8.0776;
metmast_TH[563].temperature[0]=8.0754;
metmast_TH[564].temperature[0]=8.0725;
metmast_TH[565].temperature[0]=8.0785;
metmast_TH[566].temperature[0]=8.0814;
metmast_TH[567].temperature[0]=8.0803;
metmast_TH[568].temperature[0]=8.0774;
metmast_TH[569].temperature[0]=8.0822;
metmast_TH[570].temperature[0]=8.0794;
metmast_TH[571].temperature[0]=8.0829;
metmast_TH[572].temperature[0]=8.07;
metmast_TH[573].temperature[0]=8.0717;
metmast_TH[574].temperature[0]=8.0778;
metmast_TH[575].temperature[0]=8.0807;
metmast_TH[576].temperature[0]=8.0739;
metmast_TH[577].temperature[0]=8.0717;
metmast_TH[578].temperature[0]=8.0662;
metmast_TH[579].temperature[0]=8.0752;
metmast_TH[580].temperature[0]=8.0725;
metmast_TH[581].temperature[0]=8.0684;
metmast_TH[582].temperature[0]=8.0807;
metmast_TH[583].temperature[0]=8.0697;
metmast_TH[584].temperature[0]=8.0703;
metmast_TH[585].temperature[0]=8.078;
metmast_TH[586].temperature[0]=8.071;
metmast_TH[587].temperature[0]=8.0776;
metmast_TH[588].temperature[0]=8.0822;
metmast_TH[589].temperature[0]=8.0719;
metmast_TH[590].temperature[0]=8.0784;
metmast_TH[591].temperature[0]=8.0796;
metmast_TH[592].temperature[0]=8.0774;
metmast_TH[593].temperature[0]=8.0745;
metmast_TH[594].temperature[0]=8.0781;
metmast_TH[595].temperature[0]=8.0823;
metmast_TH[596].temperature[0]=8.087;
metmast_TH[597].temperature[0]=8.0836;
metmast_TH[598].temperature[0]=8.0776;
metmast_TH[599].temperature[0]=8.075;
metmast_TH[600].temperature[0]=8.0824;
metmast_TH[601].temperature[0]=8.0826;
metmast_TH[602].temperature[0]=8.0813;
metmast_TH[603].temperature[0]=8.0836;
metmast_TH[604].temperature[0]=8.0819;
metmast_TH[605].temperature[0]=8.0751;
metmast_TH[606].temperature[0]=8.0761;
metmast_TH[607].temperature[0]=8.0826;
metmast_TH[608].temperature[0]=8.0803;
metmast_TH[609].temperature[0]=8.073;
metmast_TH[610].temperature[0]=8.0774;
metmast_TH[611].temperature[0]=8.0791;
metmast_TH[612].temperature[0]=8.0767;
metmast_TH[613].temperature[0]=8.0757;
metmast_TH[614].temperature[0]=8.0797;
metmast_TH[615].temperature[0]=8.0753;
metmast_TH[616].temperature[0]=8.0778;
metmast_TH[617].temperature[0]=8.0818;
metmast_TH[618].temperature[0]=8.079;
metmast_TH[619].temperature[0]=8.0785;
metmast_TH[620].temperature[0]=8.0781;
metmast_TH[621].temperature[0]=8.0717;
metmast_TH[622].temperature[0]=8.0804;
metmast_TH[623].temperature[0]=8.0761;
metmast_TH[624].temperature[0]=8.078;
metmast_TH[625].temperature[0]=8.0837;
metmast_TH[626].temperature[0]=8.0778;
metmast_TH[627].temperature[0]=8.0829;
metmast_TH[628].temperature[0]=8.0803;
metmast_TH[629].temperature[0]=8.0778;
metmast_TH[630].temperature[0]=8.0797;
metmast_TH[631].temperature[0]=8.0802;
metmast_TH[632].temperature[0]=8.0746;
metmast_TH[633].temperature[0]=8.08;
metmast_TH[634].temperature[0]=8.0881;
metmast_TH[635].temperature[0]=8.0846;
metmast_TH[636].temperature[0]=8.08;
metmast_TH[637].temperature[0]=8.0853;
metmast_TH[638].temperature[0]=8.0794;
metmast_TH[639].temperature[0]=8.0833;
metmast_TH[640].temperature[0]=8.0825;
metmast_TH[641].temperature[0]=8.0817;
metmast_TH[642].temperature[0]=8.0839;
metmast_TH[643].temperature[0]=8.0806;
metmast_TH[644].temperature[0]=8.0806;
metmast_TH[645].temperature[0]=8.0829;
metmast_TH[646].temperature[0]=8.0831;
metmast_TH[647].temperature[0]=8.079;
metmast_TH[648].temperature[0]=8.0786;
metmast_TH[649].temperature[0]=8.0822;
metmast_TH[650].temperature[0]=8.0885;
metmast_TH[651].temperature[0]=8.0845;
metmast_TH[652].temperature[0]=8.0977;
metmast_TH[653].temperature[0]=8.0846;
metmast_TH[654].temperature[0]=8.083;
metmast_TH[655].temperature[0]=8.0874;
metmast_TH[656].temperature[0]=8.0885;
metmast_TH[657].temperature[0]=8.0944;
metmast_TH[658].temperature[0]=8.0895;
metmast_TH[659].temperature[0]=8.0844;
metmast_TH[660].temperature[0]=8.0818;
metmast_TH[661].temperature[0]=8.0863;
metmast_TH[662].temperature[0]=8.0822;
metmast_TH[663].temperature[0]=8.0848;
metmast_TH[664].temperature[0]=8.0857;
metmast_TH[665].temperature[0]=8.0826;
metmast_TH[666].temperature[0]=8.0861;
metmast_TH[667].temperature[0]=8.0886;
metmast_TH[668].temperature[0]=8.0881;
metmast_TH[669].temperature[0]=8.089;
metmast_TH[670].temperature[0]=8.0808;
metmast_TH[671].temperature[0]=8.0837;
metmast_TH[672].temperature[0]=8.0773;
metmast_TH[673].temperature[0]=8.0868;
metmast_TH[674].temperature[0]=8.0837;
metmast_TH[675].temperature[0]=8.0857;
metmast_TH[676].temperature[0]=8.0819;
metmast_TH[677].temperature[0]=8.0866;
metmast_TH[678].temperature[0]=8.0883;
metmast_TH[679].temperature[0]=8.0836;
metmast_TH[680].temperature[0]=8.0818;
metmast_TH[681].temperature[0]=8.0855;
metmast_TH[682].temperature[0]=8.0855;
metmast_TH[683].temperature[0]=8.0837;
metmast_TH[684].temperature[0]=8.0891;
metmast_TH[685].temperature[0]=8.0924;
metmast_TH[686].temperature[0]=8.0853;
metmast_TH[687].temperature[0]=8.082;
metmast_TH[688].temperature[0]=8.0934;
metmast_TH[689].temperature[0]=8.0872;
metmast_TH[690].temperature[0]=8.0873;
metmast_TH[691].temperature[0]=8.0825;
metmast_TH[692].temperature[0]=8.0863;
metmast_TH[693].temperature[0]=8.0842;
metmast_TH[694].temperature[0]=8.0842;
metmast_TH[695].temperature[0]=8.0895;
metmast_TH[696].temperature[0]=8.0939;
metmast_TH[697].temperature[0]=8.0789;
metmast_TH[698].temperature[0]=8.0836;
metmast_TH[699].temperature[0]=8.0916;
metmast_TH[700].temperature[0]=8.0829;
metmast_TH[701].temperature[0]=8.0772;
metmast_TH[702].temperature[0]=8.0844;
metmast_TH[703].temperature[0]=8.089;
metmast_TH[704].temperature[0]=8.0877;
metmast_TH[705].temperature[0]=8.096;
metmast_TH[706].temperature[0]=8.0859;
metmast_TH[707].temperature[0]=8.0862;
metmast_TH[708].temperature[0]=8.0858;
metmast_TH[709].temperature[0]=8.0858;
metmast_TH[710].temperature[0]=8.0848;
metmast_TH[711].temperature[0]=8.0912;
metmast_TH[712].temperature[0]=8.0894;
metmast_TH[713].temperature[0]=8.0931;
metmast_TH[714].temperature[0]=8.0842;
metmast_TH[715].temperature[0]=8.0898;
metmast_TH[716].temperature[0]=8.0884;
metmast_TH[717].temperature[0]=8.0946;
metmast_TH[718].temperature[0]=8.0892;
metmast_TH[719].temperature[0]=8.0841;
metmast_TH[720].temperature[0]=8.0944;
metmast_TH[721].temperature[0]=8.0885;
metmast_TH[722].temperature[0]=8.0852;
metmast_TH[723].temperature[0]=8.0787;
metmast_TH[724].temperature[0]=8.0897;
metmast_TH[725].temperature[0]=8.0806;
metmast_TH[726].temperature[0]=8.0935;
metmast_TH[727].temperature[0]=8.0868;
metmast_TH[728].temperature[0]=8.0888;
metmast_TH[729].temperature[0]=8.0927;
metmast_TH[730].temperature[0]=8.0917;
metmast_TH[731].temperature[0]=8.098;
metmast_TH[732].temperature[0]=8.0925;
metmast_TH[733].temperature[0]=8.0972;
metmast_TH[734].temperature[0]=8.0902;
metmast_TH[735].temperature[0]=8.0873;
metmast_TH[736].temperature[0]=8.0923;
metmast_TH[737].temperature[0]=8.087;
metmast_TH[738].temperature[0]=8.0894;
metmast_TH[739].temperature[0]=8.0966;
metmast_TH[740].temperature[0]=8.0914;
metmast_TH[741].temperature[0]=8.0908;
metmast_TH[742].temperature[0]=8.0951;
metmast_TH[743].temperature[0]=8.0922;
metmast_TH[744].temperature[0]=8.0901;
metmast_TH[745].temperature[0]=8.095;
metmast_TH[746].temperature[0]=8.0924;
metmast_TH[747].temperature[0]=8.0913;
metmast_TH[748].temperature[0]=8.0898;
metmast_TH[749].temperature[0]=8.0933;
metmast_TH[750].temperature[0]=8.0951;
metmast_TH[751].temperature[0]=8.0918;
metmast_TH[752].temperature[0]=8.0885;
metmast_TH[753].temperature[0]=8.0955;
metmast_TH[754].temperature[0]=8.0888;
metmast_TH[755].temperature[0]=8.1021;
metmast_TH[756].temperature[0]=8.0974;
metmast_TH[757].temperature[0]=8.0841;
metmast_TH[758].temperature[0]=8.0906;
metmast_TH[759].temperature[0]=8.092;
metmast_TH[760].temperature[0]=8.1006;
metmast_TH[761].temperature[0]=8.0878;
metmast_TH[762].temperature[0]=8.0983;
metmast_TH[763].temperature[0]=8.095;
metmast_TH[764].temperature[0]=8.098;
metmast_TH[765].temperature[0]=8.0941;
metmast_TH[766].temperature[0]=8.0941;
metmast_TH[767].temperature[0]=8.0968;
metmast_TH[768].temperature[0]=8.105;
metmast_TH[769].temperature[0]=8.0972;
metmast_TH[770].temperature[0]=8.0972;
metmast_TH[771].temperature[0]=8.0984;
metmast_TH[772].temperature[0]=8.0996;
metmast_TH[773].temperature[0]=8.1066;
metmast_TH[774].temperature[0]=8.1022;
metmast_TH[775].temperature[0]=8.1011;
metmast_TH[776].temperature[0]=8.0964;
metmast_TH[777].temperature[0]=8.1007;
metmast_TH[778].temperature[0]=8.0988;
metmast_TH[779].temperature[0]=8.101;
metmast_TH[780].temperature[0]=8.1014;
metmast_TH[781].temperature[0]=8.1079;
metmast_TH[782].temperature[0]=8.098;
metmast_TH[783].temperature[0]=8.0962;
metmast_TH[784].temperature[0]=8.1036;
metmast_TH[785].temperature[0]=8.1034;
metmast_TH[786].temperature[0]=8.0953;
metmast_TH[787].temperature[0]=8.1041;
metmast_TH[788].temperature[0]=8.0892;
metmast_TH[789].temperature[0]=8.0946;
metmast_TH[790].temperature[0]=8.0968;
metmast_TH[791].temperature[0]=8.0957;
metmast_TH[792].temperature[0]=8.1016;
metmast_TH[793].temperature[0]=8.0936;
metmast_TH[794].temperature[0]=8.1027;
metmast_TH[795].temperature[0]=8.0958;
metmast_TH[796].temperature[0]=8.0974;
metmast_TH[797].temperature[0]=8.099;
metmast_TH[798].temperature[0]=8.1012;
metmast_TH[799].temperature[0]=8.0909;
metmast_TH[800].temperature[0]=8.0968;
metmast_TH[801].temperature[0]=8.0881;
metmast_TH[802].temperature[0]=8.0908;
metmast_TH[803].temperature[0]=8.0944;
metmast_TH[804].temperature[0]=8.0942;
metmast_TH[805].temperature[0]=8.0885;
metmast_TH[806].temperature[0]=8.0963;
metmast_TH[807].temperature[0]=8.095;
metmast_TH[808].temperature[0]=8.0923;
metmast_TH[809].temperature[0]=8.0848;
metmast_TH[810].temperature[0]=8.0869;
metmast_TH[811].temperature[0]=8.0935;
metmast_TH[812].temperature[0]=8.0958;
metmast_TH[813].temperature[0]=8.0848;
metmast_TH[814].temperature[0]=8.0873;
metmast_TH[815].temperature[0]=8.094;
metmast_TH[816].temperature[0]=8.0875;
metmast_TH[817].temperature[0]=8.0803;
metmast_TH[818].temperature[0]=8.0934;
metmast_TH[819].temperature[0]=8.0889;
metmast_TH[820].temperature[0]=8.0918;
metmast_TH[821].temperature[0]=8.0953;
metmast_TH[822].temperature[0]=8.088;
metmast_TH[823].temperature[0]=8.0897;
metmast_TH[824].temperature[0]=8.0934;
metmast_TH[825].temperature[0]=8.096;
metmast_TH[826].temperature[0]=8.0982;
metmast_TH[827].temperature[0]=8.0946;
metmast_TH[828].temperature[0]=8.0977;
metmast_TH[829].temperature[0]=8.0942;
metmast_TH[830].temperature[0]=8.0947;
metmast_TH[831].temperature[0]=8.0853;
metmast_TH[832].temperature[0]=8.101;
metmast_TH[833].temperature[0]=8.0986;
metmast_TH[834].temperature[0]=8.0912;
metmast_TH[835].temperature[0]=8.0896;
metmast_TH[836].temperature[0]=8.0957;
metmast_TH[837].temperature[0]=8.0897;
metmast_TH[838].temperature[0]=8.1027;
metmast_TH[839].temperature[0]=8.0912;
metmast_TH[840].temperature[0]=8.0919;
metmast_TH[841].temperature[0]=8.0955;
metmast_TH[842].temperature[0]=8.0928;
metmast_TH[843].temperature[0]=8.0866;
metmast_TH[844].temperature[0]=8.0888;
metmast_TH[845].temperature[0]=8.0988;
metmast_TH[846].temperature[0]=8.098;
metmast_TH[847].temperature[0]=8.092;
metmast_TH[848].temperature[0]=8.0839;
metmast_TH[849].temperature[0]=8.0967;
metmast_TH[850].temperature[0]=8.0909;
metmast_TH[851].temperature[0]=8.0925;
metmast_TH[852].temperature[0]=8.0895;
metmast_TH[853].temperature[0]=8.0933;
metmast_TH[854].temperature[0]=8.0941;
metmast_TH[855].temperature[0]=8.0896;
metmast_TH[856].temperature[0]=8.0901;
metmast_TH[857].temperature[0]=8.0953;
metmast_TH[858].temperature[0]=8.0936;
metmast_TH[859].temperature[0]=8.1016;
metmast_TH[860].temperature[0]=8.0952;
metmast_TH[861].temperature[0]=8.0956;
metmast_TH[862].temperature[0]=8.0898;
metmast_TH[863].temperature[0]=8.087;
metmast_TH[864].temperature[0]=8.0946;
metmast_TH[865].temperature[0]=8.095;
metmast_TH[866].temperature[0]=8.0923;
metmast_TH[867].temperature[0]=8.0877;
metmast_TH[868].temperature[0]=8.0907;
metmast_TH[869].temperature[0]=8.0919;
metmast_TH[870].temperature[0]=8.089;
metmast_TH[871].temperature[0]=8.0917;
metmast_TH[872].temperature[0]=8.098;
metmast_TH[873].temperature[0]=8.0895;
metmast_TH[874].temperature[0]=8.0884;
metmast_TH[875].temperature[0]=8.0874;
metmast_TH[876].temperature[0]=8.0886;
metmast_TH[877].temperature[0]=8.0866;
metmast_TH[878].temperature[0]=8.0906;
metmast_TH[879].temperature[0]=8.0963;
metmast_TH[880].temperature[0]=8.0886;
metmast_TH[881].temperature[0]=8.0945;
metmast_TH[882].temperature[0]=8.0966;
metmast_TH[883].temperature[0]=8.094;
metmast_TH[884].temperature[0]=8.1007;
metmast_TH[885].temperature[0]=8.098;
metmast_TH[886].temperature[0]=8.1033;
metmast_TH[887].temperature[0]=8.0971;
metmast_TH[888].temperature[0]=8.0958;
metmast_TH[889].temperature[0]=8.095;
metmast_TH[890].temperature[0]=8.0942;
metmast_TH[891].temperature[0]=8.1066;
metmast_TH[892].temperature[0]=8.1021;
metmast_TH[893].temperature[0]=8.1029;
metmast_TH[894].temperature[0]=8.101;
metmast_TH[895].temperature[0]=8.1085;
metmast_TH[896].temperature[0]=8.0979;
metmast_TH[897].temperature[0]=8.0958;
metmast_TH[898].temperature[0]=8.0967;
metmast_TH[899].temperature[0]=8.1089;
metmast_TH[900].temperature[0]=8.0973;
metmast_TH[901].temperature[0]=8.1014;
metmast_TH[902].temperature[0]=8.1094;
metmast_TH[903].temperature[0]=8.1016;
metmast_TH[904].temperature[0]=8.1062;
metmast_TH[905].temperature[0]=8.1005;
metmast_TH[906].temperature[0]=8.1;
metmast_TH[907].temperature[0]=8.0996;
metmast_TH[908].temperature[0]=8.0989;
metmast_TH[909].temperature[0]=8.0924;
metmast_TH[910].temperature[0]=8.0988;
metmast_TH[911].temperature[0]=8.1028;
metmast_TH[912].temperature[0]=8.0967;
metmast_TH[913].temperature[0]=8.096;
metmast_TH[914].temperature[0]=8.0953;
metmast_TH[915].temperature[0]=8.0967;
metmast_TH[916].temperature[0]=8.0969;
metmast_TH[917].temperature[0]=8.1;
metmast_TH[918].temperature[0]=8.0933;
metmast_TH[919].temperature[0]=8.092;
metmast_TH[920].temperature[0]=8.0913;
metmast_TH[921].temperature[0]=8.0916;
metmast_TH[922].temperature[0]=8.0872;
metmast_TH[923].temperature[0]=8.0875;
metmast_TH[924].temperature[0]=8.0851;
metmast_TH[925].temperature[0]=8.0967;
metmast_TH[926].temperature[0]=8.0891;
metmast_TH[927].temperature[0]=8.0955;
metmast_TH[928].temperature[0]=8.0872;
metmast_TH[929].temperature[0]=8.0953;
metmast_TH[930].temperature[0]=8.0857;
metmast_TH[931].temperature[0]=8.0892;
metmast_TH[932].temperature[0]=8.0928;
metmast_TH[933].temperature[0]=8.0905;
metmast_TH[934].temperature[0]=8.1019;
metmast_TH[935].temperature[0]=8.0992;
metmast_TH[936].temperature[0]=8.0912;
metmast_TH[937].temperature[0]=8.0953;
metmast_TH[938].temperature[0]=8.0901;
metmast_TH[939].temperature[0]=8.1006;
metmast_TH[940].temperature[0]=8.0944;
metmast_TH[941].temperature[0]=8.0922;
metmast_TH[942].temperature[0]=8.1027;
metmast_TH[943].temperature[0]=8.1052;
metmast_TH[944].temperature[0]=8.0992;
metmast_TH[945].temperature[0]=8.0989;
metmast_TH[946].temperature[0]=8.0999;
metmast_TH[947].temperature[0]=8.0973;
metmast_TH[948].temperature[0]=8.0902;
metmast_TH[949].temperature[0]=8.0972;
metmast_TH[950].temperature[0]=8.1011;
metmast_TH[951].temperature[0]=8.0961;
metmast_TH[952].temperature[0]=8.099;
metmast_TH[953].temperature[0]=8.0913;
metmast_TH[954].temperature[0]=8.0964;
metmast_TH[955].temperature[0]=8.0988;
metmast_TH[956].temperature[0]=8.1019;
metmast_TH[957].temperature[0]=8.0922;
metmast_TH[958].temperature[0]=8.1061;
metmast_TH[959].temperature[0]=8.0999;
metmast_TH[960].temperature[0]=8.0895;
metmast_TH[961].temperature[0]=8.0928;
metmast_TH[962].temperature[0]=8.1006;
metmast_TH[963].temperature[0]=8.099;
metmast_TH[964].temperature[0]=8.0962;
metmast_TH[965].temperature[0]=8.0945;
metmast_TH[966].temperature[0]=8.1068;
metmast_TH[967].temperature[0]=8.0989;
metmast_TH[968].temperature[0]=8.0931;
metmast_TH[969].temperature[0]=8.0938;
metmast_TH[970].temperature[0]=8.0947;
metmast_TH[971].temperature[0]=8.0922;
metmast_TH[972].temperature[0]=8.0909;
metmast_TH[973].temperature[0]=8.0916;
metmast_TH[974].temperature[0]=8.089;
metmast_TH[975].temperature[0]=8.0902;
metmast_TH[976].temperature[0]=8.0896;
metmast_TH[977].temperature[0]=8.0957;
metmast_TH[978].temperature[0]=8.0953;
metmast_TH[979].temperature[0]=8.0858;
metmast_TH[980].temperature[0]=8.0952;
metmast_TH[981].temperature[0]=8.0919;
metmast_TH[982].temperature[0]=8.0913;
metmast_TH[983].temperature[0]=8.0897;
metmast_TH[984].temperature[0]=8.0885;
metmast_TH[985].temperature[0]=8.0967;
metmast_TH[986].temperature[0]=8.0922;
metmast_TH[987].temperature[0]=8.0909;
metmast_TH[988].temperature[0]=8.0923;
metmast_TH[989].temperature[0]=8.0868;
metmast_TH[990].temperature[0]=8.0903;
metmast_TH[991].temperature[0]=8.0891;
metmast_TH[992].temperature[0]=8.094;
metmast_TH[993].temperature[0]=8.09;
metmast_TH[994].temperature[0]=8.089;
metmast_TH[995].temperature[0]=8.0935;
metmast_TH[996].temperature[0]=8.0933;
metmast_TH[997].temperature[0]=8.0935;
metmast_TH[998].temperature[0]=8.0912;
metmast_TH[999].temperature[0]=8.0955;
metmast_TH[0].humidity[0]=91.0755;
metmast_TH[1].humidity[0]=91.0707;
metmast_TH[2].humidity[0]=91.0665;
metmast_TH[3].humidity[0]=91.0572;
metmast_TH[4].humidity[0]=91.0466;
metmast_TH[5].humidity[0]=91.0519;
metmast_TH[6].humidity[0]=91.0523;
metmast_TH[7].humidity[0]=91.0455;
metmast_TH[8].humidity[0]=91.05;
metmast_TH[9].humidity[0]=91.0516;
metmast_TH[10].humidity[0]=91.0529;
metmast_TH[11].humidity[0]=91.0555;
metmast_TH[12].humidity[0]=91.0522;
metmast_TH[13].humidity[0]=91.0521;
metmast_TH[14].humidity[0]=91.0508;
metmast_TH[15].humidity[0]=91.0483;
metmast_TH[16].humidity[0]=91.0435;
metmast_TH[17].humidity[0]=91.0362;
metmast_TH[18].humidity[0]=91.0384;
metmast_TH[19].humidity[0]=91.0433;
metmast_TH[20].humidity[0]=91.05;
metmast_TH[21].humidity[0]=91.053;
metmast_TH[22].humidity[0]=91.066;
metmast_TH[23].humidity[0]=91.0742;
metmast_TH[24].humidity[0]=91.0788;
metmast_TH[25].humidity[0]=91.0861;
metmast_TH[26].humidity[0]=91.0961;
metmast_TH[27].humidity[0]=91.1016;
metmast_TH[28].humidity[0]=91.1177;
metmast_TH[29].humidity[0]=91.14;
metmast_TH[30].humidity[0]=91.1485;
metmast_TH[31].humidity[0]=91.1662;
metmast_TH[32].humidity[0]=91.1802;
metmast_TH[33].humidity[0]=91.1848;
metmast_TH[34].humidity[0]=91.1982;
metmast_TH[35].humidity[0]=91.201;
metmast_TH[36].humidity[0]=91.2121;
metmast_TH[37].humidity[0]=91.217;
metmast_TH[38].humidity[0]=91.2236;
metmast_TH[39].humidity[0]=91.2237;
metmast_TH[40].humidity[0]=91.2261;
metmast_TH[41].humidity[0]=91.2347;
metmast_TH[42].humidity[0]=91.2293;
metmast_TH[43].humidity[0]=91.2294;
metmast_TH[44].humidity[0]=91.232;
metmast_TH[45].humidity[0]=91.2325;
metmast_TH[46].humidity[0]=91.2346;
metmast_TH[47].humidity[0]=91.2458;
metmast_TH[48].humidity[0]=91.2541;
metmast_TH[49].humidity[0]=91.2645;
metmast_TH[50].humidity[0]=91.2702;
metmast_TH[51].humidity[0]=91.2749;
metmast_TH[52].humidity[0]=91.2782;
metmast_TH[53].humidity[0]=91.2857;
metmast_TH[54].humidity[0]=91.2871;
metmast_TH[55].humidity[0]=91.2976;
metmast_TH[56].humidity[0]=91.3012;
metmast_TH[57].humidity[0]=91.3122;
metmast_TH[58].humidity[0]=91.3064;
metmast_TH[59].humidity[0]=91.304;
metmast_TH[60].humidity[0]=91.3083;
metmast_TH[61].humidity[0]=91.3081;
metmast_TH[62].humidity[0]=91.3162;
metmast_TH[63].humidity[0]=91.3114;
metmast_TH[64].humidity[0]=91.3137;
metmast_TH[65].humidity[0]=91.3131;
metmast_TH[66].humidity[0]=91.3194;
metmast_TH[67].humidity[0]=91.318;
metmast_TH[68].humidity[0]=91.3187;
metmast_TH[69].humidity[0]=91.3111;
metmast_TH[70].humidity[0]=91.312;
metmast_TH[71].humidity[0]=91.3159;
metmast_TH[72].humidity[0]=91.3127;
metmast_TH[73].humidity[0]=91.3126;
metmast_TH[74].humidity[0]=91.3062;
metmast_TH[75].humidity[0]=91.3023;
metmast_TH[76].humidity[0]=91.3035;
metmast_TH[77].humidity[0]=91.2949;
metmast_TH[78].humidity[0]=91.2946;
metmast_TH[79].humidity[0]=91.2857;
metmast_TH[80].humidity[0]=91.2801;
metmast_TH[81].humidity[0]=91.2716;
metmast_TH[82].humidity[0]=91.2647;
metmast_TH[83].humidity[0]=91.2605;
metmast_TH[84].humidity[0]=91.2556;
metmast_TH[85].humidity[0]=91.2431;
metmast_TH[86].humidity[0]=91.2404;
metmast_TH[87].humidity[0]=91.2376;
metmast_TH[88].humidity[0]=91.2303;
metmast_TH[89].humidity[0]=91.2255;
metmast_TH[90].humidity[0]=91.2281;
metmast_TH[91].humidity[0]=91.2272;
metmast_TH[92].humidity[0]=91.223;
metmast_TH[93].humidity[0]=91.2159;
metmast_TH[94].humidity[0]=91.2149;
metmast_TH[95].humidity[0]=91.2036;
metmast_TH[96].humidity[0]=91.1975;
metmast_TH[97].humidity[0]=91.1936;
metmast_TH[98].humidity[0]=91.1982;
metmast_TH[99].humidity[0]=91.1937;
metmast_TH[100].humidity[0]=91.1931;
metmast_TH[101].humidity[0]=91.1922;
metmast_TH[102].humidity[0]=91.1899;
metmast_TH[103].humidity[0]=91.1885;
metmast_TH[104].humidity[0]=91.1862;
metmast_TH[105].humidity[0]=91.1789;
metmast_TH[106].humidity[0]=91.1743;
metmast_TH[107].humidity[0]=91.1746;
metmast_TH[108].humidity[0]=91.176;
metmast_TH[109].humidity[0]=91.1749;
metmast_TH[110].humidity[0]=91.1695;
metmast_TH[111].humidity[0]=91.1643;
metmast_TH[112].humidity[0]=91.1685;
metmast_TH[113].humidity[0]=91.1715;
metmast_TH[114].humidity[0]=91.1662;
metmast_TH[115].humidity[0]=91.1564;
metmast_TH[116].humidity[0]=91.1591;
metmast_TH[117].humidity[0]=91.1562;
metmast_TH[118].humidity[0]=91.1544;
metmast_TH[119].humidity[0]=91.1533;
metmast_TH[120].humidity[0]=91.1486;
metmast_TH[121].humidity[0]=91.1452;
metmast_TH[122].humidity[0]=91.143;
metmast_TH[123].humidity[0]=91.1413;
metmast_TH[124].humidity[0]=91.1409;
metmast_TH[125].humidity[0]=91.1353;
metmast_TH[126].humidity[0]=91.1369;
metmast_TH[127].humidity[0]=91.133;
metmast_TH[128].humidity[0]=91.1298;
metmast_TH[129].humidity[0]=91.1303;
metmast_TH[130].humidity[0]=91.134;
metmast_TH[131].humidity[0]=91.1335;
metmast_TH[132].humidity[0]=91.1279;
metmast_TH[133].humidity[0]=91.1269;
metmast_TH[134].humidity[0]=91.1191;
metmast_TH[135].humidity[0]=91.1173;
metmast_TH[136].humidity[0]=91.1148;
metmast_TH[137].humidity[0]=91.1137;
metmast_TH[138].humidity[0]=91.1126;
metmast_TH[139].humidity[0]=91.1104;
metmast_TH[140].humidity[0]=91.1104;
metmast_TH[141].humidity[0]=91.1141;
metmast_TH[142].humidity[0]=91.1147;
metmast_TH[143].humidity[0]=91.1259;
metmast_TH[144].humidity[0]=91.1332;
metmast_TH[145].humidity[0]=91.1439;
metmast_TH[146].humidity[0]=91.1478;
metmast_TH[147].humidity[0]=91.1408;
metmast_TH[148].humidity[0]=91.132;
metmast_TH[149].humidity[0]=91.1379;
metmast_TH[150].humidity[0]=91.1381;
metmast_TH[151].humidity[0]=91.1429;
metmast_TH[152].humidity[0]=91.1431;
metmast_TH[153].humidity[0]=91.1409;
metmast_TH[154].humidity[0]=91.1406;
metmast_TH[155].humidity[0]=91.1433;
metmast_TH[156].humidity[0]=91.1464;
metmast_TH[157].humidity[0]=91.1502;
metmast_TH[158].humidity[0]=91.1474;
metmast_TH[159].humidity[0]=91.1517;
metmast_TH[160].humidity[0]=91.1538;
metmast_TH[161].humidity[0]=91.1535;
metmast_TH[162].humidity[0]=91.1586;
metmast_TH[163].humidity[0]=91.1649;
metmast_TH[164].humidity[0]=91.1745;
metmast_TH[165].humidity[0]=91.1812;
metmast_TH[166].humidity[0]=91.1867;
metmast_TH[167].humidity[0]=91.1988;
metmast_TH[168].humidity[0]=91.2095;
metmast_TH[169].humidity[0]=91.2153;
metmast_TH[170].humidity[0]=91.2163;
metmast_TH[171].humidity[0]=91.2175;
metmast_TH[172].humidity[0]=91.2218;
metmast_TH[173].humidity[0]=91.2241;
metmast_TH[174].humidity[0]=91.2277;
metmast_TH[175].humidity[0]=91.2318;
metmast_TH[176].humidity[0]=91.2384;
metmast_TH[177].humidity[0]=91.2415;
metmast_TH[178].humidity[0]=91.2454;
metmast_TH[179].humidity[0]=91.2514;
metmast_TH[180].humidity[0]=91.2497;
metmast_TH[181].humidity[0]=91.2431;
metmast_TH[182].humidity[0]=91.2463;
metmast_TH[183].humidity[0]=91.2535;
metmast_TH[184].humidity[0]=91.2562;
metmast_TH[185].humidity[0]=91.2531;
metmast_TH[186].humidity[0]=91.2508;
metmast_TH[187].humidity[0]=91.2511;
metmast_TH[188].humidity[0]=91.2447;
metmast_TH[189].humidity[0]=91.2498;
metmast_TH[190].humidity[0]=91.2564;
metmast_TH[191].humidity[0]=91.259;
metmast_TH[192].humidity[0]=91.2568;
metmast_TH[193].humidity[0]=91.2578;
metmast_TH[194].humidity[0]=91.255;
metmast_TH[195].humidity[0]=91.2558;
metmast_TH[196].humidity[0]=91.2563;
metmast_TH[197].humidity[0]=91.2531;
metmast_TH[198].humidity[0]=91.2567;
metmast_TH[199].humidity[0]=91.2591;
metmast_TH[200].humidity[0]=91.2637;
metmast_TH[201].humidity[0]=91.2618;
metmast_TH[202].humidity[0]=91.2642;
metmast_TH[203].humidity[0]=91.2679;
metmast_TH[204].humidity[0]=91.2724;
metmast_TH[205].humidity[0]=91.2756;
metmast_TH[206].humidity[0]=91.2764;
metmast_TH[207].humidity[0]=91.284;
metmast_TH[208].humidity[0]=91.2888;
metmast_TH[209].humidity[0]=91.2921;
metmast_TH[210].humidity[0]=91.2926;
metmast_TH[211].humidity[0]=91.2899;
metmast_TH[212].humidity[0]=91.2896;
metmast_TH[213].humidity[0]=91.2932;
metmast_TH[214].humidity[0]=91.3021;
metmast_TH[215].humidity[0]=91.2978;
metmast_TH[216].humidity[0]=91.3027;
metmast_TH[217].humidity[0]=91.301;
metmast_TH[218].humidity[0]=91.2956;
metmast_TH[219].humidity[0]=91.2987;
metmast_TH[220].humidity[0]=91.2871;
metmast_TH[221].humidity[0]=91.2836;
metmast_TH[222].humidity[0]=91.284;
metmast_TH[223].humidity[0]=91.2841;
metmast_TH[224].humidity[0]=91.2783;
metmast_TH[225].humidity[0]=91.2723;
metmast_TH[226].humidity[0]=91.2773;
metmast_TH[227].humidity[0]=91.2758;
metmast_TH[228].humidity[0]=91.2778;
metmast_TH[229].humidity[0]=91.2763;
metmast_TH[230].humidity[0]=91.2668;
metmast_TH[231].humidity[0]=91.2684;
metmast_TH[232].humidity[0]=91.2739;
metmast_TH[233].humidity[0]=91.2727;
metmast_TH[234].humidity[0]=91.2707;
metmast_TH[235].humidity[0]=91.2706;
metmast_TH[236].humidity[0]=91.2651;
metmast_TH[237].humidity[0]=91.2658;
metmast_TH[238].humidity[0]=91.265;
metmast_TH[239].humidity[0]=91.2603;
metmast_TH[240].humidity[0]=91.254;
metmast_TH[241].humidity[0]=91.2457;
metmast_TH[242].humidity[0]=91.2439;
metmast_TH[243].humidity[0]=91.2414;
metmast_TH[244].humidity[0]=91.2308;
metmast_TH[245].humidity[0]=91.2301;
metmast_TH[246].humidity[0]=91.2257;
metmast_TH[247].humidity[0]=91.2192;
metmast_TH[248].humidity[0]=91.2157;
metmast_TH[249].humidity[0]=91.2053;
metmast_TH[250].humidity[0]=91.2022;
metmast_TH[251].humidity[0]=91.2072;
metmast_TH[252].humidity[0]=91.2089;
metmast_TH[253].humidity[0]=91.2097;
metmast_TH[254].humidity[0]=91.1994;
metmast_TH[255].humidity[0]=91.1976;
metmast_TH[256].humidity[0]=91.1966;
metmast_TH[257].humidity[0]=91.2012;
metmast_TH[258].humidity[0]=91.1987;
metmast_TH[259].humidity[0]=91.1931;
metmast_TH[260].humidity[0]=91.1971;
metmast_TH[261].humidity[0]=91.19;
metmast_TH[262].humidity[0]=91.1883;
metmast_TH[263].humidity[0]=91.1773;
metmast_TH[264].humidity[0]=91.1762;
metmast_TH[265].humidity[0]=91.1844;
metmast_TH[266].humidity[0]=91.1772;
metmast_TH[267].humidity[0]=91.1818;
metmast_TH[268].humidity[0]=91.18;
metmast_TH[269].humidity[0]=91.1809;
metmast_TH[270].humidity[0]=91.1774;
metmast_TH[271].humidity[0]=91.1749;
metmast_TH[272].humidity[0]=91.1706;
metmast_TH[273].humidity[0]=91.1688;
metmast_TH[274].humidity[0]=91.1728;
metmast_TH[275].humidity[0]=91.1684;
metmast_TH[276].humidity[0]=91.1596;
metmast_TH[277].humidity[0]=91.1544;
metmast_TH[278].humidity[0]=91.1517;
metmast_TH[279].humidity[0]=91.1519;
metmast_TH[280].humidity[0]=91.1518;
metmast_TH[281].humidity[0]=91.1474;
metmast_TH[282].humidity[0]=91.1401;
metmast_TH[283].humidity[0]=91.1258;
metmast_TH[284].humidity[0]=91.1207;
metmast_TH[285].humidity[0]=91.1188;
metmast_TH[286].humidity[0]=91.1169;
metmast_TH[287].humidity[0]=91.1153;
metmast_TH[288].humidity[0]=91.1146;
metmast_TH[289].humidity[0]=91.1143;
metmast_TH[290].humidity[0]=91.1135;
metmast_TH[291].humidity[0]=91.1131;
metmast_TH[292].humidity[0]=91.1077;
metmast_TH[293].humidity[0]=91.1055;
metmast_TH[294].humidity[0]=91.0977;
metmast_TH[295].humidity[0]=91.0931;
metmast_TH[296].humidity[0]=91.0869;
metmast_TH[297].humidity[0]=91.079;
metmast_TH[298].humidity[0]=91.0727;
metmast_TH[299].humidity[0]=91.0681;
metmast_TH[300].humidity[0]=91.0744;
metmast_TH[301].humidity[0]=91.0684;
metmast_TH[302].humidity[0]=91.0601;
metmast_TH[303].humidity[0]=91.0647;
metmast_TH[304].humidity[0]=91.06;
metmast_TH[305].humidity[0]=91.0563;
metmast_TH[306].humidity[0]=91.0527;
metmast_TH[307].humidity[0]=91.0577;
metmast_TH[308].humidity[0]=91.0549;
metmast_TH[309].humidity[0]=91.0595;
metmast_TH[310].humidity[0]=91.0576;
metmast_TH[311].humidity[0]=91.061;
metmast_TH[312].humidity[0]=91.0696;
metmast_TH[313].humidity[0]=91.0744;
metmast_TH[314].humidity[0]=91.0781;
metmast_TH[315].humidity[0]=91.08;
metmast_TH[316].humidity[0]=91.0799;
metmast_TH[317].humidity[0]=91.0844;
metmast_TH[318].humidity[0]=91.0882;
metmast_TH[319].humidity[0]=91.1044;
metmast_TH[320].humidity[0]=91.1098;
metmast_TH[321].humidity[0]=91.1202;
metmast_TH[322].humidity[0]=91.1276;
metmast_TH[323].humidity[0]=91.1396;
metmast_TH[324].humidity[0]=91.1419;
metmast_TH[325].humidity[0]=91.1507;
metmast_TH[326].humidity[0]=91.1539;
metmast_TH[327].humidity[0]=91.1594;
metmast_TH[328].humidity[0]=91.1613;
metmast_TH[329].humidity[0]=91.1665;
metmast_TH[330].humidity[0]=91.1682;
metmast_TH[331].humidity[0]=91.1752;
metmast_TH[332].humidity[0]=91.1748;
metmast_TH[333].humidity[0]=91.1854;
metmast_TH[334].humidity[0]=91.1915;
metmast_TH[335].humidity[0]=91.1973;
metmast_TH[336].humidity[0]=91.1982;
metmast_TH[337].humidity[0]=91.2039;
metmast_TH[338].humidity[0]=91.2102;
metmast_TH[339].humidity[0]=91.2131;
metmast_TH[340].humidity[0]=91.2133;
metmast_TH[341].humidity[0]=91.2097;
metmast_TH[342].humidity[0]=91.2177;
metmast_TH[343].humidity[0]=91.2117;
metmast_TH[344].humidity[0]=91.2135;
metmast_TH[345].humidity[0]=91.2121;
metmast_TH[346].humidity[0]=91.2138;
metmast_TH[347].humidity[0]=91.2165;
metmast_TH[348].humidity[0]=91.2131;
metmast_TH[349].humidity[0]=91.2083;
metmast_TH[350].humidity[0]=91.2127;
metmast_TH[351].humidity[0]=91.2054;
metmast_TH[352].humidity[0]=91.2041;
metmast_TH[353].humidity[0]=91.2052;
metmast_TH[354].humidity[0]=91.2006;
metmast_TH[355].humidity[0]=91.1953;
metmast_TH[356].humidity[0]=91.1895;
metmast_TH[357].humidity[0]=91.1833;
metmast_TH[358].humidity[0]=91.1756;
metmast_TH[359].humidity[0]=91.1733;
metmast_TH[360].humidity[0]=91.1696;
metmast_TH[361].humidity[0]=91.1602;
metmast_TH[362].humidity[0]=91.1614;
metmast_TH[363].humidity[0]=91.1529;
metmast_TH[364].humidity[0]=91.1528;
metmast_TH[365].humidity[0]=91.1446;
metmast_TH[366].humidity[0]=91.1472;
metmast_TH[367].humidity[0]=91.1423;
metmast_TH[368].humidity[0]=91.1374;
metmast_TH[369].humidity[0]=91.1319;
metmast_TH[370].humidity[0]=91.1214;
metmast_TH[371].humidity[0]=91.1158;
metmast_TH[372].humidity[0]=91.1138;
metmast_TH[373].humidity[0]=91.1104;
metmast_TH[374].humidity[0]=91.1033;
metmast_TH[375].humidity[0]=91.1013;
metmast_TH[376].humidity[0]=91.097;
metmast_TH[377].humidity[0]=91.0976;
metmast_TH[378].humidity[0]=91.0888;
metmast_TH[379].humidity[0]=91.0827;
metmast_TH[380].humidity[0]=91.0823;
metmast_TH[381].humidity[0]=91.0747;
metmast_TH[382].humidity[0]=91.0709;
metmast_TH[383].humidity[0]=91.0764;
metmast_TH[384].humidity[0]=91.0723;
metmast_TH[385].humidity[0]=91.0689;
metmast_TH[386].humidity[0]=91.0714;
metmast_TH[387].humidity[0]=91.0754;
metmast_TH[388].humidity[0]=91.0751;
metmast_TH[389].humidity[0]=91.0782;
metmast_TH[390].humidity[0]=91.0777;
metmast_TH[391].humidity[0]=91.0804;
metmast_TH[392].humidity[0]=91.0786;
metmast_TH[393].humidity[0]=91.0827;
metmast_TH[394].humidity[0]=91.0893;
metmast_TH[395].humidity[0]=91.0909;
metmast_TH[396].humidity[0]=91.0921;
metmast_TH[397].humidity[0]=91.0876;
metmast_TH[398].humidity[0]=91.081;
metmast_TH[399].humidity[0]=91.0812;
metmast_TH[400].humidity[0]=91.0797;
metmast_TH[401].humidity[0]=91.0777;
metmast_TH[402].humidity[0]=91.075;
metmast_TH[403].humidity[0]=91.0738;
metmast_TH[404].humidity[0]=91.0729;
metmast_TH[405].humidity[0]=91.0732;
metmast_TH[406].humidity[0]=91.0753;
metmast_TH[407].humidity[0]=91.0729;
metmast_TH[408].humidity[0]=91.0721;
metmast_TH[409].humidity[0]=91.0732;
metmast_TH[410].humidity[0]=91.0649;
metmast_TH[411].humidity[0]=91.0651;
metmast_TH[412].humidity[0]=91.0623;
metmast_TH[413].humidity[0]=91.0668;
metmast_TH[414].humidity[0]=91.0616;
metmast_TH[415].humidity[0]=91.0566;
metmast_TH[416].humidity[0]=91.0496;
metmast_TH[417].humidity[0]=91.045;
metmast_TH[418].humidity[0]=91.0425;
metmast_TH[419].humidity[0]=91.043;
metmast_TH[420].humidity[0]=91.0335;
metmast_TH[421].humidity[0]=91.0334;
metmast_TH[422].humidity[0]=91.0277;
metmast_TH[423].humidity[0]=91.0273;
metmast_TH[424].humidity[0]=91.0209;
metmast_TH[425].humidity[0]=91.0198;
metmast_TH[426].humidity[0]=91.0174;
metmast_TH[427].humidity[0]=91.0159;
metmast_TH[428].humidity[0]=91.0136;
metmast_TH[429].humidity[0]=91.0156;
metmast_TH[430].humidity[0]=91.0124;
metmast_TH[431].humidity[0]=91.0053;
metmast_TH[432].humidity[0]=91.0054;
metmast_TH[433].humidity[0]=90.9991;
metmast_TH[434].humidity[0]=90.9986;
metmast_TH[435].humidity[0]=90.9974;
metmast_TH[436].humidity[0]=90.9921;
metmast_TH[437].humidity[0]=90.9947;
metmast_TH[438].humidity[0]=90.9974;
metmast_TH[439].humidity[0]=90.9905;
metmast_TH[440].humidity[0]=90.9891;
metmast_TH[441].humidity[0]=90.9875;
metmast_TH[442].humidity[0]=90.9738;
metmast_TH[443].humidity[0]=90.9642;
metmast_TH[444].humidity[0]=90.9676;
metmast_TH[445].humidity[0]=90.9615;
metmast_TH[446].humidity[0]=90.9589;
metmast_TH[447].humidity[0]=90.9548;
metmast_TH[448].humidity[0]=90.9488;
metmast_TH[449].humidity[0]=90.9437;
metmast_TH[450].humidity[0]=90.9484;
metmast_TH[451].humidity[0]=90.9533;
metmast_TH[452].humidity[0]=90.9528;
metmast_TH[453].humidity[0]=90.9517;
metmast_TH[454].humidity[0]=90.949;
metmast_TH[455].humidity[0]=90.9534;
metmast_TH[456].humidity[0]=90.9485;
metmast_TH[457].humidity[0]=90.9509;
metmast_TH[458].humidity[0]=90.9516;
metmast_TH[459].humidity[0]=90.9534;
metmast_TH[460].humidity[0]=90.9584;
metmast_TH[461].humidity[0]=90.9616;
metmast_TH[462].humidity[0]=90.9595;
metmast_TH[463].humidity[0]=90.9622;
metmast_TH[464].humidity[0]=90.9629;
metmast_TH[465].humidity[0]=90.9642;
metmast_TH[466].humidity[0]=90.9644;
metmast_TH[467].humidity[0]=90.9577;
metmast_TH[468].humidity[0]=90.9621;
metmast_TH[469].humidity[0]=90.9625;
metmast_TH[470].humidity[0]=90.9595;
metmast_TH[471].humidity[0]=90.9554;
metmast_TH[472].humidity[0]=90.9572;
metmast_TH[473].humidity[0]=90.9509;
metmast_TH[474].humidity[0]=90.9565;
metmast_TH[475].humidity[0]=90.9496;
metmast_TH[476].humidity[0]=90.9463;
metmast_TH[477].humidity[0]=90.9463;
metmast_TH[478].humidity[0]=90.9457;
metmast_TH[479].humidity[0]=90.942;
metmast_TH[480].humidity[0]=90.9481;
metmast_TH[481].humidity[0]=90.9434;
metmast_TH[482].humidity[0]=90.9466;
metmast_TH[483].humidity[0]=90.947;
metmast_TH[484].humidity[0]=90.9429;
metmast_TH[485].humidity[0]=90.9411;
metmast_TH[486].humidity[0]=90.9374;
metmast_TH[487].humidity[0]=90.938;
metmast_TH[488].humidity[0]=90.9474;
metmast_TH[489].humidity[0]=90.9441;
metmast_TH[490].humidity[0]=90.9435;
metmast_TH[491].humidity[0]=90.9457;
metmast_TH[492].humidity[0]=90.9521;
metmast_TH[493].humidity[0]=90.9577;
metmast_TH[494].humidity[0]=90.9622;
metmast_TH[495].humidity[0]=90.9741;
metmast_TH[496].humidity[0]=90.9847;
metmast_TH[497].humidity[0]=90.9855;
metmast_TH[498].humidity[0]=90.9984;
metmast_TH[499].humidity[0]=91.0062;
metmast_TH[500].humidity[0]=91.0097;
metmast_TH[501].humidity[0]=91.015;
metmast_TH[502].humidity[0]=91.0203;
metmast_TH[503].humidity[0]=91.0261;
metmast_TH[504].humidity[0]=91.0341;
metmast_TH[505].humidity[0]=91.0341;
metmast_TH[506].humidity[0]=91.0414;
metmast_TH[507].humidity[0]=91.0406;
metmast_TH[508].humidity[0]=91.0444;
metmast_TH[509].humidity[0]=91.0443;
metmast_TH[510].humidity[0]=91.0529;
metmast_TH[511].humidity[0]=91.0604;
metmast_TH[512].humidity[0]=91.0589;
metmast_TH[513].humidity[0]=91.0627;
metmast_TH[514].humidity[0]=91.0657;
metmast_TH[515].humidity[0]=91.0707;
metmast_TH[516].humidity[0]=91.0722;
metmast_TH[517].humidity[0]=91.0678;
metmast_TH[518].humidity[0]=91.0754;
metmast_TH[519].humidity[0]=91.0729;
metmast_TH[520].humidity[0]=91.0727;
metmast_TH[521].humidity[0]=91.075;
metmast_TH[522].humidity[0]=91.0754;
metmast_TH[523].humidity[0]=91.076;
metmast_TH[524].humidity[0]=91.0806;
metmast_TH[525].humidity[0]=91.0812;
metmast_TH[526].humidity[0]=91.0788;
metmast_TH[527].humidity[0]=91.0773;
metmast_TH[528].humidity[0]=91.0728;
metmast_TH[529].humidity[0]=91.0761;
metmast_TH[530].humidity[0]=91.0761;
metmast_TH[531].humidity[0]=91.0789;
metmast_TH[532].humidity[0]=91.0809;
metmast_TH[533].humidity[0]=91.0823;
metmast_TH[534].humidity[0]=91.0783;
metmast_TH[535].humidity[0]=91.0767;
metmast_TH[536].humidity[0]=91.0765;
metmast_TH[537].humidity[0]=91.0782;
metmast_TH[538].humidity[0]=91.075;
metmast_TH[539].humidity[0]=91.0734;
metmast_TH[540].humidity[0]=91.0707;
metmast_TH[541].humidity[0]=91.0749;
metmast_TH[542].humidity[0]=91.0729;
metmast_TH[543].humidity[0]=91.0722;
metmast_TH[544].humidity[0]=91.0668;
metmast_TH[545].humidity[0]=91.0742;
metmast_TH[546].humidity[0]=91.0765;
metmast_TH[547].humidity[0]=91.0789;
metmast_TH[548].humidity[0]=91.0806;
metmast_TH[549].humidity[0]=91.0783;
metmast_TH[550].humidity[0]=91.0788;
metmast_TH[551].humidity[0]=91.0851;
metmast_TH[552].humidity[0]=91.0826;
metmast_TH[553].humidity[0]=91.0841;
metmast_TH[554].humidity[0]=91.0916;
metmast_TH[555].humidity[0]=91.0893;
metmast_TH[556].humidity[0]=91.0937;
metmast_TH[557].humidity[0]=91.0939;
metmast_TH[558].humidity[0]=91.0963;
metmast_TH[559].humidity[0]=91.0919;
metmast_TH[560].humidity[0]=91.0899;
metmast_TH[561].humidity[0]=91.0895;
metmast_TH[562].humidity[0]=91.09;
metmast_TH[563].humidity[0]=91.09;
metmast_TH[564].humidity[0]=91.0883;
metmast_TH[565].humidity[0]=91.0876;
metmast_TH[566].humidity[0]=91.083;
metmast_TH[567].humidity[0]=91.0832;
metmast_TH[568].humidity[0]=91.0887;
metmast_TH[569].humidity[0]=91.088;
metmast_TH[570].humidity[0]=91.083;
metmast_TH[571].humidity[0]=91.0839;
metmast_TH[572].humidity[0]=91.0837;
metmast_TH[573].humidity[0]=91.0875;
metmast_TH[574].humidity[0]=91.0867;
metmast_TH[575].humidity[0]=91.092;
metmast_TH[576].humidity[0]=91.0936;
metmast_TH[577].humidity[0]=91.0928;
metmast_TH[578].humidity[0]=91.0961;
metmast_TH[579].humidity[0]=91.0981;
metmast_TH[580].humidity[0]=91.0914;
metmast_TH[581].humidity[0]=91.1029;
metmast_TH[582].humidity[0]=91.1002;
metmast_TH[583].humidity[0]=91.0949;
metmast_TH[584].humidity[0]=91.0948;
metmast_TH[585].humidity[0]=91.0967;
metmast_TH[586].humidity[0]=91.0976;
metmast_TH[587].humidity[0]=91.0927;
metmast_TH[588].humidity[0]=91.0851;
metmast_TH[589].humidity[0]=91.086;
metmast_TH[590].humidity[0]=91.0797;
metmast_TH[591].humidity[0]=91.0793;
metmast_TH[592].humidity[0]=91.075;
metmast_TH[593].humidity[0]=91.0731;
metmast_TH[594].humidity[0]=91.0734;
metmast_TH[595].humidity[0]=91.0776;
metmast_TH[596].humidity[0]=91.0734;
metmast_TH[597].humidity[0]=91.0689;
metmast_TH[598].humidity[0]=91.0716;
metmast_TH[599].humidity[0]=91.0698;
metmast_TH[600].humidity[0]=91.0596;
metmast_TH[601].humidity[0]=91.0679;
metmast_TH[602].humidity[0]=91.0611;
metmast_TH[603].humidity[0]=91.0584;
metmast_TH[604].humidity[0]=91.0627;
metmast_TH[605].humidity[0]=91.0578;
metmast_TH[606].humidity[0]=91.0591;
metmast_TH[607].humidity[0]=91.0501;
metmast_TH[608].humidity[0]=91.0495;
metmast_TH[609].humidity[0]=91.0544;
metmast_TH[610].humidity[0]=91.0556;
metmast_TH[611].humidity[0]=91.0582;
metmast_TH[612].humidity[0]=91.0673;
metmast_TH[613].humidity[0]=91.0629;
metmast_TH[614].humidity[0]=91.0527;
metmast_TH[615].humidity[0]=91.0546;
metmast_TH[616].humidity[0]=91.0591;
metmast_TH[617].humidity[0]=91.06;
metmast_TH[618].humidity[0]=91.0632;
metmast_TH[619].humidity[0]=91.0541;
metmast_TH[620].humidity[0]=91.062;
metmast_TH[621].humidity[0]=91.0546;
metmast_TH[622].humidity[0]=91.0501;
metmast_TH[623].humidity[0]=91.0462;
metmast_TH[624].humidity[0]=91.0443;
metmast_TH[625].humidity[0]=91.0475;
metmast_TH[626].humidity[0]=91.0479;
metmast_TH[627].humidity[0]=91.0455;
metmast_TH[628].humidity[0]=91.0469;
metmast_TH[629].humidity[0]=91.0379;
metmast_TH[630].humidity[0]=91.0374;
metmast_TH[631].humidity[0]=91.0333;
metmast_TH[632].humidity[0]=91.0336;
metmast_TH[633].humidity[0]=91.0339;
metmast_TH[634].humidity[0]=91.0268;
metmast_TH[635].humidity[0]=91.0289;
metmast_TH[636].humidity[0]=91.0323;
metmast_TH[637].humidity[0]=91.0289;
metmast_TH[638].humidity[0]=91.0309;
metmast_TH[639].humidity[0]=91.0307;
metmast_TH[640].humidity[0]=91.0349;
metmast_TH[641].humidity[0]=91.0336;
metmast_TH[642].humidity[0]=91.039;
metmast_TH[643].humidity[0]=91.0438;
metmast_TH[644].humidity[0]=91.0454;
metmast_TH[645].humidity[0]=91.0479;
metmast_TH[646].humidity[0]=91.0485;
metmast_TH[647].humidity[0]=91.0541;
metmast_TH[648].humidity[0]=91.0651;
metmast_TH[649].humidity[0]=91.0612;
metmast_TH[650].humidity[0]=91.0715;
metmast_TH[651].humidity[0]=91.0747;
metmast_TH[652].humidity[0]=91.0775;
metmast_TH[653].humidity[0]=91.0815;
metmast_TH[654].humidity[0]=91.0875;
metmast_TH[655].humidity[0]=91.0894;
metmast_TH[656].humidity[0]=91.0881;
metmast_TH[657].humidity[0]=91.0851;
metmast_TH[658].humidity[0]=91.0872;
metmast_TH[659].humidity[0]=91.087;
metmast_TH[660].humidity[0]=91.0844;
metmast_TH[661].humidity[0]=91.0854;
metmast_TH[662].humidity[0]=91.0839;
metmast_TH[663].humidity[0]=91.0853;
metmast_TH[664].humidity[0]=91.0817;
metmast_TH[665].humidity[0]=91.0811;
metmast_TH[666].humidity[0]=91.079;
metmast_TH[667].humidity[0]=91.0819;
metmast_TH[668].humidity[0]=91.0858;
metmast_TH[669].humidity[0]=91.0787;
metmast_TH[670].humidity[0]=91.0832;
metmast_TH[671].humidity[0]=91.0805;
metmast_TH[672].humidity[0]=91.0776;
metmast_TH[673].humidity[0]=91.0792;
metmast_TH[674].humidity[0]=91.075;
metmast_TH[675].humidity[0]=91.0745;
metmast_TH[676].humidity[0]=91.0747;
metmast_TH[677].humidity[0]=91.0732;
metmast_TH[678].humidity[0]=91.0718;
metmast_TH[679].humidity[0]=91.0717;
metmast_TH[680].humidity[0]=91.072;
metmast_TH[681].humidity[0]=91.0584;
metmast_TH[682].humidity[0]=91.0529;
metmast_TH[683].humidity[0]=91.0585;
metmast_TH[684].humidity[0]=91.0502;
metmast_TH[685].humidity[0]=91.0451;
metmast_TH[686].humidity[0]=91.0555;
metmast_TH[687].humidity[0]=91.0441;
metmast_TH[688].humidity[0]=91.0394;
metmast_TH[689].humidity[0]=91.0364;
metmast_TH[690].humidity[0]=91.0285;
metmast_TH[691].humidity[0]=91.0316;
metmast_TH[692].humidity[0]=91.0247;
metmast_TH[693].humidity[0]=91.0237;
metmast_TH[694].humidity[0]=91.0159;
metmast_TH[695].humidity[0]=91.0154;
metmast_TH[696].humidity[0]=91.0119;
metmast_TH[697].humidity[0]=91.0099;
metmast_TH[698].humidity[0]=91.0108;
metmast_TH[699].humidity[0]=91.0024;
metmast_TH[700].humidity[0]=90.9965;
metmast_TH[701].humidity[0]=90.9883;
metmast_TH[702].humidity[0]=90.9859;
metmast_TH[703].humidity[0]=90.9892;
metmast_TH[704].humidity[0]=90.9832;
metmast_TH[705].humidity[0]=90.9802;
metmast_TH[706].humidity[0]=90.9737;
metmast_TH[707].humidity[0]=90.9732;
metmast_TH[708].humidity[0]=90.9782;
metmast_TH[709].humidity[0]=90.9708;
metmast_TH[710].humidity[0]=90.9675;
metmast_TH[711].humidity[0]=90.9639;
metmast_TH[712].humidity[0]=90.9627;
metmast_TH[713].humidity[0]=90.96;
metmast_TH[714].humidity[0]=90.9605;
metmast_TH[715].humidity[0]=90.9587;
metmast_TH[716].humidity[0]=90.9642;
metmast_TH[717].humidity[0]=90.9687;
metmast_TH[718].humidity[0]=90.9683;
metmast_TH[719].humidity[0]=90.9662;
metmast_TH[720].humidity[0]=90.9734;
metmast_TH[721].humidity[0]=90.9732;
metmast_TH[722].humidity[0]=90.9791;
metmast_TH[723].humidity[0]=90.9822;
metmast_TH[724].humidity[0]=90.9916;
metmast_TH[725].humidity[0]=90.9942;
metmast_TH[726].humidity[0]=91.0038;
metmast_TH[727].humidity[0]=91.0078;
metmast_TH[728].humidity[0]=91.0047;
metmast_TH[729].humidity[0]=91.0087;
metmast_TH[730].humidity[0]=91.0135;
metmast_TH[731].humidity[0]=91.0114;
metmast_TH[732].humidity[0]=91.0099;
metmast_TH[733].humidity[0]=91.0136;
metmast_TH[734].humidity[0]=91.0146;
metmast_TH[735].humidity[0]=91.0234;
metmast_TH[736].humidity[0]=91.0184;
metmast_TH[737].humidity[0]=91.0309;
metmast_TH[738].humidity[0]=91.0374;
metmast_TH[739].humidity[0]=91.0473;
metmast_TH[740].humidity[0]=91.0627;
metmast_TH[741].humidity[0]=91.0682;
metmast_TH[742].humidity[0]=91.0732;
metmast_TH[743].humidity[0]=91.0775;
metmast_TH[744].humidity[0]=91.0855;
metmast_TH[745].humidity[0]=91.0877;
metmast_TH[746].humidity[0]=91.0933;
metmast_TH[747].humidity[0]=91.0924;
metmast_TH[748].humidity[0]=91.0869;
metmast_TH[749].humidity[0]=91.0976;
metmast_TH[750].humidity[0]=91.1005;
metmast_TH[751].humidity[0]=91.0988;
metmast_TH[752].humidity[0]=91.0964;
metmast_TH[753].humidity[0]=91.0876;
metmast_TH[754].humidity[0]=91.0959;
metmast_TH[755].humidity[0]=91.0971;
metmast_TH[756].humidity[0]=91.0945;
metmast_TH[757].humidity[0]=91.0867;
metmast_TH[758].humidity[0]=91.0804;
metmast_TH[759].humidity[0]=91.0801;
metmast_TH[760].humidity[0]=91.0764;
metmast_TH[761].humidity[0]=91.0682;
metmast_TH[762].humidity[0]=91.0688;
metmast_TH[763].humidity[0]=91.061;
metmast_TH[764].humidity[0]=91.0551;
metmast_TH[765].humidity[0]=91.0506;
metmast_TH[766].humidity[0]=91.0484;
metmast_TH[767].humidity[0]=91.0372;
metmast_TH[768].humidity[0]=91.0329;
metmast_TH[769].humidity[0]=91.0247;
metmast_TH[770].humidity[0]=91.03;
metmast_TH[771].humidity[0]=91.0202;
metmast_TH[772].humidity[0]=91.0167;
metmast_TH[773].humidity[0]=91.0115;
metmast_TH[774].humidity[0]=91.0092;
metmast_TH[775].humidity[0]=90.9996;
metmast_TH[776].humidity[0]=90.9899;
metmast_TH[777].humidity[0]=90.9875;
metmast_TH[778].humidity[0]=90.9931;
metmast_TH[779].humidity[0]=90.9886;
metmast_TH[780].humidity[0]=90.9883;
metmast_TH[781].humidity[0]=90.9882;
metmast_TH[782].humidity[0]=90.9883;
metmast_TH[783].humidity[0]=90.9843;
metmast_TH[784].humidity[0]=90.9863;
metmast_TH[785].humidity[0]=90.9814;
metmast_TH[786].humidity[0]=90.9727;
metmast_TH[787].humidity[0]=90.9699;
metmast_TH[788].humidity[0]=90.9675;
metmast_TH[789].humidity[0]=90.9645;
metmast_TH[790].humidity[0]=90.9568;
metmast_TH[791].humidity[0]=90.9577;
metmast_TH[792].humidity[0]=90.9609;
metmast_TH[793].humidity[0]=90.9571;
metmast_TH[794].humidity[0]=90.9545;
metmast_TH[795].humidity[0]=90.9557;
metmast_TH[796].humidity[0]=90.9571;
metmast_TH[797].humidity[0]=90.9626;
metmast_TH[798].humidity[0]=90.9551;
metmast_TH[799].humidity[0]=90.9572;
metmast_TH[800].humidity[0]=90.957;
metmast_TH[801].humidity[0]=90.9579;
metmast_TH[802].humidity[0]=90.9595;
metmast_TH[803].humidity[0]=90.9612;
metmast_TH[804].humidity[0]=90.9544;
metmast_TH[805].humidity[0]=90.9495;
metmast_TH[806].humidity[0]=90.9531;
metmast_TH[807].humidity[0]=90.9546;
metmast_TH[808].humidity[0]=90.9494;
metmast_TH[809].humidity[0]=90.9478;
metmast_TH[810].humidity[0]=90.944;
metmast_TH[811].humidity[0]=90.9271;
metmast_TH[812].humidity[0]=90.9343;
metmast_TH[813].humidity[0]=90.9346;
metmast_TH[814].humidity[0]=90.9277;
metmast_TH[815].humidity[0]=90.9249;
metmast_TH[816].humidity[0]=90.9214;
metmast_TH[817].humidity[0]=90.9222;
metmast_TH[818].humidity[0]=90.9179;
metmast_TH[819].humidity[0]=90.9131;
metmast_TH[820].humidity[0]=90.909;
metmast_TH[821].humidity[0]=90.9122;
metmast_TH[822].humidity[0]=90.9173;
metmast_TH[823].humidity[0]=90.9179;
metmast_TH[824].humidity[0]=90.9185;
metmast_TH[825].humidity[0]=90.9222;
metmast_TH[826].humidity[0]=90.9156;
metmast_TH[827].humidity[0]=90.9162;
metmast_TH[828].humidity[0]=90.9189;
metmast_TH[829].humidity[0]=90.9192;
metmast_TH[830].humidity[0]=90.92;
metmast_TH[831].humidity[0]=90.9275;
metmast_TH[832].humidity[0]=90.9216;
metmast_TH[833].humidity[0]=90.9257;
metmast_TH[834].humidity[0]=90.9203;
metmast_TH[835].humidity[0]=90.9249;
metmast_TH[836].humidity[0]=90.9253;
metmast_TH[837].humidity[0]=90.9249;
metmast_TH[838].humidity[0]=90.9343;
metmast_TH[839].humidity[0]=90.9291;
metmast_TH[840].humidity[0]=90.92;
metmast_TH[841].humidity[0]=90.9247;
metmast_TH[842].humidity[0]=90.9267;
metmast_TH[843].humidity[0]=90.9316;
metmast_TH[844].humidity[0]=90.9318;
metmast_TH[845].humidity[0]=90.9304;
metmast_TH[846].humidity[0]=90.9328;
metmast_TH[847].humidity[0]=90.9355;
metmast_TH[848].humidity[0]=90.9369;
metmast_TH[849].humidity[0]=90.9313;
metmast_TH[850].humidity[0]=90.9377;
metmast_TH[851].humidity[0]=90.9429;
metmast_TH[852].humidity[0]=90.9368;
metmast_TH[853].humidity[0]=90.9416;
metmast_TH[854].humidity[0]=90.9361;
metmast_TH[855].humidity[0]=90.9357;
metmast_TH[856].humidity[0]=90.9349;
metmast_TH[857].humidity[0]=90.9426;
metmast_TH[858].humidity[0]=90.9329;
metmast_TH[859].humidity[0]=90.9361;
metmast_TH[860].humidity[0]=90.933;
metmast_TH[861].humidity[0]=90.9417;
metmast_TH[862].humidity[0]=90.9388;
metmast_TH[863].humidity[0]=90.9418;
metmast_TH[864].humidity[0]=90.9444;
metmast_TH[865].humidity[0]=90.9401;
metmast_TH[866].humidity[0]=90.94;
metmast_TH[867].humidity[0]=90.9307;
metmast_TH[868].humidity[0]=90.9262;
metmast_TH[869].humidity[0]=90.9257;
metmast_TH[870].humidity[0]=90.9289;
metmast_TH[871].humidity[0]=90.9291;
metmast_TH[872].humidity[0]=90.9308;
metmast_TH[873].humidity[0]=90.9246;
metmast_TH[874].humidity[0]=90.9197;
metmast_TH[875].humidity[0]=90.9234;
metmast_TH[876].humidity[0]=90.9234;
metmast_TH[877].humidity[0]=90.9212;
metmast_TH[878].humidity[0]=90.9247;
metmast_TH[879].humidity[0]=90.9252;
metmast_TH[880].humidity[0]=90.9218;
metmast_TH[881].humidity[0]=90.9189;
metmast_TH[882].humidity[0]=90.9206;
metmast_TH[883].humidity[0]=90.9124;
metmast_TH[884].humidity[0]=90.9105;
metmast_TH[885].humidity[0]=90.9118;
metmast_TH[886].humidity[0]=90.9144;
metmast_TH[887].humidity[0]=90.9211;
metmast_TH[888].humidity[0]=90.9246;
metmast_TH[889].humidity[0]=90.9296;
metmast_TH[890].humidity[0]=90.9278;
metmast_TH[891].humidity[0]=90.9349;
metmast_TH[892].humidity[0]=90.9387;
metmast_TH[893].humidity[0]=90.9424;
metmast_TH[894].humidity[0]=90.9424;
metmast_TH[895].humidity[0]=90.9462;
metmast_TH[896].humidity[0]=90.9437;
metmast_TH[897].humidity[0]=90.947;
metmast_TH[898].humidity[0]=90.9476;
metmast_TH[899].humidity[0]=90.9537;
metmast_TH[900].humidity[0]=90.9532;
metmast_TH[901].humidity[0]=90.9555;
metmast_TH[902].humidity[0]=90.9516;
metmast_TH[903].humidity[0]=90.9522;
metmast_TH[904].humidity[0]=90.9503;
metmast_TH[905].humidity[0]=90.9509;
metmast_TH[906].humidity[0]=90.952;
metmast_TH[907].humidity[0]=90.9495;
metmast_TH[908].humidity[0]=90.9473;
metmast_TH[909].humidity[0]=90.9494;
metmast_TH[910].humidity[0]=90.9527;
metmast_TH[911].humidity[0]=90.9494;
metmast_TH[912].humidity[0]=90.9493;
metmast_TH[913].humidity[0]=90.9489;
metmast_TH[914].humidity[0]=90.9489;
metmast_TH[915].humidity[0]=90.9441;
metmast_TH[916].humidity[0]=90.9395;
metmast_TH[917].humidity[0]=90.9345;
metmast_TH[918].humidity[0]=90.9269;
metmast_TH[919].humidity[0]=90.9205;
metmast_TH[920].humidity[0]=90.9227;
metmast_TH[921].humidity[0]=90.9216;
metmast_TH[922].humidity[0]=90.914;
metmast_TH[923].humidity[0]=90.9083;
metmast_TH[924].humidity[0]=90.9074;
metmast_TH[925].humidity[0]=90.9072;
metmast_TH[926].humidity[0]=90.9006;
metmast_TH[927].humidity[0]=90.8956;
metmast_TH[928].humidity[0]=90.8973;
metmast_TH[929].humidity[0]=90.8904;
metmast_TH[930].humidity[0]=90.8888;
metmast_TH[931].humidity[0]=90.8762;
metmast_TH[932].humidity[0]=90.8815;
metmast_TH[933].humidity[0]=90.8759;
metmast_TH[934].humidity[0]=90.8726;
metmast_TH[935].humidity[0]=90.8672;
metmast_TH[936].humidity[0]=90.8705;
metmast_TH[937].humidity[0]=90.861;
metmast_TH[938].humidity[0]=90.8602;
metmast_TH[939].humidity[0]=90.8555;
metmast_TH[940].humidity[0]=90.8567;
metmast_TH[941].humidity[0]=90.8482;
metmast_TH[942].humidity[0]=90.8461;
metmast_TH[943].humidity[0]=90.8451;
metmast_TH[944].humidity[0]=90.8445;
metmast_TH[945].humidity[0]=90.8415;
metmast_TH[946].humidity[0]=90.8376;
metmast_TH[947].humidity[0]=90.8371;
metmast_TH[948].humidity[0]=90.8404;
metmast_TH[949].humidity[0]=90.8383;
metmast_TH[950].humidity[0]=90.8345;
metmast_TH[951].humidity[0]=90.8367;
metmast_TH[952].humidity[0]=90.8406;
metmast_TH[953].humidity[0]=90.8345;
metmast_TH[954].humidity[0]=90.8414;
metmast_TH[955].humidity[0]=90.8415;
metmast_TH[956].humidity[0]=90.8426;
metmast_TH[957].humidity[0]=90.839;
metmast_TH[958].humidity[0]=90.8428;
metmast_TH[959].humidity[0]=90.8451;
metmast_TH[960].humidity[0]=90.8464;
metmast_TH[961].humidity[0]=90.8493;
metmast_TH[962].humidity[0]=90.8482;
metmast_TH[963].humidity[0]=90.8489;
metmast_TH[964].humidity[0]=90.8566;
metmast_TH[965].humidity[0]=90.858;
metmast_TH[966].humidity[0]=90.8565;
metmast_TH[967].humidity[0]=90.8599;
metmast_TH[968].humidity[0]=90.8643;
metmast_TH[969].humidity[0]=90.8594;
metmast_TH[970].humidity[0]=90.8648;
metmast_TH[971].humidity[0]=90.8589;
metmast_TH[972].humidity[0]=90.8621;
metmast_TH[973].humidity[0]=90.8608;
metmast_TH[974].humidity[0]=90.8572;
metmast_TH[975].humidity[0]=90.8489;
metmast_TH[976].humidity[0]=90.8476;
metmast_TH[977].humidity[0]=90.8487;
metmast_TH[978].humidity[0]=90.845;
metmast_TH[979].humidity[0]=90.8428;
metmast_TH[980].humidity[0]=90.8429;
metmast_TH[981].humidity[0]=90.8455;
metmast_TH[982].humidity[0]=90.8412;
metmast_TH[983].humidity[0]=90.8395;
metmast_TH[984].humidity[0]=90.8365;
metmast_TH[985].humidity[0]=90.8377;
metmast_TH[986].humidity[0]=90.8316;
metmast_TH[987].humidity[0]=90.8311;
metmast_TH[988].humidity[0]=90.8255;
metmast_TH[989].humidity[0]=90.8327;
metmast_TH[990].humidity[0]=90.8284;
metmast_TH[991].humidity[0]=90.8224;
metmast_TH[992].humidity[0]=90.8221;
metmast_TH[993].humidity[0]=90.8232;
metmast_TH[994].humidity[0]=90.8166;
metmast_TH[995].humidity[0]=90.8182;
metmast_TH[996].humidity[0]=90.8129;
metmast_TH[997].humidity[0]=90.8123;
metmast_TH[998].humidity[0]=90.8145;
metmast_TH[999].humidity[0]=90.8085;
metmast_TH[0].pressure[0]=1003.7341;
metmast_TH[1].pressure[0]=1003.7414;
metmast_TH[2].pressure[0]=1003.6953;
metmast_TH[3].pressure[0]=1003.6521;
metmast_TH[4].pressure[0]=1003.6382;
metmast_TH[5].pressure[0]=1003.5979;
metmast_TH[6].pressure[0]=1003.6001;
metmast_TH[7].pressure[0]=1003.6755;
metmast_TH[8].pressure[0]=1003.6616;
metmast_TH[9].pressure[0]=1003.6286;
metmast_TH[10].pressure[0]=1003.6719;
metmast_TH[11].pressure[0]=1003.677;
metmast_TH[12].pressure[0]=1003.6213;
metmast_TH[13].pressure[0]=1003.5927;
metmast_TH[14].pressure[0]=1003.6741;
metmast_TH[15].pressure[0]=1003.622;
metmast_TH[16].pressure[0]=1003.6081;
metmast_TH[17].pressure[0]=1003.5891;
metmast_TH[18].pressure[0]=1003.4917;
metmast_TH[19].pressure[0]=1003.4865;
metmast_TH[20].pressure[0]=1003.5349;
metmast_TH[21].pressure[0]=1003.5678;
metmast_TH[22].pressure[0]=1003.5334;
metmast_TH[23].pressure[0]=1003.4719;
metmast_TH[24].pressure[0]=1003.4807;
metmast_TH[25].pressure[0]=1003.4067;
metmast_TH[26].pressure[0]=1003.3188;
metmast_TH[27].pressure[0]=1003.2397;
metmast_TH[28].pressure[0]=1003.3064;
metmast_TH[29].pressure[0]=1003.3972;
metmast_TH[30].pressure[0]=1003.5437;
metmast_TH[31].pressure[0]=1003.5598;
metmast_TH[32].pressure[0]=1003.5547;
metmast_TH[33].pressure[0]=1003.5913;
metmast_TH[34].pressure[0]=1003.4433;
metmast_TH[35].pressure[0]=1003.31;
metmast_TH[36].pressure[0]=1003.2961;
metmast_TH[37].pressure[0]=1003.3796;
metmast_TH[38].pressure[0]=1003.4741;
metmast_TH[39].pressure[0]=1003.4851;
metmast_TH[40].pressure[0]=1003.3225;
metmast_TH[41].pressure[0]=1003.2412;
metmast_TH[42].pressure[0]=1003.3349;
metmast_TH[43].pressure[0]=1003.3606;
metmast_TH[44].pressure[0]=1003.3034;
metmast_TH[45].pressure[0]=1003.2822;
metmast_TH[46].pressure[0]=1003.2705;
metmast_TH[47].pressure[0]=1003.3195;
metmast_TH[48].pressure[0]=1003.3093;
metmast_TH[49].pressure[0]=1003.3576;
metmast_TH[50].pressure[0]=1003.447;
metmast_TH[51].pressure[0]=1003.4177;
metmast_TH[52].pressure[0]=1003.3972;
metmast_TH[53].pressure[0]=1003.3078;
metmast_TH[54].pressure[0]=1003.2529;
metmast_TH[55].pressure[0]=1003.3825;
metmast_TH[56].pressure[0]=1003.5254;
metmast_TH[57].pressure[0]=1003.5407;
metmast_TH[58].pressure[0]=1003.5554;
metmast_TH[59].pressure[0]=1003.5642;
metmast_TH[60].pressure[0]=1003.5993;
metmast_TH[61].pressure[0]=1003.6389;
metmast_TH[62].pressure[0]=1003.5591;
metmast_TH[63].pressure[0]=1003.5202;
metmast_TH[64].pressure[0]=1003.5261;
metmast_TH[65].pressure[0]=1003.5078;
metmast_TH[66].pressure[0]=1003.4961;
metmast_TH[67].pressure[0]=1003.4961;
metmast_TH[68].pressure[0]=1003.57;
metmast_TH[69].pressure[0]=1003.5715;
metmast_TH[70].pressure[0]=1003.603;
metmast_TH[71].pressure[0]=1003.592;
metmast_TH[72].pressure[0]=1003.5517;
metmast_TH[73].pressure[0]=1003.5979;
metmast_TH[74].pressure[0]=1003.6118;
metmast_TH[75].pressure[0]=1003.6206;
metmast_TH[76].pressure[0]=1003.6294;
metmast_TH[77].pressure[0]=1003.6155;
metmast_TH[78].pressure[0]=1003.6147;
metmast_TH[79].pressure[0]=1003.5993;
metmast_TH[80].pressure[0]=1003.5664;
metmast_TH[81].pressure[0]=1003.5876;
metmast_TH[82].pressure[0]=1003.6579;
metmast_TH[83].pressure[0]=1003.6433;
metmast_TH[84].pressure[0]=1003.655;
metmast_TH[85].pressure[0]=1003.6396;
metmast_TH[86].pressure[0]=1003.614;
metmast_TH[87].pressure[0]=1003.6169;
metmast_TH[88].pressure[0]=1003.6111;
metmast_TH[89].pressure[0]=1003.6074;
metmast_TH[90].pressure[0]=1003.6235;
metmast_TH[91].pressure[0]=1003.6037;
metmast_TH[92].pressure[0]=1003.5971;
metmast_TH[93].pressure[0]=1003.5239;
metmast_TH[94].pressure[0]=1003.5202;
metmast_TH[95].pressure[0]=1003.5415;
metmast_TH[96].pressure[0]=1003.5144;
metmast_TH[97].pressure[0]=1003.5649;
metmast_TH[98].pressure[0]=1003.5818;
metmast_TH[99].pressure[0]=1003.5451;
metmast_TH[100].pressure[0]=1003.5744;
metmast_TH[101].pressure[0]=1003.5686;
metmast_TH[102].pressure[0]=1003.5217;
metmast_TH[103].pressure[0]=1003.5437;
metmast_TH[104].pressure[0]=1003.5884;
metmast_TH[105].pressure[0]=1003.5935;
metmast_TH[106].pressure[0]=1003.57;
metmast_TH[107].pressure[0]=1003.6111;
metmast_TH[108].pressure[0]=1003.5869;
metmast_TH[109].pressure[0]=1003.5129;
metmast_TH[110].pressure[0]=1003.592;
metmast_TH[111].pressure[0]=1003.5591;
metmast_TH[112].pressure[0]=1003.6001;
metmast_TH[113].pressure[0]=1003.6748;
metmast_TH[114].pressure[0]=1003.6631;
metmast_TH[115].pressure[0]=1003.6579;
metmast_TH[116].pressure[0]=1003.6572;
metmast_TH[117].pressure[0]=1003.6484;
metmast_TH[118].pressure[0]=1003.6045;
metmast_TH[119].pressure[0]=1003.6528;
metmast_TH[120].pressure[0]=1003.6594;
metmast_TH[121].pressure[0]=1003.6382;
metmast_TH[122].pressure[0]=1003.5949;
metmast_TH[123].pressure[0]=1003.5964;
metmast_TH[124].pressure[0]=1003.5759;
metmast_TH[125].pressure[0]=1003.5569;
metmast_TH[126].pressure[0]=1003.4609;
metmast_TH[127].pressure[0]=1003.4609;
metmast_TH[128].pressure[0]=1003.3686;
metmast_TH[129].pressure[0]=1003.3928;
metmast_TH[130].pressure[0]=1003.5517;
metmast_TH[131].pressure[0]=1003.5869;
metmast_TH[132].pressure[0]=1003.622;
metmast_TH[133].pressure[0]=1003.5459;
metmast_TH[134].pressure[0]=1003.5085;
metmast_TH[135].pressure[0]=1003.5422;
metmast_TH[136].pressure[0]=1003.5847;
metmast_TH[137].pressure[0]=1003.5063;
metmast_TH[138].pressure[0]=1003.4902;
metmast_TH[139].pressure[0]=1003.3986;
metmast_TH[140].pressure[0]=1003.4756;
metmast_TH[141].pressure[0]=1003.5444;
metmast_TH[142].pressure[0]=1003.5188;
metmast_TH[143].pressure[0]=1003.5927;
metmast_TH[144].pressure[0]=1003.6543;
metmast_TH[145].pressure[0]=1003.6448;
metmast_TH[146].pressure[0]=1003.644;
metmast_TH[147].pressure[0]=1003.655;
metmast_TH[148].pressure[0]=1003.5363;
metmast_TH[149].pressure[0]=1003.4521;
metmast_TH[150].pressure[0]=1003.3466;
metmast_TH[151].pressure[0]=1003.5847;
metmast_TH[152].pressure[0]=1003.5737;
metmast_TH[153].pressure[0]=1003.3137;
metmast_TH[154].pressure[0]=1003.3686;
metmast_TH[155].pressure[0]=1003.5664;
metmast_TH[156].pressure[0]=1003.6037;
metmast_TH[157].pressure[0]=1003.603;
metmast_TH[158].pressure[0]=1003.5803;
metmast_TH[159].pressure[0]=1003.4404;
metmast_TH[160].pressure[0]=1003.2653;
metmast_TH[161].pressure[0]=1003.2536;
metmast_TH[162].pressure[0]=1003.2968;
metmast_TH[163].pressure[0]=1003.4631;
metmast_TH[164].pressure[0]=1003.4902;
metmast_TH[165].pressure[0]=1003.4887;
metmast_TH[166].pressure[0]=1003.5012;
metmast_TH[167].pressure[0]=1003.5884;
metmast_TH[168].pressure[0]=1003.5803;
metmast_TH[169].pressure[0]=1003.5949;
metmast_TH[170].pressure[0]=1003.5869;
metmast_TH[171].pressure[0]=1003.5891;
metmast_TH[172].pressure[0]=1003.5627;
metmast_TH[173].pressure[0]=1003.5891;
metmast_TH[174].pressure[0]=1003.5781;
metmast_TH[175].pressure[0]=1003.551;
metmast_TH[176].pressure[0]=1003.5737;
metmast_TH[177].pressure[0]=1003.5634;
metmast_TH[178].pressure[0]=1003.5437;
metmast_TH[179].pressure[0]=1003.5796;
metmast_TH[180].pressure[0]=1003.5027;
metmast_TH[181].pressure[0]=1003.4353;
metmast_TH[182].pressure[0]=1003.4895;
metmast_TH[183].pressure[0]=1003.2976;
metmast_TH[184].pressure[0]=1003.3525;
metmast_TH[185].pressure[0]=1003.2675;
metmast_TH[186].pressure[0]=1003.3488;
metmast_TH[187].pressure[0]=1003.3803;
metmast_TH[188].pressure[0]=1003.5254;
metmast_TH[189].pressure[0]=1003.4917;
metmast_TH[190].pressure[0]=1003.562;
metmast_TH[191].pressure[0]=1003.4858;
metmast_TH[192].pressure[0]=1003.4323;
metmast_TH[193].pressure[0]=1003.5005;
metmast_TH[194].pressure[0]=1003.4419;
metmast_TH[195].pressure[0]=1003.4082;
metmast_TH[196].pressure[0]=1003.4507;
metmast_TH[197].pressure[0]=1003.3972;
metmast_TH[198].pressure[0]=1003.4235;
metmast_TH[199].pressure[0]=1003.4221;
metmast_TH[200].pressure[0]=1003.5136;
metmast_TH[201].pressure[0]=1003.5034;
metmast_TH[202].pressure[0]=1003.384;
metmast_TH[203].pressure[0]=1003.395;
metmast_TH[204].pressure[0]=1003.1906;
metmast_TH[205].pressure[0]=1003.2185;
metmast_TH[206].pressure[0]=1003.1569;
metmast_TH[207].pressure[0]=1003.2683;
metmast_TH[208].pressure[0]=1003.2778;
metmast_TH[209].pressure[0]=1003.2858;
metmast_TH[210].pressure[0]=1003.2815;
metmast_TH[211].pressure[0]=1003.2954;
metmast_TH[212].pressure[0]=1003.3481;
metmast_TH[213].pressure[0]=1003.4038;
metmast_TH[214].pressure[0]=1003.3862;
metmast_TH[215].pressure[0]=1003.3225;
metmast_TH[216].pressure[0]=1003.2543;
metmast_TH[217].pressure[0]=1003.2558;
metmast_TH[218].pressure[0]=1003.3474;
metmast_TH[219].pressure[0]=1003.3913;
metmast_TH[220].pressure[0]=1003.2895;
metmast_TH[221].pressure[0]=1003.3964;
metmast_TH[222].pressure[0]=1003.4228;
metmast_TH[223].pressure[0]=1003.4477;
metmast_TH[224].pressure[0]=1003.4968;
metmast_TH[225].pressure[0]=1003.4587;
metmast_TH[226].pressure[0]=1003.4712;
metmast_TH[227].pressure[0]=1003.4301;
metmast_TH[228].pressure[0]=1003.2668;
metmast_TH[229].pressure[0]=1003.1423;
metmast_TH[230].pressure[0]=1003.3415;
metmast_TH[231].pressure[0]=1003.3532;
metmast_TH[232].pressure[0]=1003.3078;
metmast_TH[233].pressure[0]=1003.3364;
metmast_TH[234].pressure[0]=1003.4463;
metmast_TH[235].pressure[0]=1003.4873;
metmast_TH[236].pressure[0]=1003.4712;
metmast_TH[237].pressure[0]=1003.4983;
metmast_TH[238].pressure[0]=1003.5561;
metmast_TH[239].pressure[0]=1003.5254;
metmast_TH[240].pressure[0]=1003.4507;
metmast_TH[241].pressure[0]=1003.4199;
metmast_TH[242].pressure[0]=1003.4807;
metmast_TH[243].pressure[0]=1003.4924;
metmast_TH[244].pressure[0]=1003.4865;
metmast_TH[245].pressure[0]=1003.5305;
metmast_TH[246].pressure[0]=1003.5473;
metmast_TH[247].pressure[0]=1003.5774;
metmast_TH[248].pressure[0]=1003.5532;
metmast_TH[249].pressure[0]=1003.4902;
metmast_TH[250].pressure[0]=1003.5144;
metmast_TH[251].pressure[0]=1003.5305;
metmast_TH[252].pressure[0]=1003.529;
metmast_TH[253].pressure[0]=1003.54;
metmast_TH[254].pressure[0]=1003.5158;
metmast_TH[255].pressure[0]=1003.532;
metmast_TH[256].pressure[0]=1003.5224;
metmast_TH[257].pressure[0]=1003.4704;
metmast_TH[258].pressure[0]=1003.5151;
metmast_TH[259].pressure[0]=1003.518;
metmast_TH[260].pressure[0]=1003.5005;
metmast_TH[261].pressure[0]=1003.4206;
metmast_TH[262].pressure[0]=1003.3789;
metmast_TH[263].pressure[0]=1003.4338;
metmast_TH[264].pressure[0]=1003.4726;
metmast_TH[265].pressure[0]=1003.4287;
metmast_TH[266].pressure[0]=1003.3503;
metmast_TH[267].pressure[0]=1003.3767;
metmast_TH[268].pressure[0]=1003.3664;
metmast_TH[269].pressure[0]=1003.4609;
metmast_TH[270].pressure[0]=1003.447;
metmast_TH[271].pressure[0]=1003.447;
metmast_TH[272].pressure[0]=1003.4909;
metmast_TH[273].pressure[0]=1003.4675;
metmast_TH[274].pressure[0]=1003.5034;
metmast_TH[275].pressure[0]=1003.5613;
metmast_TH[276].pressure[0]=1003.5986;
metmast_TH[277].pressure[0]=1003.5986;
metmast_TH[278].pressure[0]=1003.6037;
metmast_TH[279].pressure[0]=1003.6279;
metmast_TH[280].pressure[0]=1003.6015;
metmast_TH[281].pressure[0]=1003.6074;
metmast_TH[282].pressure[0]=1003.5986;
metmast_TH[283].pressure[0]=1003.5781;
metmast_TH[284].pressure[0]=1003.6008;
metmast_TH[285].pressure[0]=1003.5986;
metmast_TH[286].pressure[0]=1003.5554;
metmast_TH[287].pressure[0]=1003.4734;
metmast_TH[288].pressure[0]=1003.5385;
metmast_TH[289].pressure[0]=1003.5144;
metmast_TH[290].pressure[0]=1003.6067;
metmast_TH[291].pressure[0]=1003.6096;
metmast_TH[292].pressure[0]=1003.6206;
metmast_TH[293].pressure[0]=1003.6118;
metmast_TH[294].pressure[0]=1003.5979;
metmast_TH[295].pressure[0]=1003.518;
metmast_TH[296].pressure[0]=1003.4704;
metmast_TH[297].pressure[0]=1003.4243;
metmast_TH[298].pressure[0]=1003.3569;
metmast_TH[299].pressure[0]=1003.3862;
metmast_TH[300].pressure[0]=1003.5232;
metmast_TH[301].pressure[0]=1003.5554;
metmast_TH[302].pressure[0]=1003.5173;
metmast_TH[303].pressure[0]=1003.4501;
metmast_TH[304].pressure[0]=1003.5268;
metmast_TH[305].pressure[0]=1003.4375;
metmast_TH[306].pressure[0]=1003.3781;
metmast_TH[307].pressure[0]=1003.5334;
metmast_TH[308].pressure[0]=1003.5349;
metmast_TH[309].pressure[0]=1003.4345;
metmast_TH[310].pressure[0]=1003.3474;
metmast_TH[311].pressure[0]=1003.34;
metmast_TH[312].pressure[0]=1003.3415;
metmast_TH[313].pressure[0]=1003.4111;
metmast_TH[314].pressure[0]=1003.4338;
metmast_TH[315].pressure[0]=1003.4155;
metmast_TH[316].pressure[0]=1003.4939;
metmast_TH[317].pressure[0]=1003.4975;
metmast_TH[318].pressure[0]=1003.4719;
metmast_TH[319].pressure[0]=1003.3415;
metmast_TH[320].pressure[0]=1003.4734;
metmast_TH[321].pressure[0]=1003.3715;
metmast_TH[322].pressure[0]=1003.2822;
metmast_TH[323].pressure[0]=1003.2815;
metmast_TH[324].pressure[0]=1003.2968;
metmast_TH[325].pressure[0]=1003.247;
metmast_TH[326].pressure[0]=1003.2961;
metmast_TH[327].pressure[0]=1003.3488;
metmast_TH[328].pressure[0]=1003.3554;
metmast_TH[329].pressure[0]=1003.1921;
metmast_TH[330].pressure[0]=1003.2353;
metmast_TH[331].pressure[0]=1003.1936;
metmast_TH[332].pressure[0]=1003.302;
metmast_TH[333].pressure[0]=1003.4206;
metmast_TH[334].pressure[0]=1003.4272;
metmast_TH[335].pressure[0]=1003.3415;
metmast_TH[336].pressure[0]=1003.3525;
metmast_TH[337].pressure[0]=1003.3781;
metmast_TH[338].pressure[0]=1003.3327;
metmast_TH[339].pressure[0]=1003.321;
metmast_TH[340].pressure[0]=1003.31;
metmast_TH[341].pressure[0]=1003.3488;
metmast_TH[342].pressure[0]=1003.351;
metmast_TH[343].pressure[0]=1003.4118;
metmast_TH[344].pressure[0]=1003.436;
metmast_TH[345].pressure[0]=1003.4038;
metmast_TH[346].pressure[0]=1003.4726;
metmast_TH[347].pressure[0]=1003.4469;
metmast_TH[348].pressure[0]=1003.5027;
metmast_TH[349].pressure[0]=1003.4887;
metmast_TH[350].pressure[0]=1003.4909;
metmast_TH[351].pressure[0]=1003.4895;
metmast_TH[352].pressure[0]=1003.4968;
metmast_TH[353].pressure[0]=1003.4843;
metmast_TH[354].pressure[0]=1003.3899;
metmast_TH[355].pressure[0]=1003.4814;
metmast_TH[356].pressure[0]=1003.4909;
metmast_TH[357].pressure[0]=1003.4939;
metmast_TH[358].pressure[0]=1003.4887;
metmast_TH[359].pressure[0]=1003.4997;
metmast_TH[360].pressure[0]=1003.5136;
metmast_TH[361].pressure[0]=1003.5246;
metmast_TH[362].pressure[0]=1003.5128;
metmast_TH[363].pressure[0]=1003.5041;
metmast_TH[364].pressure[0]=1003.4895;
metmast_TH[365].pressure[0]=1003.4763;
metmast_TH[366].pressure[0]=1003.4895;
metmast_TH[367].pressure[0]=1003.5056;
metmast_TH[368].pressure[0]=1003.5063;
metmast_TH[369].pressure[0]=1003.5576;
metmast_TH[370].pressure[0]=1003.5407;
metmast_TH[371].pressure[0]=1003.4858;
metmast_TH[372].pressure[0]=1003.4953;
metmast_TH[373].pressure[0]=1003.5796;
metmast_TH[374].pressure[0]=1003.5759;
metmast_TH[375].pressure[0]=1003.5378;
metmast_TH[376].pressure[0]=1003.5385;
metmast_TH[377].pressure[0]=1003.5254;
metmast_TH[378].pressure[0]=1003.5642;
metmast_TH[379].pressure[0]=1003.5898;
metmast_TH[380].pressure[0]=1003.4631;
metmast_TH[381].pressure[0]=1003.4331;
metmast_TH[382].pressure[0]=1003.4206;
metmast_TH[383].pressure[0]=1003.5254;
metmast_TH[384].pressure[0]=1003.447;
metmast_TH[385].pressure[0]=1003.36;
metmast_TH[386].pressure[0]=1003.3269;
metmast_TH[387].pressure[0]=1003.3855;
metmast_TH[388].pressure[0]=1003.3606;
metmast_TH[389].pressure[0]=1003.4492;
metmast_TH[390].pressure[0]=1003.455;
metmast_TH[391].pressure[0]=1003.3679;
metmast_TH[392].pressure[0]=1003.4507;
metmast_TH[393].pressure[0]=1003.5005;
metmast_TH[394].pressure[0]=1003.4821;
metmast_TH[395].pressure[0]=1003.4946;
metmast_TH[396].pressure[0]=1003.4389;
metmast_TH[397].pressure[0]=1003.4038;
metmast_TH[398].pressure[0]=1003.5283;
metmast_TH[399].pressure[0]=1003.5041;
metmast_TH[400].pressure[0]=1003.4433;
metmast_TH[401].pressure[0]=1003.4785;
metmast_TH[402].pressure[0]=1003.5019;
metmast_TH[403].pressure[0]=1003.4924;
metmast_TH[404].pressure[0]=1003.4463;
metmast_TH[405].pressure[0]=1003.4887;
metmast_TH[406].pressure[0]=1003.403;
metmast_TH[407].pressure[0]=1003.3884;
metmast_TH[408].pressure[0]=1003.2932;
metmast_TH[409].pressure[0]=1003.458;
metmast_TH[410].pressure[0]=1003.5363;
metmast_TH[411].pressure[0]=1003.5261;
metmast_TH[412].pressure[0]=1003.5305;
metmast_TH[413].pressure[0]=1003.57;
metmast_TH[414].pressure[0]=1003.573;
metmast_TH[415].pressure[0]=1003.5847;
metmast_TH[416].pressure[0]=1003.5796;
metmast_TH[417].pressure[0]=1003.584;
metmast_TH[418].pressure[0]=1003.5891;
metmast_TH[419].pressure[0]=1003.5664;
metmast_TH[420].pressure[0]=1003.5803;
metmast_TH[421].pressure[0]=1003.5591;
metmast_TH[422].pressure[0]=1003.5342;
metmast_TH[423].pressure[0]=1003.5415;
metmast_TH[424].pressure[0]=1003.5056;
metmast_TH[425].pressure[0]=1003.4785;
metmast_TH[426].pressure[0]=1003.4873;
metmast_TH[427].pressure[0]=1003.477;
metmast_TH[428].pressure[0]=1003.458;
metmast_TH[429].pressure[0]=1003.4536;
metmast_TH[430].pressure[0]=1003.4712;
metmast_TH[431].pressure[0]=1003.4785;
metmast_TH[432].pressure[0]=1003.5012;
metmast_TH[433].pressure[0]=1003.5049;
metmast_TH[434].pressure[0]=1003.5693;
metmast_TH[435].pressure[0]=1003.5715;
metmast_TH[436].pressure[0]=1003.5363;
metmast_TH[437].pressure[0]=1003.5613;
metmast_TH[438].pressure[0]=1003.5627;
metmast_TH[439].pressure[0]=1003.5898;
metmast_TH[440].pressure[0]=1003.5949;
metmast_TH[441].pressure[0]=1003.5986;
metmast_TH[442].pressure[0]=1003.5913;
metmast_TH[443].pressure[0]=1003.5927;
metmast_TH[444].pressure[0]=1003.5349;
metmast_TH[445].pressure[0]=1003.4917;
metmast_TH[446].pressure[0]=1003.3972;
metmast_TH[447].pressure[0]=1003.3444;
metmast_TH[448].pressure[0]=1003.4785;
metmast_TH[449].pressure[0]=1003.4587;
metmast_TH[450].pressure[0]=1003.4448;
metmast_TH[451].pressure[0]=1003.5173;
metmast_TH[452].pressure[0]=1003.469;
metmast_TH[453].pressure[0]=1003.4565;
metmast_TH[454].pressure[0]=1003.5503;
metmast_TH[455].pressure[0]=1003.4807;
metmast_TH[456].pressure[0]=1003.4653;
metmast_TH[457].pressure[0]=1003.499;
metmast_TH[458].pressure[0]=1003.5012;
metmast_TH[459].pressure[0]=1003.4895;
metmast_TH[460].pressure[0]=1003.4983;
metmast_TH[461].pressure[0]=1003.5107;
metmast_TH[462].pressure[0]=1003.5261;
metmast_TH[463].pressure[0]=1003.5188;
metmast_TH[464].pressure[0]=1003.5166;
metmast_TH[465].pressure[0]=1003.5092;
metmast_TH[466].pressure[0]=1003.5144;
metmast_TH[467].pressure[0]=1003.5136;
metmast_TH[468].pressure[0]=1003.532;
metmast_TH[469].pressure[0]=1003.54;
metmast_TH[470].pressure[0]=1003.4807;
metmast_TH[471].pressure[0]=1003.5283;
metmast_TH[472].pressure[0]=1003.5481;
metmast_TH[473].pressure[0]=1003.5385;
metmast_TH[474].pressure[0]=1003.5041;
metmast_TH[475].pressure[0]=1003.5708;
metmast_TH[476].pressure[0]=1003.5305;
metmast_TH[477].pressure[0]=1003.518;
metmast_TH[478].pressure[0]=1003.4931;
metmast_TH[479].pressure[0]=1003.4719;
metmast_TH[480].pressure[0]=1003.3972;
metmast_TH[481].pressure[0]=1003.3525;
metmast_TH[482].pressure[0]=1003.4206;
metmast_TH[483].pressure[0]=1003.4843;
metmast_TH[484].pressure[0]=1003.5188;
metmast_TH[485].pressure[0]=1003.4836;
metmast_TH[486].pressure[0]=1003.4763;
metmast_TH[487].pressure[0]=1003.488;
metmast_TH[488].pressure[0]=1003.4594;
metmast_TH[489].pressure[0]=1003.507;
metmast_TH[490].pressure[0]=1003.4953;
metmast_TH[491].pressure[0]=1003.4829;
metmast_TH[492].pressure[0]=1003.4953;
metmast_TH[493].pressure[0]=1003.4902;
metmast_TH[494].pressure[0]=1003.4756;
metmast_TH[495].pressure[0]=1003.4836;
metmast_TH[496].pressure[0]=1003.4675;
metmast_TH[497].pressure[0]=1003.4924;
metmast_TH[498].pressure[0]=1003.5261;
metmast_TH[499].pressure[0]=1003.5114;
metmast_TH[500].pressure[0]=1003.4917;
metmast_TH[501].pressure[0]=1003.4814;
metmast_TH[502].pressure[0]=1003.4338;
metmast_TH[503].pressure[0]=1003.2976;
metmast_TH[504].pressure[0]=1003.3137;
metmast_TH[505].pressure[0]=1003.2251;
metmast_TH[506].pressure[0]=1003.3027;
metmast_TH[507].pressure[0]=1003.291;
metmast_TH[508].pressure[0]=1003.2353;
metmast_TH[509].pressure[0]=1003.2507;
metmast_TH[510].pressure[0]=1003.3012;
metmast_TH[511].pressure[0]=1003.3928;
metmast_TH[512].pressure[0]=1003.3474;
metmast_TH[513].pressure[0]=1003.4624;
metmast_TH[514].pressure[0]=1003.4843;
metmast_TH[515].pressure[0]=1003.4433;
metmast_TH[516].pressure[0]=1003.488;
metmast_TH[517].pressure[0]=1003.5019;
metmast_TH[518].pressure[0]=1003.4851;
metmast_TH[519].pressure[0]=1003.4323;
metmast_TH[520].pressure[0]=1003.4792;
metmast_TH[521].pressure[0]=1003.5085;
metmast_TH[522].pressure[0]=1003.5378;
metmast_TH[523].pressure[0]=1003.4235;
metmast_TH[524].pressure[0]=1003.4096;
metmast_TH[525].pressure[0]=1003.3913;
metmast_TH[526].pressure[0]=1003.3393;
metmast_TH[527].pressure[0]=1003.2924;
metmast_TH[528].pressure[0]=1003.2727;
metmast_TH[529].pressure[0]=1003.2529;
metmast_TH[530].pressure[0]=1003.2617;
metmast_TH[531].pressure[0]=1003.354;
metmast_TH[532].pressure[0]=1003.3364;
metmast_TH[533].pressure[0]=1003.4148;
metmast_TH[534].pressure[0]=1003.3862;
metmast_TH[535].pressure[0]=1003.4287;
metmast_TH[536].pressure[0]=1003.414;
metmast_TH[537].pressure[0]=1003.436;
metmast_TH[538].pressure[0]=1003.4719;
metmast_TH[539].pressure[0]=1003.5224;
metmast_TH[540].pressure[0]=1003.4792;
metmast_TH[541].pressure[0]=1003.469;
metmast_TH[542].pressure[0]=1003.417;
metmast_TH[543].pressure[0]=1003.4426;
metmast_TH[544].pressure[0]=1003.5049;
metmast_TH[545].pressure[0]=1003.5144;
metmast_TH[546].pressure[0]=1003.4726;
metmast_TH[547].pressure[0]=1003.5195;
metmast_TH[548].pressure[0]=1003.4338;
metmast_TH[549].pressure[0]=1003.3415;
metmast_TH[550].pressure[0]=1003.4067;
metmast_TH[551].pressure[0]=1003.4118;
metmast_TH[552].pressure[0]=1003.4287;
metmast_TH[553].pressure[0]=1003.4851;
metmast_TH[554].pressure[0]=1003.499;
metmast_TH[555].pressure[0]=1003.4602;
metmast_TH[556].pressure[0]=1003.4675;
metmast_TH[557].pressure[0]=1003.5136;
metmast_TH[558].pressure[0]=1003.499;
metmast_TH[559].pressure[0]=1003.4646;
metmast_TH[560].pressure[0]=1003.477;
metmast_TH[561].pressure[0]=1003.4778;
metmast_TH[562].pressure[0]=1003.4726;
metmast_TH[563].pressure[0]=1003.4931;
metmast_TH[564].pressure[0]=1003.4983;
metmast_TH[565].pressure[0]=1003.4997;
metmast_TH[566].pressure[0]=1003.5012;
metmast_TH[567].pressure[0]=1003.499;
metmast_TH[568].pressure[0]=1003.4799;
metmast_TH[569].pressure[0]=1003.5195;
metmast_TH[570].pressure[0]=1003.4214;
metmast_TH[571].pressure[0]=1003.3803;
metmast_TH[572].pressure[0]=1003.3144;
metmast_TH[573].pressure[0]=1003.288;
metmast_TH[574].pressure[0]=1003.3012;
metmast_TH[575].pressure[0]=1003.4111;
metmast_TH[576].pressure[0]=1003.5027;
metmast_TH[577].pressure[0]=1003.4975;
metmast_TH[578].pressure[0]=1003.521;
metmast_TH[579].pressure[0]=1003.5144;
metmast_TH[580].pressure[0]=1003.4799;
metmast_TH[581].pressure[0]=1003.4411;
metmast_TH[582].pressure[0]=1003.4873;
metmast_TH[583].pressure[0]=1003.4536;
metmast_TH[584].pressure[0]=1003.5261;
metmast_TH[585].pressure[0]=1003.4953;
metmast_TH[586].pressure[0]=1003.4953;
metmast_TH[587].pressure[0]=1003.4514;
metmast_TH[588].pressure[0]=1003.5327;
metmast_TH[589].pressure[0]=1003.5195;
metmast_TH[590].pressure[0]=1003.5378;
metmast_TH[591].pressure[0]=1003.4917;
metmast_TH[592].pressure[0]=1003.4521;
metmast_TH[593].pressure[0]=1003.4016;
metmast_TH[594].pressure[0]=1003.3979;
metmast_TH[595].pressure[0]=1003.4983;
metmast_TH[596].pressure[0]=1003.458;
metmast_TH[597].pressure[0]=1003.4016;
metmast_TH[598].pressure[0]=1003.403;
metmast_TH[599].pressure[0]=1003.5122;
metmast_TH[600].pressure[0]=1003.4887;
metmast_TH[601].pressure[0]=1003.3745;
metmast_TH[602].pressure[0]=1003.3972;
metmast_TH[603].pressure[0]=1003.4382;
metmast_TH[604].pressure[0]=1003.5385;
metmast_TH[605].pressure[0]=1003.5034;
metmast_TH[606].pressure[0]=1003.5129;
metmast_TH[607].pressure[0]=1003.51;
metmast_TH[608].pressure[0]=1003.4843;
metmast_TH[609].pressure[0]=1003.5268;
metmast_TH[610].pressure[0]=1003.529;
metmast_TH[611].pressure[0]=1003.4917;
metmast_TH[612].pressure[0]=1003.4895;
metmast_TH[613].pressure[0]=1003.5356;
metmast_TH[614].pressure[0]=1003.4785;
metmast_TH[615].pressure[0]=1003.4829;
metmast_TH[616].pressure[0]=1003.4485;
metmast_TH[617].pressure[0]=1003.4697;
metmast_TH[618].pressure[0]=1003.529;
metmast_TH[619].pressure[0]=1003.5041;
metmast_TH[620].pressure[0]=1003.4924;
metmast_TH[621].pressure[0]=1003.4829;
metmast_TH[622].pressure[0]=1003.4243;
metmast_TH[623].pressure[0]=1003.4287;
metmast_TH[624].pressure[0]=1003.3943;
metmast_TH[625].pressure[0]=1003.4961;
metmast_TH[626].pressure[0]=1003.4975;
metmast_TH[627].pressure[0]=1003.4624;
metmast_TH[628].pressure[0]=1003.4917;
metmast_TH[629].pressure[0]=1003.5283;
metmast_TH[630].pressure[0]=1003.4814;
metmast_TH[631].pressure[0]=1003.4946;
metmast_TH[632].pressure[0]=1003.4228;
metmast_TH[633].pressure[0]=1003.332;
metmast_TH[634].pressure[0]=1003.302;
metmast_TH[635].pressure[0]=1003.3466;
metmast_TH[636].pressure[0]=1003.4719;
metmast_TH[637].pressure[0]=1003.2529;
metmast_TH[638].pressure[0]=1003.4492;
metmast_TH[639].pressure[0]=1003.3781;
metmast_TH[640].pressure[0]=1003.4074;
metmast_TH[641].pressure[0]=1003.4177;
metmast_TH[642].pressure[0]=1003.4367;
metmast_TH[643].pressure[0]=1003.4668;
metmast_TH[644].pressure[0]=1003.384;
metmast_TH[645].pressure[0]=1003.3576;
metmast_TH[646].pressure[0]=1003.2954;
metmast_TH[647].pressure[0]=1003.3532;
metmast_TH[648].pressure[0]=1003.3503;
metmast_TH[649].pressure[0]=1003.3855;
metmast_TH[650].pressure[0]=1003.3913;
metmast_TH[651].pressure[0]=1003.3591;
metmast_TH[652].pressure[0]=1003.4082;
metmast_TH[653].pressure[0]=1003.3598;
metmast_TH[654].pressure[0]=1003.3891;
metmast_TH[655].pressure[0]=1003.4646;
metmast_TH[656].pressure[0]=1003.4741;
metmast_TH[657].pressure[0]=1003.4463;
metmast_TH[658].pressure[0]=1003.3657;
metmast_TH[659].pressure[0]=1003.3437;
metmast_TH[660].pressure[0]=1003.2448;
metmast_TH[661].pressure[0]=1003.2822;
metmast_TH[662].pressure[0]=1003.299;
metmast_TH[663].pressure[0]=1003.3261;
metmast_TH[664].pressure[0]=1003.4433;
metmast_TH[665].pressure[0]=1003.4543;
metmast_TH[666].pressure[0]=1003.4426;
metmast_TH[667].pressure[0]=1003.3899;
metmast_TH[668].pressure[0]=1003.3554;
metmast_TH[669].pressure[0]=1003.3452;
metmast_TH[670].pressure[0]=1003.3964;
metmast_TH[671].pressure[0]=1003.4712;
metmast_TH[672].pressure[0]=1003.458;
metmast_TH[673].pressure[0]=1003.4594;
metmast_TH[674].pressure[0]=1003.3642;
metmast_TH[675].pressure[0]=1003.3386;
metmast_TH[676].pressure[0]=1003.3781;
metmast_TH[677].pressure[0]=1003.3635;
metmast_TH[678].pressure[0]=1003.3195;
metmast_TH[679].pressure[0]=1003.3576;
metmast_TH[680].pressure[0]=1003.3855;
metmast_TH[681].pressure[0]=1003.4001;
metmast_TH[682].pressure[0]=1003.4323;
metmast_TH[683].pressure[0]=1003.4338;
metmast_TH[684].pressure[0]=1003.4602;
metmast_TH[685].pressure[0]=1003.4814;
metmast_TH[686].pressure[0]=1003.51;
metmast_TH[687].pressure[0]=1003.4821;
metmast_TH[688].pressure[0]=1003.3943;
metmast_TH[689].pressure[0]=1003.3283;
metmast_TH[690].pressure[0]=1003.4675;
metmast_TH[691].pressure[0]=1003.4836;
metmast_TH[692].pressure[0]=1003.5114;
metmast_TH[693].pressure[0]=1003.4638;
metmast_TH[694].pressure[0]=1003.4638;
metmast_TH[695].pressure[0]=1003.3899;
metmast_TH[696].pressure[0]=1003.4734;
metmast_TH[697].pressure[0]=1003.4455;
metmast_TH[698].pressure[0]=1003.4602;
metmast_TH[699].pressure[0]=1003.4572;
metmast_TH[700].pressure[0]=1003.4587;
metmast_TH[701].pressure[0]=1003.3884;
metmast_TH[702].pressure[0]=1003.4778;
metmast_TH[703].pressure[0]=1003.458;
metmast_TH[704].pressure[0]=1003.4514;
metmast_TH[705].pressure[0]=1003.4558;
metmast_TH[706].pressure[0]=1003.4726;
metmast_TH[707].pressure[0]=1003.51;
metmast_TH[708].pressure[0]=1003.5034;
metmast_TH[709].pressure[0]=1003.4924;
metmast_TH[710].pressure[0]=1003.3532;
metmast_TH[711].pressure[0]=1003.3979;
metmast_TH[712].pressure[0]=1003.4792;
metmast_TH[713].pressure[0]=1003.5049;
metmast_TH[714].pressure[0]=1003.4961;
metmast_TH[715].pressure[0]=1003.4865;
metmast_TH[716].pressure[0]=1003.3503;
metmast_TH[717].pressure[0]=1003.3335;
metmast_TH[718].pressure[0]=1003.5005;
metmast_TH[719].pressure[0]=1003.5056;
metmast_TH[720].pressure[0]=1003.4294;
metmast_TH[721].pressure[0]=1003.3379;
metmast_TH[722].pressure[0]=1003.2683;
metmast_TH[723].pressure[0]=1003.258;
metmast_TH[724].pressure[0]=1003.291;
metmast_TH[725].pressure[0]=1003.3129;
metmast_TH[726].pressure[0]=1003.4323;
metmast_TH[727].pressure[0]=1003.4301;
metmast_TH[728].pressure[0]=1003.3759;
metmast_TH[729].pressure[0]=1003.4382;
metmast_TH[730].pressure[0]=1003.3745;
metmast_TH[731].pressure[0]=1003.2954;
metmast_TH[732].pressure[0]=1003.2851;
metmast_TH[733].pressure[0]=1003.1635;
metmast_TH[734].pressure[0]=1003.2485;
metmast_TH[735].pressure[0]=1003.2346;
metmast_TH[736].pressure[0]=1003.269;
metmast_TH[737].pressure[0]=1003.2851;
metmast_TH[738].pressure[0]=1003.2844;
metmast_TH[739].pressure[0]=1003.2463;
metmast_TH[740].pressure[0]=1003.2097;
metmast_TH[741].pressure[0]=1003.2272;
metmast_TH[742].pressure[0]=1003.2236;
metmast_TH[743].pressure[0]=1003.2609;
metmast_TH[744].pressure[0]=1003.2895;
metmast_TH[745].pressure[0]=1003.2683;
metmast_TH[746].pressure[0]=1003.2221;
metmast_TH[747].pressure[0]=1003.2478;
metmast_TH[748].pressure[0]=1003.3349;
metmast_TH[749].pressure[0]=1003.3408;
metmast_TH[750].pressure[0]=1003.3393;
metmast_TH[751].pressure[0]=1003.3862;
metmast_TH[752].pressure[0]=1003.3884;
metmast_TH[753].pressure[0]=1003.4331;
metmast_TH[754].pressure[0]=1003.406;
metmast_TH[755].pressure[0]=1003.4646;
metmast_TH[756].pressure[0]=1003.4748;
metmast_TH[757].pressure[0]=1003.4455;
metmast_TH[758].pressure[0]=1003.4279;
metmast_TH[759].pressure[0]=1003.3606;
metmast_TH[760].pressure[0]=1003.2976;
metmast_TH[761].pressure[0]=1003.3803;
metmast_TH[762].pressure[0]=1003.4785;
metmast_TH[763].pressure[0]=1003.5019;
metmast_TH[764].pressure[0]=1003.5049;
metmast_TH[765].pressure[0]=1003.5136;
metmast_TH[766].pressure[0]=1003.4821;
metmast_TH[767].pressure[0]=1003.5019;
metmast_TH[768].pressure[0]=1003.4975;
metmast_TH[769].pressure[0]=1003.4799;
metmast_TH[770].pressure[0]=1003.4594;
metmast_TH[771].pressure[0]=1003.4917;
metmast_TH[772].pressure[0]=1003.4316;
metmast_TH[773].pressure[0]=1003.4558;
metmast_TH[774].pressure[0]=1003.4126;
metmast_TH[775].pressure[0]=1003.3891;
metmast_TH[776].pressure[0]=1003.4235;
metmast_TH[777].pressure[0]=1003.4821;
metmast_TH[778].pressure[0]=1003.4177;
metmast_TH[779].pressure[0]=1003.4602;
metmast_TH[780].pressure[0]=1003.4865;
metmast_TH[781].pressure[0]=1003.4858;
metmast_TH[782].pressure[0]=1003.4909;
metmast_TH[783].pressure[0]=1003.4953;
metmast_TH[784].pressure[0]=1003.4616;
metmast_TH[785].pressure[0]=1003.4631;
metmast_TH[786].pressure[0]=1003.4785;
metmast_TH[787].pressure[0]=1003.4983;
metmast_TH[788].pressure[0]=1003.4843;
metmast_TH[789].pressure[0]=1003.4785;
metmast_TH[790].pressure[0]=1003.4697;
metmast_TH[791].pressure[0]=1003.3298;
metmast_TH[792].pressure[0]=1003.3232;
metmast_TH[793].pressure[0]=1003.3818;
metmast_TH[794].pressure[0]=1003.4873;
metmast_TH[795].pressure[0]=1003.4873;
metmast_TH[796].pressure[0]=1003.469;
metmast_TH[797].pressure[0]=1003.4792;
metmast_TH[798].pressure[0]=1003.3195;
metmast_TH[799].pressure[0]=1003.3657;
metmast_TH[800].pressure[0]=1003.4206;
metmast_TH[801].pressure[0]=1003.3444;
metmast_TH[802].pressure[0]=1003.3408;
metmast_TH[803].pressure[0]=1003.4602;
metmast_TH[804].pressure[0]=1003.4756;
metmast_TH[805].pressure[0]=1003.4924;
metmast_TH[806].pressure[0]=1003.4946;
metmast_TH[807].pressure[0]=1003.4887;
metmast_TH[808].pressure[0]=1003.5027;
metmast_TH[809].pressure[0]=1003.4961;
metmast_TH[810].pressure[0]=1003.4887;
metmast_TH[811].pressure[0]=1003.4997;
metmast_TH[812].pressure[0]=1003.5085;
metmast_TH[813].pressure[0]=1003.5129;
metmast_TH[814].pressure[0]=1003.5056;
metmast_TH[815].pressure[0]=1003.4931;
metmast_TH[816].pressure[0]=1003.4778;
metmast_TH[817].pressure[0]=1003.5151;
metmast_TH[818].pressure[0]=1003.5034;
metmast_TH[819].pressure[0]=1003.5122;
metmast_TH[820].pressure[0]=1003.507;
metmast_TH[821].pressure[0]=1003.4865;
metmast_TH[822].pressure[0]=1003.455;
metmast_TH[823].pressure[0]=1003.3899;
metmast_TH[824].pressure[0]=1003.4528;
metmast_TH[825].pressure[0]=1003.3869;
metmast_TH[826].pressure[0]=1003.4052;
metmast_TH[827].pressure[0]=1003.373;
metmast_TH[828].pressure[0]=1003.2719;
metmast_TH[829].pressure[0]=1003.3064;
metmast_TH[830].pressure[0]=1003.3957;
metmast_TH[831].pressure[0]=1003.3781;
metmast_TH[832].pressure[0]=1003.3964;
metmast_TH[833].pressure[0]=1003.4294;
metmast_TH[834].pressure[0]=1003.4785;
metmast_TH[835].pressure[0]=1003.4338;
metmast_TH[836].pressure[0]=1003.4587;
metmast_TH[837].pressure[0]=1003.362;
metmast_TH[838].pressure[0]=1003.447;
metmast_TH[839].pressure[0]=1003.4843;
metmast_TH[840].pressure[0]=1003.4528;
metmast_TH[841].pressure[0]=1003.4953;
metmast_TH[842].pressure[0]=1003.507;
metmast_TH[843].pressure[0]=1003.4045;
metmast_TH[844].pressure[0]=1003.3474;
metmast_TH[845].pressure[0]=1003.3532;
metmast_TH[846].pressure[0]=1003.3313;
metmast_TH[847].pressure[0]=1003.3247;
metmast_TH[848].pressure[0]=1003.4162;
metmast_TH[849].pressure[0]=1003.4741;
metmast_TH[850].pressure[0]=1003.4265;
metmast_TH[851].pressure[0]=1003.3408;
metmast_TH[852].pressure[0]=1003.3173;
metmast_TH[853].pressure[0]=1003.3115;
metmast_TH[854].pressure[0]=1003.2858;
metmast_TH[855].pressure[0]=1003.373;
metmast_TH[856].pressure[0]=1003.4235;
metmast_TH[857].pressure[0]=1003.4126;
metmast_TH[858].pressure[0]=1003.3437;
metmast_TH[859].pressure[0]=1003.406;
metmast_TH[860].pressure[0]=1003.4228;
metmast_TH[861].pressure[0]=1003.3781;
metmast_TH[862].pressure[0]=1003.406;
metmast_TH[863].pressure[0]=1003.3415;
metmast_TH[864].pressure[0]=1003.4272;
metmast_TH[865].pressure[0]=1003.4316;
metmast_TH[866].pressure[0]=1003.3613;
metmast_TH[867].pressure[0]=1003.365;
metmast_TH[868].pressure[0]=1003.4001;
metmast_TH[869].pressure[0]=1003.3986;
metmast_TH[870].pressure[0]=1003.4638;
metmast_TH[871].pressure[0]=1003.4712;
metmast_TH[872].pressure[0]=1003.436;
metmast_TH[873].pressure[0]=1003.4756;
metmast_TH[874].pressure[0]=1003.4609;
metmast_TH[875].pressure[0]=1003.4873;
metmast_TH[876].pressure[0]=1003.5056;
metmast_TH[877].pressure[0]=1003.5034;
metmast_TH[878].pressure[0]=1003.4257;
metmast_TH[879].pressure[0]=1003.3972;
metmast_TH[880].pressure[0]=1003.2917;
metmast_TH[881].pressure[0]=1003.3342;
metmast_TH[882].pressure[0]=1003.3371;
metmast_TH[883].pressure[0]=1003.3393;
metmast_TH[884].pressure[0]=1003.3261;
metmast_TH[885].pressure[0]=1003.2866;
metmast_TH[886].pressure[0]=1003.3203;
metmast_TH[887].pressure[0]=1003.3298;
metmast_TH[888].pressure[0]=1003.3203;
metmast_TH[889].pressure[0]=1003.321;
metmast_TH[890].pressure[0]=1003.3254;
metmast_TH[891].pressure[0]=1003.3459;
metmast_TH[892].pressure[0]=1003.3701;
metmast_TH[893].pressure[0]=1003.3335;
metmast_TH[894].pressure[0]=1003.3576;
metmast_TH[895].pressure[0]=1003.414;
metmast_TH[896].pressure[0]=1003.4309;
metmast_TH[897].pressure[0]=1003.4074;
metmast_TH[898].pressure[0]=1003.4214;
metmast_TH[899].pressure[0]=1003.4778;
metmast_TH[900].pressure[0]=1003.4843;
metmast_TH[901].pressure[0]=1003.4931;
metmast_TH[902].pressure[0]=1003.5041;
metmast_TH[903].pressure[0]=1003.5129;
metmast_TH[904].pressure[0]=1003.5224;
metmast_TH[905].pressure[0]=1003.5166;
metmast_TH[906].pressure[0]=1003.5473;
metmast_TH[907].pressure[0]=1003.5283;
metmast_TH[908].pressure[0]=1003.5371;
metmast_TH[909].pressure[0]=1003.5356;
metmast_TH[910].pressure[0]=1003.5114;
metmast_TH[911].pressure[0]=1003.5407;
metmast_TH[912].pressure[0]=1003.5224;
metmast_TH[913].pressure[0]=1003.5217;
metmast_TH[914].pressure[0]=1003.5151;
metmast_TH[915].pressure[0]=1003.521;
metmast_TH[916].pressure[0]=1003.5378;
metmast_TH[917].pressure[0]=1003.499;
metmast_TH[918].pressure[0]=1003.4895;
metmast_TH[919].pressure[0]=1003.5034;
metmast_TH[920].pressure[0]=1003.5056;
metmast_TH[921].pressure[0]=1003.5232;
metmast_TH[922].pressure[0]=1003.5114;
metmast_TH[923].pressure[0]=1003.5158;
metmast_TH[924].pressure[0]=1003.507;
metmast_TH[925].pressure[0]=1003.4756;
metmast_TH[926].pressure[0]=1003.5092;
metmast_TH[927].pressure[0]=1003.5202;
metmast_TH[928].pressure[0]=1003.5136;
metmast_TH[929].pressure[0]=1003.5122;
metmast_TH[930].pressure[0]=1003.5041;
metmast_TH[931].pressure[0]=1003.4939;
metmast_TH[932].pressure[0]=1003.4865;
metmast_TH[933].pressure[0]=1003.4939;
metmast_TH[934].pressure[0]=1003.4558;
metmast_TH[935].pressure[0]=1003.3371;
metmast_TH[936].pressure[0]=1003.3466;
metmast_TH[937].pressure[0]=1003.3554;
metmast_TH[938].pressure[0]=1003.3305;
metmast_TH[939].pressure[0]=1003.4389;
metmast_TH[940].pressure[0]=1003.3913;
metmast_TH[941].pressure[0]=1003.3371;
metmast_TH[942].pressure[0]=1003.3628;
metmast_TH[943].pressure[0]=1003.4653;
metmast_TH[944].pressure[0]=1003.5078;
metmast_TH[945].pressure[0]=1003.499;
metmast_TH[946].pressure[0]=1003.4902;
metmast_TH[947].pressure[0]=1003.4726;
metmast_TH[948].pressure[0]=1003.4778;
metmast_TH[949].pressure[0]=1003.4726;
metmast_TH[950].pressure[0]=1003.3986;
metmast_TH[951].pressure[0]=1003.3781;
metmast_TH[952].pressure[0]=1003.4067;
metmast_TH[953].pressure[0]=1003.3686;
metmast_TH[954].pressure[0]=1003.3664;
metmast_TH[955].pressure[0]=1003.4821;
metmast_TH[956].pressure[0]=1003.4851;
metmast_TH[957].pressure[0]=1003.4814;
metmast_TH[958].pressure[0]=1003.5144;
metmast_TH[959].pressure[0]=1003.4975;
metmast_TH[960].pressure[0]=1003.4726;
metmast_TH[961].pressure[0]=1003.4594;
metmast_TH[962].pressure[0]=1003.4697;
metmast_TH[963].pressure[0]=1003.4807;
metmast_TH[964].pressure[0]=1003.4939;
metmast_TH[965].pressure[0]=1003.4909;
metmast_TH[966].pressure[0]=1003.4917;
metmast_TH[967].pressure[0]=1003.477;
metmast_TH[968].pressure[0]=1003.4814;
metmast_TH[969].pressure[0]=1003.4558;
metmast_TH[970].pressure[0]=1003.4851;
metmast_TH[971].pressure[0]=1003.4953;
metmast_TH[972].pressure[0]=1003.499;
metmast_TH[973].pressure[0]=1003.5129;
metmast_TH[974].pressure[0]=1003.5261;
metmast_TH[975].pressure[0]=1003.5107;
metmast_TH[976].pressure[0]=1003.5122;
metmast_TH[977].pressure[0]=1003.4463;
metmast_TH[978].pressure[0]=1003.4983;
metmast_TH[979].pressure[0]=1003.4748;
metmast_TH[980].pressure[0]=1003.488;
metmast_TH[981].pressure[0]=1003.4975;
metmast_TH[982].pressure[0]=1003.4909;
metmast_TH[983].pressure[0]=1003.4675;
metmast_TH[984].pressure[0]=1003.4843;
metmast_TH[985].pressure[0]=1003.4778;
metmast_TH[986].pressure[0]=1003.4624;
metmast_TH[987].pressure[0]=1003.4953;
metmast_TH[988].pressure[0]=1003.4865;
metmast_TH[989].pressure[0]=1003.4448;
metmast_TH[990].pressure[0]=1003.4184;
metmast_TH[991].pressure[0]=1003.3496;
metmast_TH[992].pressure[0]=1003.351;
metmast_TH[993].pressure[0]=1003.4096;
metmast_TH[994].pressure[0]=1003.4367;
metmast_TH[995].pressure[0]=1003.3386;
metmast_TH[996].pressure[0]=1003.2741;
metmast_TH[997].pressure[0]=1003.3811;
metmast_TH[998].pressure[0]=1003.3899;
metmast_TH[999].pressure[0]=1003.3496;


// MATLABCODEGEN: CloseField Initializing Variables Time History



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
// MATLABCODEGEN: OpenField Assign TimeStep

int iC;
strcpy(metmast.Timestamp_UTC,metmast_TH[CycleCount].Timestamp_UTC);
metmast.ws_110m[0]=metmast_TH[CycleCount].ws_110m[0];
metmast.ws_60m[0]=metmast_TH[CycleCount].ws_60m[0];
metmast.wd_110m[0]=metmast_TH[CycleCount].wd_110m[0];
metmast.wd_60m[0]=metmast_TH[CycleCount].wd_60m[0];
metmast.temperature[0]=metmast_TH[CycleCount].temperature[0];
metmast.humidity[0]=metmast_TH[CycleCount].humidity[0];
metmast.pressure[0]=metmast_TH[CycleCount].pressure[0];

// MATLABCODEGEN: CloseField Assign TimeStep



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
SINT32 ifcmm_AppEOI(VOID)
{

    /* do while(0), to be left as soon as there is an error */
    do
    {
        /* TODO: set module info string, maximum length is SMI_DESCLEN_A */
        snprintf(ifcmm_ModuleInfoDesc, sizeof(ifcmm_ModuleInfoDesc), "TODO: set application specific module info string");

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
    ifcmm_AppDeinit();
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
VOID ifcmm_AppDeinit(VOID)
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
*        For general configuration data, ifcmm_CfgParams is being used.
*        Being called by ifcmm_CfgRead.
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
    snprintf(section, sizeof(section), ifcmm_BaseParams.AppName);

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
                         ifcmm_BaseParams.CfgLine, ifcmm_BaseParams.CfgFileName);
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
                        ifcmm_BaseParams.CfgLine, ifcmm_BaseParams.CfgFileName);
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
                TaskList[idx]->Priority = ifcmm_BaseParams.DefaultPriority;
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
                        ifcmm_BaseParams.CfgLine, ifcmm_BaseParams.CfgFileName);
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
                        ifcmm_BaseParams.CfgLine, ifcmm_BaseParams.CfgFileName);
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
            TaskList[idx]->WdogId = sys_WdogCreate(ifcmm_AppName, wdogtime_us);
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
            snprintf(TaskList[idx]->Name, sizeof(TaskList[idx]->Name), "a%s_%d", ifcmm_AppName, idx + 1);

        snprintf(TaskName, sizeof(TaskList[idx]->Name), "%s", TaskList[idx]->Name);

        /* Task options */
        TaskOptions = 0;
        if (TaskList[idx]->UseFPU)
            TaskOptions |= VX_FP_TASK;

        /* Spawn task with properties set in task list */
        TaskList[idx]->TaskId = sys_TaskSpawn(ifcmm_AppName, TaskName,
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
            if (ifcmm_BaseParams.DebugMode & APP_DBG_INFO1)
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
    pTaskData->SyncSessionId = mio_StartSyncSession(ifcmm_AppName);
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
    if ((ifcmm_ModState == RES_S_STOP) || (ifcmm_ModState == RES_S_EOI))
    {
        /* Disable software watchdog if present */
        if (pTaskData->WdogId)
            sys_WdogDisable(pTaskData->WdogId);

        LOG_I(0, "Task_WaitCycle", "Stopping task '%s' due to module stop", pTaskData->Name);

        /*
         * semaphore will be given by SMI server with calls
         * RpcStart or RpcEndOfInit
         */
        semTake(ifcmm_StateSema, WAIT_FOREVER);
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
MLOCAL VOID ifcmm_CfgInit(VOID)
{
    /* Configuration file name (profile name) */
    strncpy(ifcmm_BaseParams.CfgFileName, ifcmm_ProfileName, M_PATHLEN);

    /* Application name */
    strncpy(ifcmm_BaseParams.AppName, ifcmm_AppName, M_MODNAMELEN);

    /* Line number in configuration file (used as start line for searching) */
    ifcmm_BaseParams.CfgLine = ifcmm_CfgLine;

    /* Worker task priority from module base parameters (BaseParms) */
    ifcmm_BaseParams.DefaultPriority = ifcmm_AppPrio;

    /* Debug mode from module base parameters (BaseParms) */
    ifcmm_BaseParams.DebugMode = ifcmm_Debug;

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
SINT32 ifcmm_CfgRead(VOID)
{
    SINT32  ret;

    /* Initialize configuration with values taken at module init */
    ifcmm_CfgInit();

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
SINT32 ifcmm_SviSrvInit(VOID)
{
    SINT32  ret;
    UINT32  NbOfGlobVars = sizeof(SviGlobVarList) / sizeof(SVI_GLOBVAR);
    UINT32  i;
    CHAR    Func[] = "ifcmm_SviSrvInit";

    /* If there are any SVI variables to be exported */
    if (NbOfGlobVars)
    {
        /* Initialize SVI-handler */
        ifcmm_SviHandle = svi_Init(ifcmm_AppName, 0, 0);
        if (!ifcmm_SviHandle)
        {
            LOG_E(0, Func, "Could not initialize SVI server handle!");
            return (ERROR);
        }
    }
    else
    {
        ifcmm_SviHandle = 0;
        return (OK);
    }

    /* Add the global variables from the list SviGlobVarList */
    for (i = 0; i < NbOfGlobVars; i++)
    {
        ret = svi_AddGlobVar(ifcmm_SviHandle, SviGlobVarList[i].VarName,
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
* @brief Frees SVI server resources according to ifcmm_SviSrvInit()
*        Being called at module deinit by the bTask.
*
* @param[in]  N/A
* @param[out] N/A
*
* @retval     N/A
*******************************************************************************/
VOID ifcmm_SviSrvDeinit(VOID)
{
    /* If there was no or no successful SVI init */
    if (!ifcmm_SviHandle)
        return;

    if (svi_DeInit(ifcmm_SviHandle) < 0)
        LOG_E(0, "ifcmm_SviSrvDeinit", "Could not de-initialize SVI server");

    ifcmm_SviHandle = 0;
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
MLOCAL SINT32 SviClt_Example(UINT32 * pTime_us)
{
    SINT32  ret;

    /* Read the microseconds since CPU boot from module RES. */
    ret = svi_GetVal(pSviLib, TimeSviAddr, pTime_us);
    if (ret != SVI_E_OK)
        LOG_W(0, "SviClt_Example", "Could not read value!");

    return (ret);
}

