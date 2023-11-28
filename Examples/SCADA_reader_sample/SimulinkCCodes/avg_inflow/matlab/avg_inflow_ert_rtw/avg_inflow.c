/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: avg_inflow.c
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

#include "avg_inflow.h"
#include "rtwtypes.h"
#include "avg_inflow_types.h"
#include "avg_inflow_private.h"
#include <string.h>
#include <math.h>
#include "rt_nonfinite.h"
#include <float.h>
#include "rt_defines.h"

/* Block signals (default storage) */
B_avg_inflow_T avg_inflow_B;

/* Block states (default storage) */
DW_avg_inflow_T avg_inflow_DW;

/* External inputs (root inport signals with default storage) */
ExtU_avg_inflow_T avg_inflow_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_avg_inflow_T avg_inflow_Y;

/* Real-time model */
static RT_MODEL_avg_inflow_T avg_inflow_M_;
RT_MODEL_avg_inflow_T *const avg_inflow_M = &avg_inflow_M_;

/* Forward declaration for local functions */
static void avg_inflow_SystemCore_setup(dsp_simulink_MovingAverage_av_T *obj);

/* Forward declaration for local functions */
static void avg_inflow_SystemCore_setup_n(dsp_simulink_MovingAverage_av_T *obj);

/* Forward declaration for local functions */
static void avg_inflow_SystemCore_setup_n2(dsp_simulink_MovingStandardDe_T *obj);

/* Forward declaration for local functions */
static void avg_inflow_SystemCore_setup_lx(dsp_simulink_MovingStandardDe_T *obj);
static void avg_inflow_SystemCore_setup_l(dsp_simulink_MovingAverage_l_T *obj);
static void avg_inflow_SystemCore_setup(dsp_simulink_MovingAverage_av_T *obj)
{
  obj->isSetupComplete = false;
  obj->isInitialized = 1;
  obj->NumChannels = 1;
  obj->FrameLength = 1;
  obj->_pobj0.isInitialized = 0;
  obj->_pobj0.isInitialized = 0;
  obj->pStatistic = &obj->_pobj0;
  obj->isSetupComplete = true;
  obj->TunablePropsChanged = false;
}

