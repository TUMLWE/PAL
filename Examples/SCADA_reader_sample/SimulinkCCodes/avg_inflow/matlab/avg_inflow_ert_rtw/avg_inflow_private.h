/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: avg_inflow_private.h
 *
 * Code generated for Simulink model 'avg_inflow'.
 *
 * Model version                  : 1.5
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Wed Nov 29 01:10:05 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_avg_inflow_private_h_
#define RTW_HEADER_avg_inflow_private_h_
#include "rtwtypes.h"
#include "avg_inflow.h"
#include "avg_inflow_types.h"

extern real_T rt_atan2d_snf(real_T u0, real_T u1);
extern real_T rt_modd_snf(real_T u0, real_T u1);
extern void avg_inflow_MovingAverage1_Init(DW_MovingAverage1_avg_inflow_T
  *localDW);
extern void avg_inflow_MovingAverage1(real_T rtu_0,
  B_MovingAverage1_avg_inflow_T *localB, DW_MovingAverage1_avg_inflow_T *localDW);
extern void avg_inflow_MovingAverage5_Init(DW_MovingAverage5_avg_inflow_T
  *localDW);
extern void avg_inflow_MovingAverage5(real_T rtu_0,
  B_MovingAverage5_avg_inflow_T *localB, DW_MovingAverage5_avg_inflow_T *localDW);
extern void a_MovingStandardDeviation1_Init(DW_MovingStandardDeviation1_a_T
  *localDW);
extern void avg_in_MovingStandardDeviation1(real_T rtu_0,
  B_MovingStandardDeviation1_av_T *localB, DW_MovingStandardDeviation1_a_T
  *localDW);
extern void avg_inflow_MovingAverage1_Term(DW_MovingAverage1_avg_inflow_T
  *localDW);
extern void avg_inflow_MovingAverage5_Term(DW_MovingAverage5_avg_inflow_T
  *localDW);
extern void a_MovingStandardDeviation1_Term(DW_MovingStandardDeviation1_a_T
  *localDW);

#endif                                 /* RTW_HEADER_avg_inflow_private_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
