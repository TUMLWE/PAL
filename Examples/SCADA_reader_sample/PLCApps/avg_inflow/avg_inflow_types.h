/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: avg_inflow_types.h
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

#ifndef RTW_HEADER_avg_inflow_types_h_
#define RTW_HEADER_avg_inflow_types_h_
#include "rtwtypes.h"
#ifndef struct_tag_XAmU8V6t4q0LukMZ0CTTrB
#define struct_tag_XAmU8V6t4q0LukMZ0CTTrB

struct tag_XAmU8V6t4q0LukMZ0CTTrB
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  real_T pCumSum;
  real_T pCumSumRev[59];
  real_T pCumRevIndex;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_XAmU8V6t4q0LukMZ0CTTrB */

#ifndef typedef_g_dsp_internal_SlidingWindowA_T
#define typedef_g_dsp_internal_SlidingWindowA_T

typedef struct tag_XAmU8V6t4q0LukMZ0CTTrB g_dsp_internal_SlidingWindowA_T;

#endif                             /* typedef_g_dsp_internal_SlidingWindowA_T */

#ifndef struct_tag_BlgwLpgj2bjudmbmVKWwDE
#define struct_tag_BlgwLpgj2bjudmbmVKWwDE

struct tag_BlgwLpgj2bjudmbmVKWwDE
{
  uint32_T f1[8];
};

#endif                                 /* struct_tag_BlgwLpgj2bjudmbmVKWwDE */

#ifndef typedef_cell_wrap_avg_inflow_T
#define typedef_cell_wrap_avg_inflow_T

typedef struct tag_BlgwLpgj2bjudmbmVKWwDE cell_wrap_avg_inflow_T;

#endif                                 /* typedef_cell_wrap_avg_inflow_T */

#ifndef struct_tag_T4ZSt4G6JwynyfaPq3gOjE
#define struct_tag_T4ZSt4G6JwynyfaPq3gOjE

struct tag_T4ZSt4G6JwynyfaPq3gOjE
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_avg_inflow_T inputVarSize;
  g_dsp_internal_SlidingWindowA_T *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  g_dsp_internal_SlidingWindowA_T _pobj0;
};

#endif                                 /* struct_tag_T4ZSt4G6JwynyfaPq3gOjE */

#ifndef typedef_dsp_simulink_MovingAverage_av_T
#define typedef_dsp_simulink_MovingAverage_av_T

typedef struct tag_T4ZSt4G6JwynyfaPq3gOjE dsp_simulink_MovingAverage_av_T;

#endif                             /* typedef_dsp_simulink_MovingAverage_av_T */

#ifndef struct_tag_W7pcbAtzr4cjILZHIPq05
#define struct_tag_W7pcbAtzr4cjILZHIPq05

struct tag_W7pcbAtzr4cjILZHIPq05
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  real_T pReverseSamples[60];
  real_T pReverseT[60];
  real_T pReverseS[60];
  real_T pT;
  real_T pS;
  real_T pM;
  real_T pCounter;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_W7pcbAtzr4cjILZHIPq05 */

#ifndef typedef_g_dsp_internal_SlidingWindowV_T
#define typedef_g_dsp_internal_SlidingWindowV_T

typedef struct tag_W7pcbAtzr4cjILZHIPq05 g_dsp_internal_SlidingWindowV_T;

#endif                             /* typedef_g_dsp_internal_SlidingWindowV_T */

#ifndef struct_tag_TsSS0YphSaYcRnHnrRDi4G
#define struct_tag_TsSS0YphSaYcRnHnrRDi4G

struct tag_TsSS0YphSaYcRnHnrRDi4G
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_avg_inflow_T inputVarSize;
  g_dsp_internal_SlidingWindowV_T *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  g_dsp_internal_SlidingWindowV_T _pobj0;
};

#endif                                 /* struct_tag_TsSS0YphSaYcRnHnrRDi4G */

#ifndef typedef_dsp_simulink_MovingStandardDe_T
#define typedef_dsp_simulink_MovingStandardDe_T

typedef struct tag_TsSS0YphSaYcRnHnrRDi4G dsp_simulink_MovingStandardDe_T;

#endif                             /* typedef_dsp_simulink_MovingStandardDe_T */

#ifndef struct_tag_fSbQUA36QZQ5x67etthPmH
#define struct_tag_fSbQUA36QZQ5x67etthPmH

struct tag_fSbQUA36QZQ5x67etthPmH
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  creal_T pCumSum;
  creal_T pCumSumRev[59];
  real_T pCumRevIndex;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_fSbQUA36QZQ5x67etthPmH */

#ifndef typedef_g_dsp_internal_SlidingWindo_l_T
#define typedef_g_dsp_internal_SlidingWindo_l_T

typedef struct tag_fSbQUA36QZQ5x67etthPmH g_dsp_internal_SlidingWindo_l_T;

#endif                             /* typedef_g_dsp_internal_SlidingWindo_l_T */

#ifndef struct_tag_F2SYEow9mtDnvVIAwZenx
#define struct_tag_F2SYEow9mtDnvVIAwZenx

struct tag_F2SYEow9mtDnvVIAwZenx
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_avg_inflow_T inputVarSize;
  g_dsp_internal_SlidingWindo_l_T *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  g_dsp_internal_SlidingWindo_l_T _pobj0;
};

#endif                                 /* struct_tag_F2SYEow9mtDnvVIAwZenx */

#ifndef typedef_dsp_simulink_MovingAverage_l_T
#define typedef_dsp_simulink_MovingAverage_l_T

typedef struct tag_F2SYEow9mtDnvVIAwZenx dsp_simulink_MovingAverage_l_T;

#endif                              /* typedef_dsp_simulink_MovingAverage_l_T */

/* Forward declaration for rtModel */
typedef struct tag_RTM_avg_inflow_T RT_MODEL_avg_inflow_T;

#endif                                 /* RTW_HEADER_avg_inflow_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
