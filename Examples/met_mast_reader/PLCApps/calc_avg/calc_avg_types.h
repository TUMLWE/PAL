/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: calc_avg_types.h
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

#ifndef RTW_HEADER_calc_avg_types_h_
#define RTW_HEADER_calc_avg_types_h_
#include "rtwtypes.h"
#ifndef struct_tag_ecGlYU5M2GXxUVZuTSH6kD
#define struct_tag_ecGlYU5M2GXxUVZuTSH6kD

struct tag_ecGlYU5M2GXxUVZuTSH6kD
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  real_T pCumSum;
  real_T pCumSumRev[5999];
  real_T pCumRevIndex;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_ecGlYU5M2GXxUVZuTSH6kD */

#ifndef typedef_g_dsp_internal_SlidingWindowA_T
#define typedef_g_dsp_internal_SlidingWindowA_T

typedef struct tag_ecGlYU5M2GXxUVZuTSH6kD g_dsp_internal_SlidingWindowA_T;

#endif                             /* typedef_g_dsp_internal_SlidingWindowA_T */

#ifndef struct_tag_BlgwLpgj2bjudmbmVKWwDE
#define struct_tag_BlgwLpgj2bjudmbmVKWwDE

struct tag_BlgwLpgj2bjudmbmVKWwDE
{
  uint32_T f1[8];
};

#endif                                 /* struct_tag_BlgwLpgj2bjudmbmVKWwDE */

#ifndef typedef_cell_wrap_calc_avg_T
#define typedef_cell_wrap_calc_avg_T

typedef struct tag_BlgwLpgj2bjudmbmVKWwDE cell_wrap_calc_avg_T;

#endif                                 /* typedef_cell_wrap_calc_avg_T */

#ifndef struct_tag_rKScUQSS6qN2Sp349o0TPC
#define struct_tag_rKScUQSS6qN2Sp349o0TPC

struct tag_rKScUQSS6qN2Sp349o0TPC
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_calc_avg_T inputVarSize;
  g_dsp_internal_SlidingWindowA_T *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  g_dsp_internal_SlidingWindowA_T _pobj0;
};

#endif                                 /* struct_tag_rKScUQSS6qN2Sp349o0TPC */

#ifndef typedef_dsp_simulink_MovingAverage_ca_T
#define typedef_dsp_simulink_MovingAverage_ca_T

typedef struct tag_rKScUQSS6qN2Sp349o0TPC dsp_simulink_MovingAverage_ca_T;

#endif                             /* typedef_dsp_simulink_MovingAverage_ca_T */

#ifndef struct_tag_mc80LTkxA91dajiaRpnwiF
#define struct_tag_mc80LTkxA91dajiaRpnwiF

struct tag_mc80LTkxA91dajiaRpnwiF
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  real_T pReverseSamples[6000];
  real_T pReverseT[6000];
  real_T pReverseS[6000];
  real_T pT;
  real_T pS;
  real_T pM;
  real_T pCounter;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_mc80LTkxA91dajiaRpnwiF */

#ifndef typedef_g_dsp_internal_SlidingWindowV_T
#define typedef_g_dsp_internal_SlidingWindowV_T

typedef struct tag_mc80LTkxA91dajiaRpnwiF g_dsp_internal_SlidingWindowV_T;

#endif                             /* typedef_g_dsp_internal_SlidingWindowV_T */

#ifndef struct_tag_IYErJ6Xqcp0jBI50a9ajyF
#define struct_tag_IYErJ6Xqcp0jBI50a9ajyF

struct tag_IYErJ6Xqcp0jBI50a9ajyF
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_calc_avg_T inputVarSize;
  g_dsp_internal_SlidingWindowV_T *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  g_dsp_internal_SlidingWindowV_T _pobj0;
};

#endif                                 /* struct_tag_IYErJ6Xqcp0jBI50a9ajyF */

#ifndef typedef_dsp_simulink_MovingStandardDe_T
#define typedef_dsp_simulink_MovingStandardDe_T

typedef struct tag_IYErJ6Xqcp0jBI50a9ajyF dsp_simulink_MovingStandardDe_T;

#endif                             /* typedef_dsp_simulink_MovingStandardDe_T */

#ifndef struct_tag_jO6LEDDKMe3jGAYONZHGIC
#define struct_tag_jO6LEDDKMe3jGAYONZHGIC

struct tag_jO6LEDDKMe3jGAYONZHGIC
{
  int32_T isInitialized;
  boolean_T isSetupComplete;
  creal_T pCumSum;
  creal_T pCumSumRev[5999];
  real_T pCumRevIndex;
  real_T pModValueRev;
};

#endif                                 /* struct_tag_jO6LEDDKMe3jGAYONZHGIC */

#ifndef typedef_g_dsp_internal_SlidingWindo_d_T
#define typedef_g_dsp_internal_SlidingWindo_d_T

typedef struct tag_jO6LEDDKMe3jGAYONZHGIC g_dsp_internal_SlidingWindo_d_T;

#endif                             /* typedef_g_dsp_internal_SlidingWindo_d_T */

#ifndef struct_tag_umyOQhhROHtMlASaTLhzRC
#define struct_tag_umyOQhhROHtMlASaTLhzRC

struct tag_umyOQhhROHtMlASaTLhzRC
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T TunablePropsChanged;
  cell_wrap_calc_avg_T inputVarSize;
  g_dsp_internal_SlidingWindo_d_T *pStatistic;
  int32_T NumChannels;
  int32_T FrameLength;
  g_dsp_internal_SlidingWindo_d_T _pobj0;
};

#endif                                 /* struct_tag_umyOQhhROHtMlASaTLhzRC */

#ifndef typedef_dsp_simulink_MovingAverage_d_T
#define typedef_dsp_simulink_MovingAverage_d_T

typedef struct tag_umyOQhhROHtMlASaTLhzRC dsp_simulink_MovingAverage_d_T;

#endif                              /* typedef_dsp_simulink_MovingAverage_d_T */

/* Forward declaration for rtModel */
typedef struct tag_RTM_calc_avg_T RT_MODEL_calc_avg_T;

#endif                                 /* RTW_HEADER_calc_avg_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
