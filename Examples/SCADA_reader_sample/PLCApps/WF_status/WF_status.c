/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: WF_status.c
 *
 * Code generated for Simulink model 'WF_status'.
 *
 * Model version                  : 1.4
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Tue Oct 17 12:36:26 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "WF_status.h"
#include <string.h>

/* External inputs (root inport signals with default storage) */
ExtU_WF_status_T WF_status_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_WF_status_T WF_status_Y;

/* Real-time model */
static RT_MODEL_WF_status_T WF_status_M_;
RT_MODEL_WF_status_T *const WF_status_M = &WF_status_M_;

/* Model step function */
void WF_status_step(void)
{
  /* Logic: '<Root>/Logical Operator2' incorporates:
   *  Constant: '<S1>/Constant'
   *  Constant: '<S2>/Constant'
   *  Constant: '<S3>/Constant'
   *  Inport: '<Root>/avg_wd'
   *  Inport: '<Root>/avg_ws'
   *  Logic: '<Root>/Logical Operator1'
   *  RelationalOperator: '<S1>/Compare'
   *  RelationalOperator: '<S2>/Compare'
   *  RelationalOperator: '<S3>/Compare'
   */
  WF_status_Y.check_WindInSector = ((WF_status_U.avg_ws <= 10.0) &&
    ((WF_status_U.avg_wd <= 260.0) && (WF_status_U.avg_wd >= 180.0)));

  /* Logic: '<Root>/Logical Operator' incorporates:
   *  Constant: '<S4>/Constant'
   *  Constant: '<S5>/Constant'
   *  Constant: '<S6>/Constant'
   *  Inport: '<Root>/wt1_status'
   *  Inport: '<Root>/wt2_status'
   *  Inport: '<Root>/wt3_status'
   *  RelationalOperator: '<S4>/Compare'
   *  RelationalOperator: '<S5>/Compare'
   *  RelationalOperator: '<S6>/Compare'
   */
  WF_status_Y.check_isWFon = ((WF_status_U.wt1_status == 1.0) &&
    (WF_status_U.wt2_status == 1.0) && (WF_status_U.wt3_status == 1.0));

  /* Outport: '<Root>/check_all' incorporates:
   *  Logic: '<Root>/Logical Operator3'
   */
  WF_status_Y.check_all = (WF_status_Y.check_isWFon &&
    WF_status_Y.check_WindInSector);
}

/* Model initialize function */
void WF_status_initialize(void)
{
  /* Registration code */

  /* initialize error status */
  rtmSetErrorStatus(WF_status_M, (NULL));

  /* external inputs */
  (void)memset(&WF_status_U, 0, sizeof(ExtU_WF_status_T));

  /* external outputs */
  (void)memset(&WF_status_Y, 0, sizeof(ExtY_WF_status_T));
}

/* Model terminate function */
void WF_status_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
