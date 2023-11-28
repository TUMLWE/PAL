/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: calc_avg_private.h
 *
 * Code generated for Simulink model 'calc_avg'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Tue Nov 28 16:10:08 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_calc_avg_private_h_
#define RTW_HEADER_calc_avg_private_h_
#include "rtwtypes.h"
#include "calc_avg.h"
#include "calc_avg_types.h"

extern real_T rt_atan2d_snf(real_T u0, real_T u1);
extern real_T rt_modd_snf(real_T u0, real_T u1);
extern void calc_avg_MovingAverage1_Init(DW_MovingAverage1_calc_avg_T *localDW);
extern void calc_avg_MovingAverage1(real_T rtu_0, B_MovingAverage1_calc_avg_T
  *localB, DW_MovingAverage1_calc_avg_T *localDW);
extern void calc_avg_MovingAverage5_Init(DW_MovingAverage5_calc_avg_T *localDW);
extern void calc_avg_MovingAverage5(real_T rtu_0, B_MovingAverage5_calc_avg_T
  *localB, DW_MovingAverage5_calc_avg_T *localDW);
extern void c_MovingStandardDeviation1_Init(DW_MovingStandardDeviation1_c_T
  *localDW);
extern void calc_a_MovingStandardDeviation1(real_T rtu_0,
  B_MovingStandardDeviation1_ca_T *localB, DW_MovingStandardDeviation1_c_T
  *localDW);
extern void calc_avg_MovingAverage1_Term(DW_MovingAverage1_calc_avg_T *localDW);
extern void calc_avg_MovingAverage5_Term(DW_MovingAverage5_calc_avg_T *localDW);
extern void c_MovingStandardDeviation1_Term(DW_MovingStandardDeviation1_c_T
  *localDW);

#endif                                 /* RTW_HEADER_calc_avg_private_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
