/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: avg_inflow.h
 *
 * Code generated for Simulink model 'avg_inflow'.
 *
 * Model version                  : 1.5
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Tue Oct 17 09:16:46 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_avg_inflow_h_
#define RTW_HEADER_avg_inflow_h_
#ifndef avg_inflow_COMMON_INCLUDES_
#define avg_inflow_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* avg_inflow_COMMON_INCLUDES_ */

#include "avg_inflow_types.h"
#include "rtGetInf.h"
#include <stddef.h>
#include <string.h>
#include "rt_nonfinite.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block signals for system '<Root>/Moving Average1' */
typedef struct {
  real_T MovingAverage1;               /* '<Root>/Moving Average1' */
} B_MovingAverage1_avg_inflow_T;

/* Block states (default storage) for system '<Root>/Moving Average1' */
typedef struct {
  dsp_simulink_MovingAverage_av_T obj; /* '<Root>/Moving Average1' */
  boolean_T objisempty;                /* '<Root>/Moving Average1' */
} DW_MovingAverage1_avg_inflow_T;

/* Block signals for system '<S3>/Moving Average5' */
typedef struct {
  real_T MovingAverage5;               /* '<S3>/Moving Average5' */
} B_MovingAverage5_avg_inflow_T;

/* Block states (default storage) for system '<S3>/Moving Average5' */
typedef struct {
  dsp_simulink_MovingAverage_av_T obj; /* '<S3>/Moving Average5' */
  boolean_T objisempty;                /* '<S3>/Moving Average5' */
} DW_MovingAverage5_avg_inflow_T;

/* Block signals for system '<S3>/Moving Standard Deviation1' */
typedef struct {
  real_T MovingStandardDeviation1;     /* '<S3>/Moving Standard Deviation1' */
} B_MovingStandardDeviation1_av_T;

/* Block states (default storage) for system '<S3>/Moving Standard Deviation1' */
typedef struct {
  dsp_simulink_MovingStandardDe_T obj; /* '<S3>/Moving Standard Deviation1' */
  boolean_T objisempty;                /* '<S3>/Moving Standard Deviation1' */
} DW_MovingStandardDeviation1_a_T;

/* Block signals (default storage) */
typedef struct {
  B_MovingStandardDeviation1_av_T MovingStandardDeviation1_pn;/* '<S3>/Moving Standard Deviation1' */
  B_MovingAverage5_avg_inflow_T MovingAverage5_pn;/* '<S3>/Moving Average5' */
  B_MovingStandardDeviation1_av_T MovingStandardDeviation1_p;/* '<S3>/Moving Standard Deviation1' */
  B_MovingAverage5_avg_inflow_T MovingAverage5_p;/* '<S3>/Moving Average5' */
  B_MovingStandardDeviation1_av_T MovingStandardDeviation1;/* '<S3>/Moving Standard Deviation1' */
  B_MovingAverage5_avg_inflow_T MovingAverage5;/* '<S3>/Moving Average5' */
  B_MovingAverage1_avg_inflow_T MovingAverage2;/* '<Root>/Moving Average1' */
  B_MovingAverage1_avg_inflow_T MovingAverage1_p;/* '<Root>/Moving Average1' */
} B_avg_inflow_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  dsp_simulink_MovingStandardDe_T obj; /* '<Root>/Moving Standard Deviation' */
  dsp_simulink_MovingAverage_l_T obj_j;/* '<S1>/Moving Average1' */
  real_T Delay_DSTATE;                 /* '<S5>/Delay' */
  real_T Delay_DSTATE_h;               /* '<S3>/Delay' */
  real_T Delay_DSTATE_o;               /* '<S4>/Delay' */
  DW_MovingStandardDeviation1_a_T MovingStandardDeviation1_pn;/* '<S3>/Moving Standard Deviation1' */
  DW_MovingAverage5_avg_inflow_T MovingAverage5_pn;/* '<S3>/Moving Average5' */
  DW_MovingStandardDeviation1_a_T MovingStandardDeviation1_p;/* '<S3>/Moving Standard Deviation1' */
  DW_MovingAverage5_avg_inflow_T MovingAverage5_p;/* '<S3>/Moving Average5' */
  DW_MovingStandardDeviation1_a_T MovingStandardDeviation1;/* '<S3>/Moving Standard Deviation1' */
  DW_MovingAverage5_avg_inflow_T MovingAverage5;/* '<S3>/Moving Average5' */
  DW_MovingAverage1_avg_inflow_T MovingAverage2;/* '<Root>/Moving Average1' */
  DW_MovingAverage1_avg_inflow_T MovingAverage1_p;/* '<Root>/Moving Average1' */
} DW_avg_inflow_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T input_ws_110m;                /* '<Root>/input_ws_110m' */
  real_T input_ws_60m;                 /* '<Root>/input_ws_60m' */
  real_T input_wd_110m;                /* '<Root>/input_wd_110m' */
} ExtU_avg_inflow_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T output_TI;                    /* '<Root>/output_TI' */
  real_T output_ws_110m;               /* '<Root>/output_ws_110m' */
  real_T output_ws_60m;                /* '<Root>/output_ws_60m' */
  real_T output_shearExp;              /* '<Root>/output_shearExp' */
  real_T output_wd_110m;               /* '<Root>/output_wd_110m' */
  real_T output_InflowOK;              /* '<Root>/output_InflowOK' */
} ExtY_avg_inflow_T;