/* System initialize for atomic system: */
void avg_inflow_MovingAverage1_Init(DW_MovingAverage1_avg_inflow_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
  int32_T i;

  /* Start for MATLABSystem: '<Root>/Moving Average1' */
  localDW->obj.isInitialized = 0;
  localDW->obj.NumChannels = -1;
  localDW->obj.FrameLength = -1;
  localDW->obj.matlabCodegenIsDeleted = false;
  localDW->objisempty = true;
  avg_inflow_SystemCore_setup(&localDW->obj);

  /* InitializeConditions for MATLABSystem: '<Root>/Moving Average1' */
  obj = localDW->obj.pStatistic;
  if (obj->isInitialized == 1) {
    obj->pCumSum = 0.0;
    for (i = 0; i < 59; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  /* End of InitializeConditions for MATLABSystem: '<Root>/Moving Average1' */
}

/* Output and update for atomic system: */
void avg_inflow_MovingAverage1(real_T rtu_0, B_MovingAverage1_avg_inflow_T
  *localB, DW_MovingAverage1_avg_inflow_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
  real_T csumrev[59];
  real_T csum;
  real_T cumRevIndex;
  real_T modValueRev;
  real_T z;
  int32_T i;

  /* MATLABSystem: '<Root>/Moving Average1' */
  if (localDW->obj.TunablePropsChanged) {
    localDW->obj.TunablePropsChanged = false;
  }

  obj = localDW->obj.pStatistic;
  if (obj->isInitialized != 1) {
    obj->isSetupComplete = false;
    obj->isInitialized = 1;
    obj->pCumSum = 0.0;
    for (i = 0; i < 59; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    obj->pCumSum = 0.0;
    for (i = 0; i < 59; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  cumRevIndex = obj->pCumRevIndex;
  csum = obj->pCumSum;
  for (i = 0; i < 59; i++) {
    csumrev[i] = obj->pCumSumRev[i];
  }

  modValueRev = obj->pModValueRev;
  z = 0.0;

  /* MATLABSystem: '<Root>/Moving Average1' */
  localB->MovingAverage1 = 0.0;

  /* MATLABSystem: '<Root>/Moving Average1' */
  csum += rtu_0;
  if (modValueRev == 0.0) {
    z = csumrev[(int32_T)cumRevIndex - 1] + csum;
  }

  csumrev[(int32_T)cumRevIndex - 1] = rtu_0;
  if (cumRevIndex != 59.0) {
    cumRevIndex++;
  } else {
    cumRevIndex = 1.0;
    csum = 0.0;
    for (i = 57; i >= 0; i--) {
      csumrev[i] += csumrev[i + 1];
    }
  }

  if (modValueRev == 0.0) {
    /* MATLABSystem: '<Root>/Moving Average1' */
    localB->MovingAverage1 = z / 60.0;
  }

  obj->pCumSum = csum;
  for (i = 0; i < 59; i++) {
    obj->pCumSumRev[i] = csumrev[i];
  }

  obj->pCumRevIndex = cumRevIndex;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }
}

/* Termination for atomic system: */
void avg_inflow_MovingAverage1_Term(DW_MovingAverage1_avg_inflow_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;

  /* Terminate for MATLABSystem: '<Root>/Moving Average1' */
  if (!localDW->obj.matlabCodegenIsDeleted) {
    localDW->obj.matlabCodegenIsDeleted = true;
    if ((localDW->obj.isInitialized == 1) && localDW->obj.isSetupComplete) {
      obj = localDW->obj.pStatistic;
      if (obj->isInitialized == 1) {
        obj->isInitialized = 2;
      }

      localDW->obj.NumChannels = -1;
      localDW->obj.FrameLength = -1;
    }
  }

  /* End of Terminate for MATLABSystem: '<Root>/Moving Average1' */
}

static void avg_inflow_SystemCore_setup_n(dsp_simulink_MovingAverage_av_T *obj)
{
  obj->isSetupComplete = false;
  obj->isInitialized = 1;
  obj->NumChannels = 1;
  obj->FrameLength = 1;
  obj->_pobj0.isInitialized = 0;
  obj->_pobj0.isInitialized = 0;
  obj->pStatistic = &obj->_pobj0;
  obj->isSetupComplete = true;
  obj->TunablePropsChanged = false;
}

/* System initialize for atomic system: */
void avg_inflow_MovingAverage5_Init(DW_MovingAverage5_avg_inflow_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
  int32_T i;

  /* Start for MATLABSystem: '<S3>/Moving Average5' */
  localDW->obj.isInitialized = 0;
  localDW->obj.NumChannels = -1;
  localDW->obj.FrameLength = -1;
  localDW->obj.matlabCodegenIsDeleted = false;
  localDW->objisempty = true;
  avg_inflow_SystemCore_setup_n(&localDW->obj);

  /* InitializeConditions for MATLABSystem: '<S3>/Moving Average5' */
  obj = localDW->obj.pStatistic;
  if (obj->isInitialized == 1) {
    obj->pCumSum = 0.0;
    for (i = 0; i < 59; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  /* End of InitializeConditions for MATLABSystem: '<S3>/Moving Average5' */
}

/* Output and update for atomic system: */
void avg_inflow_MovingAverage5(real_T rtu_0, B_MovingAverage5_avg_inflow_T
  *localB, DW_MovingAverage5_avg_inflow_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
  real_T csumrev[59];
  real_T csum;
  real_T cumRevIndex;
  real_T modValueRev;
  real_T z;
  int32_T i;

  /* MATLABSystem: '<S3>/Moving Average5' */
  if (localDW->obj.TunablePropsChanged) {
    localDW->obj.TunablePropsChanged = false;
  }

  obj = localDW->obj.pStatistic;
  if (obj->isInitialized != 1) {
    obj->isSetupComplete = false;
    obj->isInitialized = 1;
    obj->pCumSum = 0.0;
    for (i = 0; i < 59; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    obj->pCumSum = 0.0;
    for (i = 0; i < 59; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  cumRevIndex = obj->pCumRevIndex;
  csum = obj->pCumSum;
  for (i = 0; i < 59; i++) {
    csumrev[i] = obj->pCumSumRev[i];
  }

  modValueRev = obj->pModValueRev;
  z = 0.0;

  /* MATLABSystem: '<S3>/Moving Average5' */
  localB->MovingAverage5 = 0.0;

  /* MATLABSystem: '<S3>/Moving Average5' */
  csum += rtu_0;
  if (modValueRev == 0.0) {
    z = csumrev[(int32_T)cumRevIndex - 1] + csum;
  }

  csumrev[(int32_T)cumRevIndex - 1] = rtu_0;
  if (cumRevIndex != 59.0) {
    cumRevIndex++;
  } else {
    cumRevIndex = 1.0;
    csum = 0.0;
    for (i = 57; i >= 0; i--) {
      csumrev[i] += csumrev[i + 1];
    }
  }

  if (modValueRev == 0.0) {
    /* MATLABSystem: '<S3>/Moving Average5' */
    localB->MovingAverage5 = z / 60.0;
  }

  obj->pCumSum = csum;
  for (i = 0; i < 59; i++) {
    obj->pCumSumRev[i] = csumrev[i];
  }

  obj->pCumRevIndex = cumRevIndex;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }
}

/* Termination for atomic system: */
void avg_inflow_MovingAverage5_Term(DW_MovingAverage5_avg_inflow_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;

  /* Terminate for MATLABSystem: '<S3>/Moving Average5' */
  if (!localDW->obj.matlabCodegenIsDeleted) {
    localDW->obj.matlabCodegenIsDeleted = true;
    if ((localDW->obj.isInitialized == 1) && localDW->obj.isSetupComplete) {
      obj = localDW->obj.pStatistic;
      if (obj->isInitialized == 1) {
        obj->isInitialized = 2;
      }

      localDW->obj.NumChannels = -1;
      localDW->obj.FrameLength = -1;
    }
  }

  /* End of Terminate for MATLABSystem: '<S3>/Moving Average5' */
}

static void avg_inflow_SystemCore_setup_n2(dsp_simulink_MovingStandardDe_T *obj)
{
  obj->isSetupComplete = false;
  obj->isInitialized = 1;
  obj->NumChannels = 1;
  obj->FrameLength = 1;
  obj->_pobj0.isInitialized = 0;
  obj->_pobj0.isInitialized = 0;
  obj->pStatistic = &obj->_pobj0;
  obj->isSetupComplete = true;
  obj->TunablePropsChanged = false;
}

/* System initialize for atomic system: */
void a_MovingStandardDeviation1_Init(DW_MovingStandardDeviation1_a_T *localDW)
{
  g_dsp_internal_SlidingWindowV_T *obj;
  int32_T i;

  /* Start for MATLABSystem: '<S3>/Moving Standard Deviation1' */
  localDW->obj.isInitialized = 0;
  localDW->obj.NumChannels = -1;
  localDW->obj.FrameLength = -1;
  localDW->obj.matlabCodegenIsDeleted = false;
  localDW->objisempty = true;
  avg_inflow_SystemCore_setup_n2(&localDW->obj);

  /* InitializeConditions for MATLABSystem: '<S3>/Moving Standard Deviation1' */
  obj = localDW->obj.pStatistic;
  if (obj->isInitialized == 1) {
    for (i = 0; i < 60; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 1.0;
    obj->pModValueRev = 0.0;
  }

  /* End of InitializeConditions for MATLABSystem: '<S3>/Moving Standard Deviation1' */
}

/* Output and update for atomic system: */
void avg_in_MovingStandardDeviation1(real_T rtu_0,
  B_MovingStandardDeviation1_av_T *localB, DW_MovingStandardDeviation1_a_T
  *localDW)
{
  g_dsp_internal_SlidingWindowV_T *obj;
  real_T reverseS[60];
  real_T reverseSamples[60];
  real_T x[60];
  real_T M;
  real_T Mprev;
  real_T S;
  real_T T;
  real_T counter;
  real_T modValueRev;
  real_T y;
  int32_T i;

  /* MATLABSystem: '<S3>/Moving Standard Deviation1' */
  if (localDW->obj.TunablePropsChanged) {
    localDW->obj.TunablePropsChanged = false;
  }

  obj = localDW->obj.pStatistic;
  if (obj->isInitialized != 1) {
    obj->isSetupComplete = false;
    obj->isInitialized = 1;
    for (i = 0; i < 60; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 0.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    for (i = 0; i < 60; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 1.0;
    obj->pModValueRev = 0.0;
  }

  for (i = 0; i < 60; i++) {
    reverseSamples[i] = obj->pReverseSamples[i];
  }

  for (i = 0; i < 60; i++) {
    x[i] = obj->pReverseT[i];
  }

  for (i = 0; i < 60; i++) {
    reverseS[i] = obj->pReverseS[i];
  }

  T = obj->pT;
  S = obj->pS;
  M = obj->pM;
  counter = obj->pCounter;
  modValueRev = obj->pModValueRev;
  y = 0.0;
  T += rtu_0;
  Mprev = M;
  M += 1.0 / counter * (rtu_0 - M);
  Mprev = rtu_0 - Mprev;
  S += (counter - 1.0) * Mprev * Mprev / counter;
  if (modValueRev == 0.0) {
    y = (60.0 - counter) / counter * T - x[(int32_T)(60.0 - counter) - 1];
    y = (counter / (((60.0 - counter) + counter) * (60.0 - counter)) * (y * y) +
         (reverseS[(int32_T)(60.0 - counter) - 1] + S)) / 59.0;
  }

  reverseSamples[(int32_T)(60.0 - counter) - 1] = rtu_0;
  if (counter < 59.0) {
    counter++;
  } else {
    counter = 1.0;
    memcpy(&x[0], &reverseSamples[0], 60U * sizeof(real_T));
    T = 0.0;
    S = 0.0;
    for (i = 0; i < 59; i++) {
      M = reverseSamples[i];
      x[i + 1] += x[i];
      Mprev = T;
      T += 1.0 / ((real_T)i + 1.0) * (M - T);
      M -= Mprev;
      S += (((real_T)i + 1.0) - 1.0) * M * M / ((real_T)i + 1.0);
      reverseS[i] = S;
    }

    T = 0.0;
    S = 0.0;
    M = 0.0;
  }

  for (i = 0; i < 60; i++) {
    obj->pReverseSamples[i] = reverseSamples[i];
  }

  for (i = 0; i < 60; i++) {
    obj->pReverseT[i] = x[i];
  }

  for (i = 0; i < 60; i++) {
    obj->pReverseS[i] = reverseS[i];
  }

  obj->pT = T;
  obj->pS = S;
  obj->pM = M;
  obj->pCounter = counter;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }

  /* MATLABSystem: '<S3>/Moving Standard Deviation1' */
  localB->MovingStandardDeviation1 = sqrt(y);
}

/* Termination for atomic system: */
void a_MovingStandardDeviation1_Term(DW_MovingStandardDeviation1_a_T *localDW)
{
  g_dsp_internal_SlidingWindowV_T *obj;

  /* Terminate for MATLABSystem: '<S3>/Moving Standard Deviation1' */
  if (!localDW->obj.matlabCodegenIsDeleted) {
    localDW->obj.matlabCodegenIsDeleted = true;
    if ((localDW->obj.isInitialized == 1) && localDW->obj.isSetupComplete) {
      obj = localDW->obj.pStatistic;
      if (obj->isInitialized == 1) {
        obj->isInitialized = 2;
      }

      localDW->obj.NumChannels = -1;
      localDW->obj.FrameLength = -1;
    }
  }

  /* End of Terminate for MATLABSystem: '<S3>/Moving Standard Deviation1' */
}

real_T rt_atan2d_snf(real_T u0, real_T u1)
{
  real_T y;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = (rtNaN);
  } else if (rtIsInf(u0) && rtIsInf(u1)) {
    int32_T tmp;
    int32_T tmp_0;
    if (u0 > 0.0) {
      tmp = 1;
    } else {
      tmp = -1;
    }

    if (u1 > 0.0) {
      tmp_0 = 1;
    } else {
      tmp_0 = -1;
    }

    y = atan2(tmp, tmp_0);
  } else if (u1 == 0.0) {
    if (u0 > 0.0) {
      y = RT_PI / 2.0;
    } else if (u0 < 0.0) {
      y = -(RT_PI / 2.0);
    } else {
      y = 0.0;
    }
  } else {
    y = atan2(u0, u1);
  }

  return y;
}

real_T rt_modd_snf(real_T u0, real_T u1)
{
  real_T y;
  y = u0;
  if (u1 == 0.0) {
    if (u0 == 0.0) {
      y = u1;
    }
  } else if (rtIsNaN(u0) || rtIsNaN(u1) || rtIsInf(u0)) {
    y = (rtNaN);
  } else if (u0 == 0.0) {
    y = 0.0 / u1;
  } else if (rtIsInf(u1)) {
    if ((u1 < 0.0) != (u0 < 0.0)) {
      y = u1;
    }
  } else {
    boolean_T yEq;
    y = fmod(u0, u1);
    yEq = (y == 0.0);
    if ((!yEq) && (u1 > floor(u1))) {
      real_T q;
      q = fabs(u0 / u1);
      yEq = !(fabs(q - floor(q + 0.5)) > DBL_EPSILON * q);
    }

    if (yEq) {
      y = u1 * 0.0;
    } else if ((u0 < 0.0) != (u1 < 0.0)) {
      y += u1;
    }
  }

  return y;
}

static void avg_inflow_SystemCore_setup_lx(dsp_simulink_MovingStandardDe_T *obj)
{
  obj->isSetupComplete = false;
  obj->isInitialized = 1;
  obj->NumChannels = 1;
  obj->FrameLength = 1;
  obj->_pobj0.isInitialized = 0;
  obj->_pobj0.isInitialized = 0;
  obj->pStatistic = &obj->_pobj0;
  obj->isSetupComplete = true;
  obj->TunablePropsChanged = false;
}

static void avg_inflow_SystemCore_setup_l(dsp_simulink_MovingAverage_l_T *obj)
{
  obj->isSetupComplete = false;
  obj->isInitialized = 1;
  obj->NumChannels = 1;
  obj->FrameLength = 1;
  obj->_pobj0.isInitialized = 0;
  obj->_pobj0.isInitialized = 0;
  obj->pStatistic = &obj->_pobj0;
  obj->isSetupComplete = true;
  obj->TunablePropsChanged = false;
}

/* Model step function */
void avg_inflow_step(void)
{
  g_dsp_internal_SlidingWindo_l_T *obj_0;
  g_dsp_internal_SlidingWindowV_T *obj;
  creal_T csumrev[59];
  real_T reverseS[60];
  real_T reverseSamples[60];
  real_T x[60];
  real_T M;
  real_T Mprev;
  real_T S;
  real_T T;
  real_T counter;
  real_T im;
  real_T modValueRev;
  real_T re;
  real_T y;
  real_T z_im;
  int32_T i;

  /* Inport: '<Root>/input_ws_110m' */
  avg_inflow_MovingAverage5(avg_inflow_U.input_ws_110m,
    &avg_inflow_B.MovingAverage5_pn, &avg_inflow_DW.MovingAverage5_pn);
  avg_in_MovingStandardDeviation1(avg_inflow_U.input_ws_110m,
    &avg_inflow_B.MovingStandardDeviation1_pn,
    &avg_inflow_DW.MovingStandardDeviation1_pn);

  /* Sum: '<S5>/Sum' incorporates:
   *  Constant: '<S5>/One'
   *  Delay: '<S5>/Delay'
   */
  avg_inflow_DW.Delay_DSTATE++;

  /* Inport: '<Root>/input_ws_60m' */
  avg_inflow_MovingAverage5(avg_inflow_U.input_ws_60m,
    &avg_inflow_B.MovingAverage5, &avg_inflow_DW.MovingAverage5);
  avg_in_MovingStandardDeviation1(avg_inflow_U.input_ws_60m,
    &avg_inflow_B.MovingStandardDeviation1,
    &avg_inflow_DW.MovingStandardDeviation1);

  /* Sum: '<S3>/Sum' incorporates:
   *  Constant: '<S3>/One'
   *  Delay: '<S3>/Delay'
   */
  avg_inflow_DW.Delay_DSTATE_h++;

  /* Inport: '<Root>/input_wd_110m' */
  avg_inflow_MovingAverage5(avg_inflow_U.input_wd_110m,
    &avg_inflow_B.MovingAverage5_p, &avg_inflow_DW.MovingAverage5_p);
  avg_in_MovingStandardDeviation1(avg_inflow_U.input_wd_110m,
    &avg_inflow_B.MovingStandardDeviation1_p,
    &avg_inflow_DW.MovingStandardDeviation1_p);

  /* Sum: '<S4>/Sum' incorporates:
   *  Constant: '<S4>/One'
   *  Delay: '<S4>/Delay'
   */
  avg_inflow_DW.Delay_DSTATE_o++;

  /* Switch: '<S16>/Switch2' incorporates:
   *  Constant: '<Root>/Constant5'
   *  Constant: '<S5>/Constant'
   *  Delay: '<S5>/Delay'
   *  RelationalOperator: '<S16>/LowerRelop1'
   *  RelationalOperator: '<S16>/UpperRelop'
   *  Switch: '<S16>/Switch'
   */
  /* Bit to Integer Conversion */
  /* Input bit order is MSB first */
  if (avg_inflow_DW.Delay_DSTATE > 60.0) {
    modValueRev = 60.0;
  } else if (avg_inflow_DW.Delay_DSTATE < 0.0) {
    /* Switch: '<S16>/Switch' incorporates:
     *  Constant: '<S5>/Constant'
     */
    modValueRev = 0.0;
  } else {
    modValueRev = avg_inflow_DW.Delay_DSTATE;
  }

  /* Switch: '<S6>/Switch2' incorporates:
   *  Constant: '<Root>/Constant5'
   *  Constant: '<S3>/Constant'
   *  Delay: '<S3>/Delay'
   *  RelationalOperator: '<S6>/LowerRelop1'
   *  RelationalOperator: '<S6>/UpperRelop'
   *  Switch: '<S6>/Switch'
   */
  if (avg_inflow_DW.Delay_DSTATE_h > 60.0) {
    counter = 60.0;
  } else if (avg_inflow_DW.Delay_DSTATE_h < 0.0) {
    /* Switch: '<S6>/Switch' incorporates:
     *  Constant: '<S3>/Constant'
     */
    counter = 0.0;
  } else {
    counter = avg_inflow_DW.Delay_DSTATE_h;
  }

  /* Switch: '<S11>/Switch2' incorporates:
   *  Constant: '<Root>/Constant5'
   *  Constant: '<S4>/Constant'
   *  Delay: '<S4>/Delay'
   *  RelationalOperator: '<S11>/LowerRelop1'
   *  RelationalOperator: '<S11>/UpperRelop'
   *  Switch: '<S11>/Switch'
   */
  if (avg_inflow_DW.Delay_DSTATE_o > 60.0) {
    y = 60.0;
  } else if (avg_inflow_DW.Delay_DSTATE_o < 0.0) {
    /* Switch: '<S11>/Switch' incorporates:
     *  Constant: '<S4>/Constant'
     */
    y = 0.0;
  } else {
    y = avg_inflow_DW.Delay_DSTATE_o;
  }

  /* Outport: '<Root>/output_InflowOK' incorporates:
   *  Constant: '<Root>/Constant5'
   *  Constant: '<S10>/Constant'
   *  Constant: '<S14>/Constant'
   *  Constant: '<S15>/Constant'
   *  Constant: '<S19>/Constant'
   *  Constant: '<S20>/Constant'
   *  Constant: '<S9>/Constant'
   *  DataTypeConversion: '<Root>/Cast To Double'
   *  Logic: '<S12>/Logical Operator'
   *  Logic: '<S13>/Logical Operator'
   *  Logic: '<S17>/Logical Operator'
   *  Logic: '<S18>/Logical Operator'
   *  Logic: '<S3>/Logical Operator'
   *  Logic: '<S4>/Logical Operator'
   *  Logic: '<S5>/Logical Operator'
   *  Logic: '<S7>/Logical Operator'
   *  Logic: '<S8>/Logical Operator'
   *  RelationalOperator: '<S10>/Compare'
   *  RelationalOperator: '<S12>/IsInf'
   *  RelationalOperator: '<S12>/IsNaN'
   *  RelationalOperator: '<S13>/IsInf'
   *  RelationalOperator: '<S13>/IsNaN'
   *  RelationalOperator: '<S14>/Compare'
   *  RelationalOperator: '<S15>/Compare'
   *  RelationalOperator: '<S17>/IsInf'
   *  RelationalOperator: '<S17>/IsNaN'
   *  RelationalOperator: '<S18>/IsInf'
   *  RelationalOperator: '<S18>/IsNaN'
   *  RelationalOperator: '<S19>/Compare'
   *  RelationalOperator: '<S20>/Compare'
   *  RelationalOperator: '<S3>/GreaterThan'
   *  RelationalOperator: '<S4>/GreaterThan'
   *  RelationalOperator: '<S5>/GreaterThan'
   *  RelationalOperator: '<S7>/IsInf'
   *  RelationalOperator: '<S7>/IsNaN'
   *  RelationalOperator: '<S8>/IsInf'
   *  RelationalOperator: '<S8>/IsNaN'
   *  RelationalOperator: '<S9>/Compare'
   *  S-Function (scominttobit): '<Root>/Bit to Integer Converter1'
   *  Switch: '<S11>/Switch2'
   *  Switch: '<S16>/Switch2'
   *  Switch: '<S6>/Switch2'
   */
  avg_inflow_Y.output_InflowOK = (uint8_T)(((uint32_T)
    ((avg_inflow_B.MovingAverage5_pn.MovingAverage5 <= 1.0E-5) || rtIsNaN
     (avg_inflow_B.MovingAverage5_pn.MovingAverage5) || rtIsInf
     (avg_inflow_B.MovingAverage5_pn.MovingAverage5) ||
     ((avg_inflow_B.MovingStandardDeviation1_pn.MovingStandardDeviation1 <=
       1.0E-5) || rtIsNaN
      (avg_inflow_B.MovingStandardDeviation1_pn.MovingStandardDeviation1) ||
      rtIsInf(avg_inflow_B.MovingStandardDeviation1_pn.MovingStandardDeviation1))
     || (modValueRev < 60.0)) << 1U | (uint32_T)
    ((avg_inflow_B.MovingAverage5.MovingAverage5 <= 1.0E-5) || rtIsNaN
     (avg_inflow_B.MovingAverage5.MovingAverage5) || rtIsInf
     (avg_inflow_B.MovingAverage5.MovingAverage5) ||
     ((avg_inflow_B.MovingStandardDeviation1.MovingStandardDeviation1 <= 1.0E-5)
      || rtIsNaN(avg_inflow_B.MovingStandardDeviation1.MovingStandardDeviation1)
      || rtIsInf(avg_inflow_B.MovingStandardDeviation1.MovingStandardDeviation1))
     || (counter < 60.0))) << 1U | (uint32_T)
    ((avg_inflow_B.MovingAverage5_p.MovingAverage5 <= 1.0E-5) || rtIsNaN
     (avg_inflow_B.MovingAverage5_p.MovingAverage5) || rtIsInf
     (avg_inflow_B.MovingAverage5_p.MovingAverage5) ||
     ((avg_inflow_B.MovingStandardDeviation1_p.MovingStandardDeviation1 <=
       1.0E-5) || rtIsNaN
      (avg_inflow_B.MovingStandardDeviation1_p.MovingStandardDeviation1) ||
      rtIsInf(avg_inflow_B.MovingStandardDeviation1_p.MovingStandardDeviation1))
     || (y < 60.0)));

  /* MATLABSystem: '<Root>/Moving Standard Deviation' incorporates:
   *  Inport: '<Root>/input_ws_110m'
   */
  if (avg_inflow_DW.obj.TunablePropsChanged) {
    avg_inflow_DW.obj.TunablePropsChanged = false;
  }

  obj = avg_inflow_DW.obj.pStatistic;
  if (obj->isInitialized != 1) {
    obj->isSetupComplete = false;
    obj->isInitialized = 1;
    for (i = 0; i < 60; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 0.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    for (i = 0; i < 60; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 60; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 1.0;
    obj->pModValueRev = 0.0;
  }

  for (i = 0; i < 60; i++) {
    reverseSamples[i] = obj->pReverseSamples[i];
  }

  for (i = 0; i < 60; i++) {
    x[i] = obj->pReverseT[i];
  }

  for (i = 0; i < 60; i++) {
    reverseS[i] = obj->pReverseS[i];
  }

  T = obj->pT;
  S = obj->pS;
  M = obj->pM;
  counter = obj->pCounter;
  modValueRev = obj->pModValueRev;
  y = 0.0;
  T += avg_inflow_U.input_ws_110m;
  Mprev = M;
  M += 1.0 / counter * (avg_inflow_U.input_ws_110m - M);
  Mprev = avg_inflow_U.input_ws_110m - Mprev;
  S += (counter - 1.0) * Mprev * Mprev / counter;
  if (modValueRev == 0.0) {
    y = (60.0 - counter) / counter * T - x[(int32_T)(60.0 - counter) - 1];
    y = (counter / (((60.0 - counter) + counter) * (60.0 - counter)) * (y * y) +
         (reverseS[(int32_T)(60.0 - counter) - 1] + S)) / 59.0;
  }

  reverseSamples[(int32_T)(60.0 - counter) - 1] = avg_inflow_U.input_ws_110m;
  if (counter < 59.0) {
    counter++;
  } else {
    counter = 1.0;
    memcpy(&x[0], &reverseSamples[0], 60U * sizeof(real_T));
    T = 0.0;
    S = 0.0;
    for (i = 0; i < 59; i++) {
      M = reverseSamples[i];
      x[i + 1] += x[i];
      Mprev = T;
      T += 1.0 / ((real_T)i + 1.0) * (M - T);
      M -= Mprev;
      S += (((real_T)i + 1.0) - 1.0) * M * M / ((real_T)i + 1.0);
      reverseS[i] = S;
    }

    T = 0.0;
    S = 0.0;
    M = 0.0;
  }

  for (i = 0; i < 60; i++) {
    obj->pReverseSamples[i] = reverseSamples[i];
  }

  for (i = 0; i < 60; i++) {
    obj->pReverseT[i] = x[i];
  }

  for (i = 0; i < 60; i++) {
    obj->pReverseS[i] = reverseS[i];
  }

  obj->pT = T;
  obj->pS = S;
  obj->pM = M;
  obj->pCounter = counter;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }

  /* Inport: '<Root>/input_ws_110m' */
  avg_inflow_MovingAverage1(avg_inflow_U.input_ws_110m,
    &avg_inflow_B.MovingAverage2, &avg_inflow_DW.MovingAverage2);

  /* Outport: '<Root>/output_TI' incorporates:
   *  MATLABSystem: '<Root>/Moving Standard Deviation'
   *  Product: '<Root>/Divide'
   */
  avg_inflow_Y.output_TI = sqrt(y) / avg_inflow_B.MovingAverage2.MovingAverage1;

  /* Outport: '<Root>/output_ws_110m' */
  avg_inflow_Y.output_ws_110m = avg_inflow_B.MovingAverage2.MovingAverage1;

  /* Inport: '<Root>/input_ws_60m' */
  avg_inflow_MovingAverage1(avg_inflow_U.input_ws_60m,
    &avg_inflow_B.MovingAverage1_p, &avg_inflow_DW.MovingAverage1_p);

  /* Outport: '<Root>/output_shearExp' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function'
   */
  avg_inflow_Y.output_shearExp = (log(avg_inflow_B.MovingAverage2.MovingAverage1)
    - log(avg_inflow_B.MovingAverage1_p.MovingAverage1)) / 0.60613580357031616;

  /* Outport: '<Root>/output_ws_60m' */
  avg_inflow_Y.output_ws_60m = avg_inflow_B.MovingAverage1_p.MovingAverage1;

  /* Gain: '<S1>/Gain' incorporates:
   *  Inport: '<Root>/input_wd_110m'
   */
  modValueRev = 0.017453292519943295 * avg_inflow_U.input_wd_110m;

  /* MagnitudeAngleToComplex: '<S1>/Magnitude-Angle to Complex' */
  counter = cos(modValueRev);
  y = sin(modValueRev);

  /* MATLABSystem: '<S1>/Moving Average1' incorporates:
   *  MagnitudeAngleToComplex: '<S1>/Magnitude-Angle to Complex'
   */
  if (avg_inflow_DW.obj_j.TunablePropsChanged) {
    avg_inflow_DW.obj_j.TunablePropsChanged = false;
  }

  obj_0 = avg_inflow_DW.obj_j.pStatistic;
  if (obj_0->isInitialized != 1) {
    obj_0->isSetupComplete = false;
    obj_0->isInitialized = 1;
    obj_0->pCumSum.re = 0.0;
    obj_0->pCumSum.im = 0.0;
    for (i = 0; i < 59; i++) {
      obj_0->pCumSumRev[i].re = 0.0;
      obj_0->pCumSumRev[i].im = 0.0;
    }

    obj_0->pCumRevIndex = 1.0;
    obj_0->pModValueRev = 0.0;
    obj_0->isSetupComplete = true;
    obj_0->pCumSum.re = 0.0;
    obj_0->pCumSum.im = 0.0;
    for (i = 0; i < 59; i++) {
      obj_0->pCumSumRev[i].re = 0.0;
      obj_0->pCumSumRev[i].im = 0.0;
    }

    obj_0->pCumRevIndex = 1.0;
    obj_0->pModValueRev = 0.0;
  }

  T = obj_0->pCumRevIndex;
  S = obj_0->pCumSum.re;
  M = obj_0->pCumSum.im;
  for (i = 0; i < 59; i++) {
    csumrev[i] = obj_0->pCumSumRev[i];
  }

  modValueRev = obj_0->pModValueRev;
  Mprev = 0.0;
  z_im = 0.0;
  re = 0.0;
  im = 0.0;
  S += counter;
  M += y;
  if (modValueRev == 0.0) {
    Mprev = csumrev[(int32_T)T - 1].re + S;
    z_im = csumrev[(int32_T)T - 1].im + M;
  }

  csumrev[(int32_T)T - 1].re = counter;
  csumrev[(int32_T)T - 1].im = y;
  if (T != 59.0) {
    T++;
  } else {
    T = 1.0;
    S = 0.0;
    M = 0.0;
    for (i = 57; i >= 0; i--) {
      csumrev[i].re += csumrev[i + 1].re;
      csumrev[i].im += csumrev[i + 1].im;
    }
  }

  if (modValueRev == 0.0) {
    if (z_im == 0.0) {
      re = Mprev / 60.0;
    } else if (Mprev == 0.0) {
      im = z_im / 60.0;
    } else {
      re = Mprev / 60.0;
      im = z_im / 60.0;
    }
  }

  obj_0->pCumSum.re = S;
  obj_0->pCumSum.im = M;
  for (i = 0; i < 59; i++) {
    obj_0->pCumSumRev[i] = csumrev[i];
  }

  obj_0->pCumRevIndex = T;
  if (modValueRev > 0.0) {
    obj_0->pModValueRev = modValueRev - 1.0;
  } else {
    obj_0->pModValueRev = 0.0;
  }

  /* Outport: '<Root>/output_wd_110m' incorporates:
   *  ComplexToMagnitudeAngle: '<S1>/Complex to Magnitude-Angle'
   *  Constant: '<S1>/One1'
   *  Gain: '<S1>/Gain1'
   *  MATLABSystem: '<S1>/Moving Average1'
   *  Math: '<S1>/Mod'
   */
  avg_inflow_Y.output_wd_110m = rt_modd_snf(57.295779513082323 * rt_atan2d_snf
    (im, re), 360.0);
}

/* Model initialize function */
void avg_inflow_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* initialize error status */
  rtmSetErrorStatus(avg_inflow_M, (NULL));

  /* block I/O */
  (void) memset(((void *) &avg_inflow_B), 0,
                sizeof(B_avg_inflow_T));

  /* states (dwork) */
  (void) memset((void *)&avg_inflow_DW, 0,
                sizeof(DW_avg_inflow_T));

  /* external inputs */
  (void)memset(&avg_inflow_U, 0, sizeof(ExtU_avg_inflow_T));

  /* external outputs */
  (void)memset(&avg_inflow_Y, 0, sizeof(ExtY_avg_inflow_T));

  {
    g_dsp_internal_SlidingWindo_l_T *obj_0;
    g_dsp_internal_SlidingWindowV_T *obj;
    int32_T i;

    /* SystemInitialize for Inport: '<Root>/input_ws_110m' */
    avg_inflow_MovingAverage5_Init(&avg_inflow_DW.MovingAverage5_pn);
    a_MovingStandardDeviation1_Init(&avg_inflow_DW.MovingStandardDeviation1_pn);

    /* SystemInitialize for Inport: '<Root>/input_ws_60m' */
    avg_inflow_MovingAverage5_Init(&avg_inflow_DW.MovingAverage5);
    a_MovingStandardDeviation1_Init(&avg_inflow_DW.MovingStandardDeviation1);

    /* SystemInitialize for Inport: '<Root>/input_wd_110m' */
    avg_inflow_MovingAverage5_Init(&avg_inflow_DW.MovingAverage5_p);
    a_MovingStandardDeviation1_Init(&avg_inflow_DW.MovingStandardDeviation1_p);

    /* Start for MATLABSystem: '<Root>/Moving Standard Deviation' */
    avg_inflow_DW.obj.isInitialized = 0;
    avg_inflow_DW.obj.NumChannels = -1;
    avg_inflow_DW.obj.FrameLength = -1;
    avg_inflow_DW.obj.matlabCodegenIsDeleted = false;
    avg_inflow_SystemCore_setup_lx(&avg_inflow_DW.obj);

    /* InitializeConditions for MATLABSystem: '<Root>/Moving Standard Deviation' */
    obj = avg_inflow_DW.obj.pStatistic;
    if (obj->isInitialized == 1) {
      for (i = 0; i < 60; i++) {
        obj->pReverseSamples[i] = 0.0;
      }

      for (i = 0; i < 60; i++) {
        obj->pReverseT[i] = 0.0;
      }

      for (i = 0; i < 60; i++) {
        obj->pReverseS[i] = 0.0;
      }

      obj->pT = 0.0;
      obj->pS = 0.0;
      obj->pM = 0.0;
      obj->pCounter = 1.0;
      obj->pModValueRev = 0.0;
    }

    /* End of InitializeConditions for MATLABSystem: '<Root>/Moving Standard Deviation' */

    /* SystemInitialize for Inport: '<Root>/input_ws_110m' */
    avg_inflow_MovingAverage1_Init(&avg_inflow_DW.MovingAverage2);

    /* SystemInitialize for Inport: '<Root>/input_ws_60m' */
    avg_inflow_MovingAverage1_Init(&avg_inflow_DW.MovingAverage1_p);

    /* Start for MATLABSystem: '<S1>/Moving Average1' */
    avg_inflow_DW.obj_j.isInitialized = 0;
    avg_inflow_DW.obj_j.NumChannels = -1;
    avg_inflow_DW.obj_j.FrameLength = -1;
    avg_inflow_DW.obj_j.matlabCodegenIsDeleted = false;
    avg_inflow_SystemCore_setup_l(&avg_inflow_DW.obj_j);

    /* InitializeConditions for MATLABSystem: '<S1>/Moving Average1' */
    obj_0 = avg_inflow_DW.obj_j.pStatistic;
    if (obj_0->isInitialized == 1) {
      obj_0->pCumSum.re = 0.0;
      obj_0->pCumSum.im = 0.0;
      for (i = 0; i < 59; i++) {
        obj_0->pCumSumRev[i].re = 0.0;
        obj_0->pCumSumRev[i].im = 0.0;
      }

      obj_0->pCumRevIndex = 1.0;
      obj_0->pModValueRev = 0.0;
    }

    /* End of InitializeConditions for MATLABSystem: '<S1>/Moving Average1' */
  }
}

/* Model terminate function */
void avg_inflow_terminate(void)
{
  g_dsp_internal_SlidingWindo_l_T *obj_0;
  g_dsp_internal_SlidingWindowV_T *obj;
  avg_inflow_MovingAverage5_Term(&avg_inflow_DW.MovingAverage5_pn);
  a_MovingStandardDeviation1_Term(&avg_inflow_DW.MovingStandardDeviation1_pn);
  avg_inflow_MovingAverage5_Term(&avg_inflow_DW.MovingAverage5);
  a_MovingStandardDeviation1_Term(&avg_inflow_DW.MovingStandardDeviation1);
  avg_inflow_MovingAverage5_Term(&avg_inflow_DW.MovingAverage5_p);
  a_MovingStandardDeviation1_Term(&avg_inflow_DW.MovingStandardDeviation1_p);

  /* Terminate for MATLABSystem: '<Root>/Moving Standard Deviation' */
  if (!avg_inflow_DW.obj.matlabCodegenIsDeleted) {
    avg_inflow_DW.obj.matlabCodegenIsDeleted = true;
    if ((avg_inflow_DW.obj.isInitialized == 1) &&
        avg_inflow_DW.obj.isSetupComplete) {
      obj = avg_inflow_DW.obj.pStatistic;
      if (obj->isInitialized == 1) {
        obj->isInitialized = 2;
      }

      avg_inflow_DW.obj.NumChannels = -1;
      avg_inflow_DW.obj.FrameLength = -1;
    }
  }

  /* End of Terminate for MATLABSystem: '<Root>/Moving Standard Deviation' */
  avg_inflow_MovingAverage1_Term(&avg_inflow_DW.MovingAverage2);
  avg_inflow_MovingAverage1_Term(&avg_inflow_DW.MovingAverage1_p);

  /* Terminate for MATLABSystem: '<S1>/Moving Average1' */
  if (!avg_inflow_DW.obj_j.matlabCodegenIsDeleted) {
    avg_inflow_DW.obj_j.matlabCodegenIsDeleted = true;
    if ((avg_inflow_DW.obj_j.isInitialized == 1) &&
        avg_inflow_DW.obj_j.isSetupComplete) {
      obj_0 = avg_inflow_DW.obj_j.pStatistic;
      if (obj_0->isInitialized == 1) {
        obj_0->isInitialized = 2;
      }

      avg_inflow_DW.obj_j.NumChannels = -1;
      avg_inflow_DW.obj_j.FrameLength = -1;
    }
  }

  /* End of Terminate for MATLABSystem: '<S1>/Moving Average1' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
