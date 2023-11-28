/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: loads_process.h
 *
 * Code generated for Simulink model 'loads_process'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Tue Oct 17 10:32:05 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_loads_process_h_
#define RTW_HEADER_loads_process_h_
#ifndef loads_process_COMMON_INCLUDES_
#define loads_process_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* loads_process_COMMON_INCLUDES_ */

#include "loads_process_types.h"
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
  real_T b1_flap;                      /* '<Root>/b1_flap' */
  real_T b1_edge;                      /* '<Root>/b1_edge' */
  real_T b1_pitch;                     /* '<Root>/b1_pitch' */
  real_T b2_flap;                      /* '<Root>/b2_flap' */
  real_T b2_edge;                      /* '<Root>/b2_edge' */
  real_T b2_pitch;                     /* '<Root>/b2_pitch' */
  real_T b3_flap;                      /* '<Root>/b3_flap' */
  real_T b3_edge;                      /* '<Root>/b3_edge' */
  real_T b3_pitch;                     /* '<Root>/b3_pitch' */
} ExtU_loads_process_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T b1_oop;                       /* '<Root>/b1_oop' */
  real_T b1_ip;                        /* '<Root>/b1_ip' */
  real_T b2_oop;                       /* '<Root>/b2_oop' */
  real_T b2_ip;                        /* '<Root>/b2_ip' */
  real_T b3_oop;                       /* '<Root>/b3_oop' */
  real_T b3_ip;                        /* '<Root>/b3_ip' */
} ExtY_loads_process_T;

/* Real-time Model Data Structure */
struct tag_RTM_loads_process_T {
  const char_T * volatile errorStatus;
};

/* External inputs (root inport signals with default storage) */
extern ExtU_loads_process_T loads_process_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_loads_process_T loads_process_Y;

/* Model entry point functions */
extern void loads_process_initialize(void);
extern void loads_process_step(void);
extern void loads_process_terminate(void);

/* Real-time Model object */
extern RT_MODEL_loads_process_T *const loads_process_M;

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
 * '<Root>' : 'loads_process'
 * '<S1>'   : 'loads_process/Subsystem'
 * '<S2>'   : 'loads_process/Subsystem1'
 * '<S3>'   : 'loads_process/Subsystem2'
 */
#endif                                 /* RTW_HEADER_loads_process_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