/* Real-time Model Data Structure */
struct tag_RTM_avg_inflow_T {
  const char_T * volatile errorStatus;
};

/* Block signals (default storage) */
extern B_avg_inflow_T avg_inflow_B;

/* Block states (default storage) */
extern DW_avg_inflow_T avg_inflow_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_avg_inflow_T avg_inflow_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_avg_inflow_T avg_inflow_Y;

/* Model entry point functions */
extern void avg_inflow_initialize(void);
extern void avg_inflow_step(void);
extern void avg_inflow_terminate(void);

/* Real-time Model object */
extern RT_MODEL_avg_inflow_T *const avg_inflow_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S6>/Data Type Duplicate' : Unused code path elimination
 * Block '<S6>/Data Type Propagation' : Unused code path elimination
 * Block '<S11>/Data Type Duplicate' : Unused code path elimination
 * Block '<S11>/Data Type Propagation' : Unused code path elimination
 * Block '<S16>/Data Type Duplicate' : Unused code path elimination
 * Block '<S16>/Data Type Propagation' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'avg_inflow'
 * '<S1>'   : 'avg_inflow/CircularMovAvg'
 * '<S2>'   : 'avg_inflow/MATLAB Function'
 * '<S3>'   : 'avg_inflow/Subsystem1'
 * '<S4>'   : 'avg_inflow/Subsystem2'
 * '<S5>'   : 'avg_inflow/Subsystem6'
 * '<S6>'   : 'avg_inflow/Subsystem1/Saturation Dynamic'
 * '<S7>'   : 'avg_inflow/Subsystem1/Subsystem4'
 * '<S8>'   : 'avg_inflow/Subsystem1/Subsystem5'
 * '<S9>'   : 'avg_inflow/Subsystem1/Subsystem4/Compare To Constant'
 * '<S10>'  : 'avg_inflow/Subsystem1/Subsystem5/Compare To Constant'
 * '<S11>'  : 'avg_inflow/Subsystem2/Saturation Dynamic'
 * '<S12>'  : 'avg_inflow/Subsystem2/Subsystem4'
 * '<S13>'  : 'avg_inflow/Subsystem2/Subsystem5'
 * '<S14>'  : 'avg_inflow/Subsystem2/Subsystem4/Compare To Constant'
 * '<S15>'  : 'avg_inflow/Subsystem2/Subsystem5/Compare To Constant'
 * '<S16>'  : 'avg_inflow/Subsystem6/Saturation Dynamic'
 * '<S17>'  : 'avg_inflow/Subsystem6/Subsystem4'
 * '<S18>'  : 'avg_inflow/Subsystem6/Subsystem5'
 * '<S19>'  : 'avg_inflow/Subsystem6/Subsystem4/Compare To Constant'
 * '<S20>'  : 'avg_inflow/Subsystem6/Subsystem5/Compare To Constant'
 */
#endif                                 /* RTW_HEADER_avg_inflow_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
