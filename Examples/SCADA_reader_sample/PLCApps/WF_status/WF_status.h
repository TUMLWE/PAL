/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: WF_status.h
 *
 * Code generated for Simulink model 'WF_status'.
 *
 * Model version                  : 1.4
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Wed Nov 29 01:10:14 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_WF_status_h_
#define RTW_HEADER_WF_status_h_
#ifndef WF_status_COMMON_INCLUDES_
#define WF_status_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* WF_status_COMMON_INCLUDES_ */

#include "WF_status_types.h"
#include <stddef.h>

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T wt1_status;                   /* '<Root>/wt1_status' */
  real_T wt2_status;                   /* '<Root>/wt2_status' */
  real_T wt3_status;                   /* '<Root>/wt3_status' */
  real_T avg_ws;                       /* '<Root>/avg_ws' */
  real_T avg_wd;                       /* '<Root>/avg_wd' */
} ExtU_WF_status_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  boolean_T check_isWFon;              /* '<Root>/check_isWFon' */
  boolean_T check_WindInSector;        /* '<Root>/check_WindInSector' */
  boolean_T check_all;                 /* '<Root>/check_all' */
} ExtY_WF_status_T;

/* Real-time Model Data Structure */
struct tag_RTM_WF_status_T {
  const char_T * volatile errorStatus;
};

/* External inputs (root inport signals with default storage) */
extern ExtU_WF_status_T WF_status_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_WF_status_T WF_status_Y;

/* Model entry point functions */
extern void WF_status_initialize(void);
extern void WF_status_step(void);
extern void WF_status_terminate(void);

/* Real-time Model object */
extern RT_MODEL_WF_status_T *const WF_status_M;

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
 * '<Root>' : 'WF_status'
 * '<S1>'   : 'WF_status/Compare To Constant'
 * '<S2>'   : 'WF_status/Compare To Constant1'
 * '<S3>'   : 'WF_status/Compare To Constant2'
 * '<S4>'   : 'WF_status/Compare To Constant3'
 * '<S5>'   : 'WF_status/Compare To Constant4'
 * '<S6>'   : 'WF_status/Compare To Constant5'
 */
#endif                                 /* RTW_HEADER_WF_status_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
