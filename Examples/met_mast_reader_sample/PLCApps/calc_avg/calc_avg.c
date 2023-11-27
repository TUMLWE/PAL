/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: calc_avg.c
 *
 * Code generated for Simulink model 'calc_avg'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Fri Oct 20 18:01:49 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "calc_avg.h"
#include "rtwtypes.h"
#include "calc_avg_types.h"
#include "calc_avg_private.h"
#include <string.h>
#include <math.h>
#include "rt_nonfinite.h"
#include <float.h>
#include "rt_defines.h"

/* Block signals (default storage) */
B_calc_avg_T calc_avg_B;

/* Block states (default storage) */
DW_calc_avg_T calc_avg_DW;

/* External inputs (root inport signals with default storage) */
ExtU_calc_avg_T calc_avg_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_calc_avg_T calc_avg_Y;

/* Real-time model */
static RT_MODEL_calc_avg_T calc_avg_M_;
RT_MODEL_calc_avg_T *const calc_avg_M = &calc_avg_M_;

/* Forward declaration for local functions */
static void calc_avg_SystemCore_setup(dsp_simulink_MovingAverage_ca_T *obj);

/* Forward declaration for local functions */
static void calc_avg_SystemCore_setup_n(dsp_simulink_MovingAverage_ca_T *obj);

/* Forward declaration for local functions */
static void calc_avg_SystemCore_setup_n2(dsp_simulink_MovingStandardDe_T *obj);

