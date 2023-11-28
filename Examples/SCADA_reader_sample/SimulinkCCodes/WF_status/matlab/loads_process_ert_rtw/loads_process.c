/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: loads_process.c
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

#include "loads_process.h"
#include <math.h>
#include "rtwtypes.h"
#include <string.h>

/* External inputs (root inport signals with default storage) */
ExtU_loads_process_T loads_process_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_loads_process_T loads_process_Y;

/* Real-time model */
static RT_MODEL_loads_process_T loads_process_M_;
RT_MODEL_loads_process_T *const loads_process_M = &loads_process_M_;

/* Model step function */
void loads_process_step(void)
{
  real_T rtb_TrigonometricFunction;
  real_T rtb_TrigonometricFunction1;

  /* Gain: '<Root>/Gain' incorporates:
   *  Inport: '<Root>/b1_pitch'
   */
  rtb_TrigonometricFunction1 = 0.017453292519943295 * loads_process_U.b1_pitch;

  /* Trigonometry: '<S1>/Trigonometric Function1' */
  rtb_TrigonometricFunction = cos(rtb_TrigonometricFunction1);

  /* Trigonometry: '<S1>/Trigonometric Function' */
  rtb_TrigonometricFunction1 = sin(rtb_TrigonometricFunction1);

  /* Outport: '<Root>/b1_oop' incorporates:
   *  Inport: '<Root>/b1_edge'
   *  Inport: '<Root>/b1_flap'
   *  Product: '<S1>/Product'
   *  Product: '<S1>/Product1'
   *  Sum: '<S1>/Sum'
   */
  loads_process_Y.b1_oop = loads_process_U.b1_flap * rtb_TrigonometricFunction +
    loads_process_U.b1_edge * rtb_TrigonometricFunction1;

  /* Outport: '<Root>/b1_ip' incorporates:
   *  Gain: '<S1>/Gain3'
   *  Inport: '<Root>/b1_edge'
   *  Inport: '<Root>/b1_flap'
   *  Product: '<S1>/Product2'
   *  Product: '<S1>/Product3'
   *  Sum: '<S1>/Sum1'
   */
  loads_process_Y.b1_ip = -rtb_TrigonometricFunction1 * loads_process_U.b1_flap
    + loads_process_U.b1_edge * rtb_TrigonometricFunction;

  /* Gain: '<Root>/Gain1' incorporates:
   *  Inport: '<Root>/b2_pitch'
   */
  rtb_TrigonometricFunction = 0.017453292519943295 * loads_process_U.b2_pitch;

  /* Trigonometry: '<S2>/Trigonometric Function1' */
  rtb_TrigonometricFunction1 = cos(rtb_TrigonometricFunction);

  /* Trigonometry: '<S2>/Trigonometric Function' */
  rtb_TrigonometricFunction = sin(rtb_TrigonometricFunction);

  /* Outport: '<Root>/b2_oop' incorporates:
   *  Inport: '<Root>/b2_edge'
   *  Inport: '<Root>/b2_flap'
   *  Product: '<S2>/Product'
   *  Product: '<S2>/Product1'
   *  Sum: '<S2>/Sum'
   */
  loads_process_Y.b2_oop = loads_process_U.b2_flap * rtb_TrigonometricFunction1
    + loads_process_U.b2_edge * rtb_TrigonometricFunction;

  /* Outport: '<Root>/b2_ip' incorporates:
   *  Gain: '<S2>/Gain3'
   *  Inport: '<Root>/b2_edge'
   *  Inport: '<Root>/b2_flap'
   *  Product: '<S2>/Product2'
   *  Product: '<S2>/Product3'
   *  Sum: '<S2>/Sum1'
   */
  loads_process_Y.b2_ip = -rtb_TrigonometricFunction * loads_process_U.b2_flap +
    loads_process_U.b2_edge * rtb_TrigonometricFunction1;

  /* Gain: '<Root>/Gain2' incorporates:
   *  Inport: '<Root>/b3_pitch'
   */
  rtb_TrigonometricFunction = 0.017453292519943295 * loads_process_U.b3_pitch;

  /* Trigonometry: '<S3>/Trigonometric Function1' */
  rtb_TrigonometricFunction1 = cos(rtb_TrigonometricFunction);

  /* Trigonometry: '<S3>/Trigonometric Function' */
  rtb_TrigonometricFunction = sin(rtb_TrigonometricFunction);

  /* Outport: '<Root>/b3_oop' incorporates:
   *  Inport: '<Root>/b3_edge'
   *  Inport: '<Root>/b3_flap'
   *  Product: '<S3>/Product'
   *  Product: '<S3>/Product1'
   *  Sum: '<S3>/Sum'
   */
  loads_process_Y.b3_oop = loads_process_U.b3_flap * rtb_TrigonometricFunction1
    + loads_process_U.b3_edge * rtb_TrigonometricFunction;

  /* Outport: '<Root>/b3_ip' incorporates:
   *  Gain: '<S3>/Gain3'
   *  Inport: '<Root>/b3_edge'
   *  Inport: '<Root>/b3_flap'
   *  Product: '<S3>/Product2'
   *  Product: '<S3>/Product3'
   *  Sum: '<S3>/Sum1'
   */
  loads_process_Y.b3_ip = -rtb_TrigonometricFunction * loads_process_U.b3_flap +
    loads_process_U.b3_edge * rtb_TrigonometricFunction1;
}

/* Model initialize function */
void loads_process_initialize(void)
{
  /* Registration code */

  /* initialize error status */
  rtmSetErrorStatus(loads_process_M, (NULL));

  /* external inputs */
  (void)memset(&loads_process_U, 0, sizeof(ExtU_loads_process_T));

  /* external outputs */
  (void)memset(&loads_process_Y, 0, sizeof(ExtY_loads_process_T));
}

/* Model terminate function */
void loads_process_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