/* Forward declaration for local functions */
static void calc_avg_SystemCore_setup_de(dsp_simulink_MovingStandardDe_T *obj);
static void calc_avg_SystemCore_setup_d(dsp_simulink_MovingAverage_d_T *obj);
static void calc_avg_SystemCore_setup(dsp_simulink_MovingAverage_ca_T *obj)
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
void calc_avg_MovingAverage1_Init(DW_MovingAverage1_calc_avg_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
  int32_T i;

  /* Start for MATLABSystem: '<Root>/Moving Average1' */
  localDW->obj.isInitialized = 0;
  localDW->obj.NumChannels = -1;
  localDW->obj.FrameLength = -1;
  localDW->obj.matlabCodegenIsDeleted = false;
  localDW->objisempty = true;
  calc_avg_SystemCore_setup(&localDW->obj);

  /* InitializeConditions for MATLABSystem: '<Root>/Moving Average1' */
  obj = localDW->obj.pStatistic;
  if (obj->isInitialized == 1) {
    obj->pCumSum = 0.0;
    for (i = 0; i < 5999; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  /* End of InitializeConditions for MATLABSystem: '<Root>/Moving Average1' */
}

/* Output and update for atomic system: */
void calc_avg_MovingAverage1(real_T rtu_0, B_MovingAverage1_calc_avg_T *localB,
  DW_MovingAverage1_calc_avg_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
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
    for (i = 0; i < 5999; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    obj->pCumSum = 0.0;
    for (i = 0; i < 5999; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  cumRevIndex = obj->pCumRevIndex;
  csum = obj->pCumSum;
  for (i = 0; i < 5999; i++) {
    localB->csumrev[i] = obj->pCumSumRev[i];
  }

  modValueRev = obj->pModValueRev;
  z = 0.0;

  /* MATLABSystem: '<Root>/Moving Average1' */
  localB->MovingAverage1 = 0.0;

  /* MATLABSystem: '<Root>/Moving Average1' */
  csum += rtu_0;
  if (modValueRev == 0.0) {
    z = localB->csumrev[(int32_T)cumRevIndex - 1] + csum;
  }

  localB->csumrev[(int32_T)cumRevIndex - 1] = rtu_0;
  if (cumRevIndex != 5999.0) {
    cumRevIndex++;
  } else {
    cumRevIndex = 1.0;
    csum = 0.0;
    for (i = 5997; i >= 0; i--) {
      localB->csumrev[i] += localB->csumrev[i + 1];
    }
  }

  if (modValueRev == 0.0) {
    /* MATLABSystem: '<Root>/Moving Average1' */
    localB->MovingAverage1 = z / 6000.0;
  }

  obj->pCumSum = csum;
  for (i = 0; i < 5999; i++) {
    obj->pCumSumRev[i] = localB->csumrev[i];
  }

  obj->pCumRevIndex = cumRevIndex;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }
}

/* Termination for atomic system: */
void calc_avg_MovingAverage1_Term(DW_MovingAverage1_calc_avg_T *localDW)
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

static void calc_avg_SystemCore_setup_n(dsp_simulink_MovingAverage_ca_T *obj)
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
void calc_avg_MovingAverage5_Init(DW_MovingAverage5_calc_avg_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
  int32_T i;

  /* Start for MATLABSystem: '<S3>/Moving Average5' */
  localDW->obj.isInitialized = 0;
  localDW->obj.NumChannels = -1;
  localDW->obj.FrameLength = -1;
  localDW->obj.matlabCodegenIsDeleted = false;
  localDW->objisempty = true;
  calc_avg_SystemCore_setup_n(&localDW->obj);

  /* InitializeConditions for MATLABSystem: '<S3>/Moving Average5' */
  obj = localDW->obj.pStatistic;
  if (obj->isInitialized == 1) {
    obj->pCumSum = 0.0;
    for (i = 0; i < 5999; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  /* End of InitializeConditions for MATLABSystem: '<S3>/Moving Average5' */
}

/* Output and update for atomic system: */
void calc_avg_MovingAverage5(real_T rtu_0, B_MovingAverage5_calc_avg_T *localB,
  DW_MovingAverage5_calc_avg_T *localDW)
{
  g_dsp_internal_SlidingWindowA_T *obj;
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
    for (i = 0; i < 5999; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    obj->pCumSum = 0.0;
    for (i = 0; i < 5999; i++) {
      obj->pCumSumRev[i] = 0.0;
    }

    obj->pCumRevIndex = 1.0;
    obj->pModValueRev = 0.0;
  }

  cumRevIndex = obj->pCumRevIndex;
  csum = obj->pCumSum;
  for (i = 0; i < 5999; i++) {
    localB->csumrev[i] = obj->pCumSumRev[i];
  }

  modValueRev = obj->pModValueRev;
  z = 0.0;

  /* MATLABSystem: '<S3>/Moving Average5' */
  localB->MovingAverage5 = 0.0;

  /* MATLABSystem: '<S3>/Moving Average5' */
  csum += rtu_0;
  if (modValueRev == 0.0) {
    z = localB->csumrev[(int32_T)cumRevIndex - 1] + csum;
  }

  localB->csumrev[(int32_T)cumRevIndex - 1] = rtu_0;
  if (cumRevIndex != 5999.0) {
    cumRevIndex++;
  } else {
    cumRevIndex = 1.0;
    csum = 0.0;
    for (i = 5997; i >= 0; i--) {
      localB->csumrev[i] += localB->csumrev[i + 1];
    }
  }

  if (modValueRev == 0.0) {
    /* MATLABSystem: '<S3>/Moving Average5' */
    localB->MovingAverage5 = z / 6000.0;
  }

  obj->pCumSum = csum;
  for (i = 0; i < 5999; i++) {
    obj->pCumSumRev[i] = localB->csumrev[i];
  }

  obj->pCumRevIndex = cumRevIndex;
  if (modValueRev > 0.0) {
    obj->pModValueRev = modValueRev - 1.0;
  } else {
    obj->pModValueRev = 0.0;
  }
}

/* Termination for atomic system: */
void calc_avg_MovingAverage5_Term(DW_MovingAverage5_calc_avg_T *localDW)
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

static void calc_avg_SystemCore_setup_n2(dsp_simulink_MovingStandardDe_T *obj)
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
void c_MovingStandardDeviation1_Init(DW_MovingStandardDeviation1_c_T *localDW)
{
  g_dsp_internal_SlidingWindowV_T *obj;
  int32_T i;

  /* Start for MATLABSystem: '<S3>/Moving Standard Deviation1' */
  localDW->obj.isInitialized = 0;
  localDW->obj.NumChannels = -1;
  localDW->obj.FrameLength = -1;
  localDW->obj.matlabCodegenIsDeleted = false;
  localDW->objisempty = true;
  calc_avg_SystemCore_setup_n2(&localDW->obj);

  /* InitializeConditions for MATLABSystem: '<S3>/Moving Standard Deviation1' */
  obj = localDW->obj.pStatistic;
  if (obj->isInitialized == 1) {
    for (i = 0; i < 6000; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
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
void calc_a_MovingStandardDeviation1(real_T rtu_0,
  B_MovingStandardDeviation1_ca_T *localB, DW_MovingStandardDeviation1_c_T
  *localDW)
{
  g_dsp_internal_SlidingWindowV_T *obj;
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
    for (i = 0; i < 6000; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 0.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    for (i = 0; i < 6000; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 1.0;
    obj->pModValueRev = 0.0;
  }

  for (i = 0; i < 6000; i++) {
    localB->reverseSamples[i] = obj->pReverseSamples[i];
  }

  for (i = 0; i < 6000; i++) {
    localB->x[i] = obj->pReverseT[i];
  }

  for (i = 0; i < 6000; i++) {
    localB->reverseS[i] = obj->pReverseS[i];
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
    y = (6000.0 - counter) / counter * T - localB->x[(int32_T)(6000.0 - counter)
      - 1];
    y = (counter / (((6000.0 - counter) + counter) * (6000.0 - counter)) * (y *
          y) + (localB->reverseS[(int32_T)(6000.0 - counter) - 1] + S)) / 5999.0;
  }

  localB->reverseSamples[(int32_T)(6000.0 - counter) - 1] = rtu_0;
  if (counter < 5999.0) {
    counter++;
  } else {
    counter = 1.0;
    memcpy(&localB->x[0], &localB->reverseSamples[0], 6000U * sizeof(real_T));
    T = 0.0;
    S = 0.0;
    for (i = 0; i < 5999; i++) {
      M = localB->reverseSamples[i];
      localB->x[i + 1] += localB->x[i];
      Mprev = T;
      T += 1.0 / ((real_T)i + 1.0) * (M - T);
      M -= Mprev;
      S += (((real_T)i + 1.0) - 1.0) * M * M / ((real_T)i + 1.0);
      localB->reverseS[i] = S;
    }

    T = 0.0;
    S = 0.0;
    M = 0.0;
  }

  for (i = 0; i < 6000; i++) {
    obj->pReverseSamples[i] = localB->reverseSamples[i];
  }

  for (i = 0; i < 6000; i++) {
    obj->pReverseT[i] = localB->x[i];
  }

  for (i = 0; i < 6000; i++) {
    obj->pReverseS[i] = localB->reverseS[i];
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
void c_MovingStandardDeviation1_Term(DW_MovingStandardDeviation1_c_T *localDW)
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

static void calc_avg_SystemCore_setup_de(dsp_simulink_MovingStandardDe_T *obj)
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

static void calc_avg_SystemCore_setup_d(dsp_simulink_MovingAverage_d_T *obj)
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
void calc_avg_step(void)
{
  g_dsp_internal_SlidingWindo_d_T *obj_0;
  g_dsp_internal_SlidingWindowV_T *obj;
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
  calc_avg_MovingAverage5(calc_avg_U.input_ws_110m,
    &calc_avg_B.MovingAverage5_pn, &calc_avg_DW.MovingAverage5_pn);
  calc_a_MovingStandardDeviation1(calc_avg_U.input_ws_110m,
    &calc_avg_B.MovingStandardDeviation1_pn,
    &calc_avg_DW.MovingStandardDeviation1_pn);

  /* Sum: '<S5>/Sum' incorporates:
   *  Constant: '<S5>/One'
   *  Delay: '<S5>/Delay'
   */
  calc_avg_DW.Delay_DSTATE++;

  /* Inport: '<Root>/input_ws_60m' */
  calc_avg_MovingAverage5(calc_avg_U.input_ws_60m, &calc_avg_B.MovingAverage5,
    &calc_avg_DW.MovingAverage5);
  calc_a_MovingStandardDeviation1(calc_avg_U.input_ws_60m,
    &calc_avg_B.MovingStandardDeviation1, &calc_avg_DW.MovingStandardDeviation1);

  /* Sum: '<S3>/Sum' incorporates:
   *  Constant: '<S3>/One'
   *  Delay: '<S3>/Delay'
   */
  calc_avg_DW.Delay_DSTATE_d++;

  /* Inport: '<Root>/input_wd_110m' */
  calc_avg_MovingAverage5(calc_avg_U.input_wd_110m, &calc_avg_B.MovingAverage5_p,
    &calc_avg_DW.MovingAverage5_p);
  calc_a_MovingStandardDeviation1(calc_avg_U.input_wd_110m,
    &calc_avg_B.MovingStandardDeviation1_p,
    &calc_avg_DW.MovingStandardDeviation1_p);

  /* Sum: '<S4>/Sum' incorporates:
   *  Constant: '<S4>/One'
   *  Delay: '<S4>/Delay'
   */
  calc_avg_DW.Delay_DSTATE_j++;

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
  if (calc_avg_DW.Delay_DSTATE > 6000.0) {
    modValueRev = 6000.0;
  } else if (calc_avg_DW.Delay_DSTATE < 0.0) {
    /* Switch: '<S16>/Switch' incorporates:
     *  Constant: '<S5>/Constant'
     */
    modValueRev = 0.0;
  } else {
    modValueRev = calc_avg_DW.Delay_DSTATE;
  }

  /* Switch: '<S6>/Switch2' incorporates:
   *  Constant: '<Root>/Constant5'
   *  Constant: '<S3>/Constant'
   *  Delay: '<S3>/Delay'
   *  RelationalOperator: '<S6>/LowerRelop1'
   *  RelationalOperator: '<S6>/UpperRelop'
   *  Switch: '<S6>/Switch'
   */
  if (calc_avg_DW.Delay_DSTATE_d > 6000.0) {
    counter = 6000.0;
  } else if (calc_avg_DW.Delay_DSTATE_d < 0.0) {
    /* Switch: '<S6>/Switch' incorporates:
     *  Constant: '<S3>/Constant'
     */
    counter = 0.0;
  } else {
    counter = calc_avg_DW.Delay_DSTATE_d;
  }

  /* Switch: '<S11>/Switch2' incorporates:
   *  Constant: '<Root>/Constant5'
   *  Constant: '<S4>/Constant'
   *  Delay: '<S4>/Delay'
   *  RelationalOperator: '<S11>/LowerRelop1'
   *  RelationalOperator: '<S11>/UpperRelop'
   *  Switch: '<S11>/Switch'
   */
  if (calc_avg_DW.Delay_DSTATE_j > 6000.0) {
    y = 6000.0;
  } else if (calc_avg_DW.Delay_DSTATE_j < 0.0) {
    /* Switch: '<S11>/Switch' incorporates:
     *  Constant: '<S4>/Constant'
     */
    y = 0.0;
  } else {
    y = calc_avg_DW.Delay_DSTATE_j;
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
  calc_avg_Y.output_InflowOK = (uint8_T)(((uint32_T)
    ((calc_avg_B.MovingAverage5_pn.MovingAverage5 <= 1.0E-5) || rtIsNaN
     (calc_avg_B.MovingAverage5_pn.MovingAverage5) || rtIsInf
     (calc_avg_B.MovingAverage5_pn.MovingAverage5) ||
     ((calc_avg_B.MovingStandardDeviation1_pn.MovingStandardDeviation1 <= 1.0E-5)
      || rtIsNaN(calc_avg_B.MovingStandardDeviation1_pn.MovingStandardDeviation1)
      || rtIsInf(calc_avg_B.MovingStandardDeviation1_pn.MovingStandardDeviation1))
     || (modValueRev < 6000.0)) << 1U | (uint32_T)
    ((calc_avg_B.MovingAverage5.MovingAverage5 <= 1.0E-5) || rtIsNaN
     (calc_avg_B.MovingAverage5.MovingAverage5) || rtIsInf
     (calc_avg_B.MovingAverage5.MovingAverage5) ||
     ((calc_avg_B.MovingStandardDeviation1.MovingStandardDeviation1 <= 1.0E-5) ||
      rtIsNaN(calc_avg_B.MovingStandardDeviation1.MovingStandardDeviation1) ||
      rtIsInf(calc_avg_B.MovingStandardDeviation1.MovingStandardDeviation1)) ||
     (counter < 6000.0))) << 1U | (uint32_T)
    ((calc_avg_B.MovingAverage5_p.MovingAverage5 <= 1.0E-5) || rtIsNaN
     (calc_avg_B.MovingAverage5_p.MovingAverage5) || rtIsInf
     (calc_avg_B.MovingAverage5_p.MovingAverage5) ||
     ((calc_avg_B.MovingStandardDeviation1_p.MovingStandardDeviation1 <= 1.0E-5)
      || rtIsNaN(calc_avg_B.MovingStandardDeviation1_p.MovingStandardDeviation1)
      || rtIsInf(calc_avg_B.MovingStandardDeviation1_p.MovingStandardDeviation1))
     || (y < 6000.0)));

  /* MATLABSystem: '<Root>/Moving Standard Deviation' incorporates:
   *  Inport: '<Root>/input_ws_110m'
   */
  if (calc_avg_DW.obj.TunablePropsChanged) {
    calc_avg_DW.obj.TunablePropsChanged = false;
  }

  obj = calc_avg_DW.obj.pStatistic;
  if (obj->isInitialized != 1) {
    obj->isSetupComplete = false;
    obj->isInitialized = 1;
    for (i = 0; i < 6000; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 0.0;
    obj->pModValueRev = 0.0;
    obj->isSetupComplete = true;
    for (i = 0; i < 6000; i++) {
      obj->pReverseSamples[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseT[i] = 0.0;
    }

    for (i = 0; i < 6000; i++) {
      obj->pReverseS[i] = 0.0;
    }

    obj->pT = 0.0;
    obj->pS = 0.0;
    obj->pM = 0.0;
    obj->pCounter = 1.0;
    obj->pModValueRev = 0.0;
  }

  for (i = 0; i < 6000; i++) {
    calc_avg_B.reverseSamples[i] = obj->pReverseSamples[i];
  }

  for (i = 0; i < 6000; i++) {
    calc_avg_B.x[i] = obj->pReverseT[i];
  }

  for (i = 0; i < 6000; i++) {
    calc_avg_B.reverseS[i] = obj->pReverseS[i];
  }

  T = obj->pT;
  S = obj->pS;
  M = obj->pM;
  counter = obj->pCounter;
  modValueRev = obj->pModValueRev;
  y = 0.0;
  T += calc_avg_U.input_ws_110m;
  Mprev = M;
  M += 1.0 / counter * (calc_avg_U.input_ws_110m - M);
  Mprev = calc_avg_U.input_ws_110m - Mprev;
  S += (counter - 1.0) * Mprev * Mprev / counter;
  if (modValueRev == 0.0) {
    y = (6000.0 - counter) / counter * T - calc_avg_B.x[(int32_T)(6000.0 -
      counter) - 1];
    y = (counter / (((6000.0 - counter) + counter) * (6000.0 - counter)) * (y *
          y) + (calc_avg_B.reverseS[(int32_T)(6000.0 - counter) - 1] + S)) /
      5999.0;
  }

  calc_avg_B.reverseSamples[(int32_T)(6000.0 - counter) - 1] =
    calc_avg_U.input_ws_110m;
  if (counter < 5999.0) {
    counter++;
  } else {
    counter = 1.0;
    memcpy(&calc_avg_B.x[0], &calc_avg_B.reverseSamples[0], 6000U * sizeof
           (real_T));
    T = 0.0;
    S = 0.0;
    for (i = 0; i < 5999; i++) {
      M = calc_avg_B.reverseSamples[i];
      calc_avg_B.x[i + 1] += calc_avg_B.x[i];
      Mprev = T;
      T += 1.0 / ((real_T)i + 1.0) * (M - T);
      M -= Mprev;
      S += (((real_T)i + 1.0) - 1.0) * M * M / ((real_T)i + 1.0);
      calc_avg_B.reverseS[i] = S;
    }

    T = 0.0;
    S = 0.0;
    M = 0.0;
  }

  for (i = 0; i < 6000; i++) {
    obj->pReverseSamples[i] = calc_avg_B.reverseSamples[i];
  }

  for (i = 0; i < 6000; i++) {
    obj->pReverseT[i] = calc_avg_B.x[i];
  }

  for (i = 0; i < 6000; i++) {
    obj->pReverseS[i] = calc_avg_B.reverseS[i];
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
  calc_avg_MovingAverage1(calc_avg_U.input_ws_110m, &calc_avg_B.MovingAverage2,
    &calc_avg_DW.MovingAverage2);

  /* Outport: '<Root>/output_TI' incorporates:
   *  MATLABSystem: '<Root>/Moving Standard Deviation'
   *  Product: '<Root>/Divide'
   */
  calc_avg_Y.output_TI = sqrt(y) / calc_avg_B.MovingAverage2.MovingAverage1;

  /* Outport: '<Root>/output_ws_110m' */
  calc_avg_Y.output_ws_110m = calc_avg_B.MovingAverage2.MovingAverage1;

  /* Inport: '<Root>/input_ws_60m' */
  calc_avg_MovingAverage1(calc_avg_U.input_ws_60m, &calc_avg_B.MovingAverage1_p,
    &calc_avg_DW.MovingAverage1_p);

  /* Outport: '<Root>/output_shearExp' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function'
   */
  calc_avg_Y.output_shearExp = (log(calc_avg_B.MovingAverage2.MovingAverage1) -
    log(calc_avg_B.MovingAverage1_p.MovingAverage1)) / 0.60613580357031616;

  /* Outport: '<Root>/output_ws_60m' */
  calc_avg_Y.output_ws_60m = calc_avg_B.MovingAverage1_p.MovingAverage1;

  /* Gain: '<S1>/Gain' incorporates:
   *  Inport: '<Root>/input_wd_110m'
   */
  modValueRev = 0.017453292519943295 * calc_avg_U.input_wd_110m;

  /* MagnitudeAngleToComplex: '<S1>/Magnitude-Angle to Complex' */
  counter = cos(modValueRev);
  y = sin(modValueRev);

  /* MATLABSystem: '<S1>/Moving Average1' incorporates:
   *  MagnitudeAngleToComplex: '<S1>/Magnitude-Angle to Complex'
   */
  if (calc_avg_DW.obj_o.TunablePropsChanged) {
    calc_avg_DW.obj_o.TunablePropsChanged = false;
  }

  obj_0 = calc_avg_DW.obj_o.pStatistic;
  if (obj_0->isInitialized != 1) {
    obj_0->isSetupComplete = false;
    obj_0->isInitialized = 1;
    obj_0->pCumSum.re = 0.0;
    obj_0->pCumSum.im = 0.0;
    for (i = 0; i < 5999; i++) {
      obj_0->pCumSumRev[i].re = 0.0;
      obj_0->pCumSumRev[i].im = 0.0;
    }

    obj_0->pCumRevIndex = 1.0;
    obj_0->pModValueRev = 0.0;
    obj_0->isSetupComplete = true;
    obj_0->pCumSum.re = 0.0;
    obj_0->pCumSum.im = 0.0;
    for (i = 0; i < 5999; i++) {
      obj_0->pCumSumRev[i].re = 0.0;
      obj_0->pCumSumRev[i].im = 0.0;
    }

    obj_0->pCumRevIndex = 1.0;
    obj_0->pModValueRev = 0.0;
  }

  T = obj_0->pCumRevIndex;
  S = obj_0->pCumSum.re;
  M = obj_0->pCumSum.im;
  for (i = 0; i < 5999; i++) {
    calc_avg_B.csumrev[i] = obj_0->pCumSumRev[i];
  }

  modValueRev = obj_0->pModValueRev;
  Mprev = 0.0;
  z_im = 0.0;
  re = 0.0;
  im = 0.0;
  S += counter;
  M += y;
  if (modValueRev == 0.0) {
    Mprev = calc_avg_B.csumrev[(int32_T)T - 1].re + S;
    z_im = calc_avg_B.csumrev[(int32_T)T - 1].im + M;
  }

  calc_avg_B.csumrev[(int32_T)T - 1].re = counter;
  calc_avg_B.csumrev[(int32_T)T - 1].im = y;
  if (T != 5999.0) {
    T++;
  } else {
    T = 1.0;
    S = 0.0;
    M = 0.0;
    for (i = 5997; i >= 0; i--) {
      calc_avg_B.csumrev[i].re += calc_avg_B.csumrev[i + 1].re;
      calc_avg_B.csumrev[i].im += calc_avg_B.csumrev[i + 1].im;
    }
  }

  if (modValueRev == 0.0) {
    if (z_im == 0.0) {
      re = Mprev / 6000.0;
    } else if (Mprev == 0.0) {
      im = z_im / 6000.0;
    } else {
      re = Mprev / 6000.0;
      im = z_im / 6000.0;
    }
  }

  obj_0->pCumSum.re = S;
  obj_0->pCumSum.im = M;
  for (i = 0; i < 5999; i++) {
    obj_0->pCumSumRev[i] = calc_avg_B.csumrev[i];
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
  calc_avg_Y.output_wd_110m = rt_modd_snf(57.295779513082323 * rt_atan2d_snf(im,
    re), 360.0);
}

/* Model initialize function */
void calc_avg_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* initialize error status */
  rtmSetErrorStatus(calc_avg_M, (NULL));

  /* block I/O */
  (void) memset(((void *) &calc_avg_B), 0,
                sizeof(B_calc_avg_T));

  /* states (dwork) */
  (void) memset((void *)&calc_avg_DW, 0,
                sizeof(DW_calc_avg_T));

  /* external inputs */
  (void)memset(&calc_avg_U, 0, sizeof(ExtU_calc_avg_T));

  /* external outputs */
  (void)memset(&calc_avg_Y, 0, sizeof(ExtY_calc_avg_T));

  {
    g_dsp_internal_SlidingWindo_d_T *obj_0;
    g_dsp_internal_SlidingWindowV_T *obj;
    int32_T i;

    /* SystemInitialize for Inport: '<Root>/input_ws_110m' */
    calc_avg_MovingAverage5_Init(&calc_avg_DW.MovingAverage5_pn);
    c_MovingStandardDeviation1_Init(&calc_avg_DW.MovingStandardDeviation1_pn);

    /* SystemInitialize for Inport: '<Root>/input_ws_60m' */
    calc_avg_MovingAverage5_Init(&calc_avg_DW.MovingAverage5);
    c_MovingStandardDeviation1_Init(&calc_avg_DW.MovingStandardDeviation1);

    /* SystemInitialize for Inport: '<Root>/input_wd_110m' */
    calc_avg_MovingAverage5_Init(&calc_avg_DW.MovingAverage5_p);
    c_MovingStandardDeviation1_Init(&calc_avg_DW.MovingStandardDeviation1_p);

    /* Start for MATLABSystem: '<Root>/Moving Standard Deviation' */
    calc_avg_DW.obj.isInitialized = 0;
    calc_avg_DW.obj.NumChannels = -1;
    calc_avg_DW.obj.FrameLength = -1;
    calc_avg_DW.obj.matlabCodegenIsDeleted = false;
    calc_avg_SystemCore_setup_de(&calc_avg_DW.obj);

    /* InitializeConditions for MATLABSystem: '<Root>/Moving Standard Deviation' */
    obj = calc_avg_DW.obj.pStatistic;
    if (obj->isInitialized == 1) {
      for (i = 0; i < 6000; i++) {
        obj->pReverseSamples[i] = 0.0;
      }

      for (i = 0; i < 6000; i++) {
        obj->pReverseT[i] = 0.0;
      }

      for (i = 0; i < 6000; i++) {
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
    calc_avg_MovingAverage1_Init(&calc_avg_DW.MovingAverage2);

    /* SystemInitialize for Inport: '<Root>/input_ws_60m' */
    calc_avg_MovingAverage1_Init(&calc_avg_DW.MovingAverage1_p);

    /* Start for MATLABSystem: '<S1>/Moving Average1' */
    calc_avg_DW.obj_o.isInitialized = 0;
    calc_avg_DW.obj_o.NumChannels = -1;
    calc_avg_DW.obj_o.FrameLength = -1;
    calc_avg_DW.obj_o.matlabCodegenIsDeleted = false;
    calc_avg_SystemCore_setup_d(&calc_avg_DW.obj_o);

    /* InitializeConditions for MATLABSystem: '<S1>/Moving Average1' */
    obj_0 = calc_avg_DW.obj_o.pStatistic;
    if (obj_0->isInitialized == 1) {
      obj_0->pCumSum.re = 0.0;
      obj_0->pCumSum.im = 0.0;
      for (i = 0; i < 5999; i++) {
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
void calc_avg_terminate(void)
{
  g_dsp_internal_SlidingWindo_d_T *obj_0;
  g_dsp_internal_SlidingWindowV_T *obj;
  calc_avg_MovingAverage5_Term(&calc_avg_DW.MovingAverage5_pn);
  c_MovingStandardDeviation1_Term(&calc_avg_DW.MovingStandardDeviation1_pn);
  calc_avg_MovingAverage5_Term(&calc_avg_DW.MovingAverage5);
  c_MovingStandardDeviation1_Term(&calc_avg_DW.MovingStandardDeviation1);
  calc_avg_MovingAverage5_Term(&calc_avg_DW.MovingAverage5_p);
  c_MovingStandardDeviation1_Term(&calc_avg_DW.MovingStandardDeviation1_p);

  /* Terminate for MATLABSystem: '<Root>/Moving Standard Deviation' */
  if (!calc_avg_DW.obj.matlabCodegenIsDeleted) {
    calc_avg_DW.obj.matlabCodegenIsDeleted = true;
    if ((calc_avg_DW.obj.isInitialized == 1) && calc_avg_DW.obj.isSetupComplete)
    {
      obj = calc_avg_DW.obj.pStatistic;
      if (obj->isInitialized == 1) {
        obj->isInitialized = 2;
      }

      calc_avg_DW.obj.NumChannels = -1;
      calc_avg_DW.obj.FrameLength = -1;
    }
  }

  /* End of Terminate for MATLABSystem: '<Root>/Moving Standard Deviation' */
  calc_avg_MovingAverage1_Term(&calc_avg_DW.MovingAverage2);
  calc_avg_MovingAverage1_Term(&calc_avg_DW.MovingAverage1_p);

  /* Terminate for MATLABSystem: '<S1>/Moving Average1' */
  if (!calc_avg_DW.obj_o.matlabCodegenIsDeleted) {
    calc_avg_DW.obj_o.matlabCodegenIsDeleted = true;
    if ((calc_avg_DW.obj_o.isInitialized == 1) &&
        calc_avg_DW.obj_o.isSetupComplete) {
      obj_0 = calc_avg_DW.obj_o.pStatistic;
      if (obj_0->isInitialized == 1) {
        obj_0->isInitialized = 2;
      }

      calc_avg_DW.obj_o.NumChannels = -1;
      calc_avg_DW.obj_o.FrameLength = -1;
    }
  }

  /* End of Terminate for MATLABSystem: '<S1>/Moving Average1' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
