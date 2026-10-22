#include <cstdio>
#include <cstdint>
#include <cmath>
#include "acl/acl.h"
using LaunchFn = uint32_t (*)(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vabs_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vabs_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vabs_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vabs_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadd_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vadds_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_u16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_u16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_u32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vand_u32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vbitsort_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vbitsort_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vblockreduce_sum_f16_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vblockreduce_sum_f32_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_eq_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_eq_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_eq_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_ge_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_ge_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_ge_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_gt_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_gt_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_gt_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_le_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_le_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_le_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_lt_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_lt_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_lt_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_ne_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_ne_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpv_ne_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_eq_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_eq_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_eq_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_ge_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_ge_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_ge_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_gt_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_gt_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_gt_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_le_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_le_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_le_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_lt_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_lt_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_lt_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_ne_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_ne_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcmpvs_ne_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_bf16f32_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_bf16f32_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_bf16f32_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_bf16f32_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_bf16f32_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_bf16f32_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16f32_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16f32_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16f32_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16f32_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16f32_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16f32_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16s16_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16s16_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16s16_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16s16_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16s16_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f16s16_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32bf16_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32bf16_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32bf16_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32bf16_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32bf16_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32bf16_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32f16_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32f16_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32f16_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32f16_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32f16_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32f16_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32s32_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32s32_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32s32_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32s32_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32s32_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_f32s32_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s16f16_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s16f16_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s16f16_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s16f16_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s16f16_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s16f16_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s32f32_ceil_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s32f32_floor_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s32f32_none_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s32f32_rint_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s32f32_round_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vcvt_s32f32_trunc_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vdiv_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vdiv_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vdiv_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vdiv_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vexp_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vexp_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vexp_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vexp_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vgather_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vgather_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vln_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vln_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vln_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vln_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmadd_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmadd_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmadd_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmadd_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmax_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmaxs_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmin_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmins_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmul_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vmuls_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_u16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_u16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_u32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vor_u32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vreduce_max_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vreduce_max_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vreduce_min_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vreduce_min_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vreduce_sum_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vreduce_sum_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrelu_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrelu_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrelu_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrelu_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrsqrt_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrsqrt_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrsqrt_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vrsqrt_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsel_spr_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsel_spr_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsel_tt_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsel_tt_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_u16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_u16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_u32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshl_u32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_u16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_u16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_u32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vshr_u32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsqrt_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsqrt_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsqrt_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsqrt_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_float_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_float_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_half_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_half_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_s16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_s16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_s32t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vsub_s32t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vxor_16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vxor_16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vxor_u16t_i(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
extern "C" uint32_t aclrtlaunch_bench_vxor_u16t_d(uint32_t, void*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint32_t);
struct Entry { const char *name; LaunchFn indep, dep; };
static const Entry entries[] = {
    {"vabs_float", aclrtlaunch_bench_vabs_float_i, aclrtlaunch_bench_vabs_float_d},
    {"vabs_half", aclrtlaunch_bench_vabs_half_i, aclrtlaunch_bench_vabs_half_d},
    {"vadd_float", aclrtlaunch_bench_vadd_float_i, aclrtlaunch_bench_vadd_float_d},
    {"vadd_half", aclrtlaunch_bench_vadd_half_i, aclrtlaunch_bench_vadd_half_d},
    {"vadd_s16t", aclrtlaunch_bench_vadd_s16t_i, aclrtlaunch_bench_vadd_s16t_d},
    {"vadd_s32t", aclrtlaunch_bench_vadd_s32t_i, aclrtlaunch_bench_vadd_s32t_d},
    {"vadds_float", aclrtlaunch_bench_vadds_float_i, aclrtlaunch_bench_vadds_float_d},
    {"vadds_half", aclrtlaunch_bench_vadds_half_i, aclrtlaunch_bench_vadds_half_d},
    {"vadds_s16t", aclrtlaunch_bench_vadds_s16t_i, aclrtlaunch_bench_vadds_s16t_d},
    {"vadds_s32t", aclrtlaunch_bench_vadds_s32t_i, aclrtlaunch_bench_vadds_s32t_d},
    {"vand_16t", aclrtlaunch_bench_vand_16t_i, aclrtlaunch_bench_vand_16t_d},
    {"vand_32t", aclrtlaunch_bench_vand_32t_i, aclrtlaunch_bench_vand_32t_d},
    {"vand_u16t", aclrtlaunch_bench_vand_u16t_i, aclrtlaunch_bench_vand_u16t_d},
    {"vand_u32t", aclrtlaunch_bench_vand_u32t_i, aclrtlaunch_bench_vand_u32t_d},
    {"vbitsort_float", aclrtlaunch_bench_vbitsort_float_i, nullptr},
    {"vbitsort_half", aclrtlaunch_bench_vbitsort_half_i, nullptr},
    {"vblockreduce_sum_f16", aclrtlaunch_bench_vblockreduce_sum_f16_i, nullptr},
    {"vblockreduce_sum_f32", aclrtlaunch_bench_vblockreduce_sum_f32_i, nullptr},
    {"vcmpv_eq_float", aclrtlaunch_bench_vcmpv_eq_float_i, nullptr},
    {"vcmpv_eq_half", aclrtlaunch_bench_vcmpv_eq_half_i, nullptr},
    {"vcmpv_eq_s32t", aclrtlaunch_bench_vcmpv_eq_s32t_i, nullptr},
    {"vcmpv_ge_float", aclrtlaunch_bench_vcmpv_ge_float_i, nullptr},
    {"vcmpv_ge_half", aclrtlaunch_bench_vcmpv_ge_half_i, nullptr},
    {"vcmpv_ge_s32t", aclrtlaunch_bench_vcmpv_ge_s32t_i, nullptr},
    {"vcmpv_gt_float", aclrtlaunch_bench_vcmpv_gt_float_i, nullptr},
    {"vcmpv_gt_half", aclrtlaunch_bench_vcmpv_gt_half_i, nullptr},
    {"vcmpv_gt_s32t", aclrtlaunch_bench_vcmpv_gt_s32t_i, nullptr},
    {"vcmpv_le_float", aclrtlaunch_bench_vcmpv_le_float_i, nullptr},
    {"vcmpv_le_half", aclrtlaunch_bench_vcmpv_le_half_i, nullptr},
    {"vcmpv_le_s32t", aclrtlaunch_bench_vcmpv_le_s32t_i, nullptr},
    {"vcmpv_lt_float", aclrtlaunch_bench_vcmpv_lt_float_i, nullptr},
    {"vcmpv_lt_half", aclrtlaunch_bench_vcmpv_lt_half_i, nullptr},
    {"vcmpv_lt_s32t", aclrtlaunch_bench_vcmpv_lt_s32t_i, nullptr},
    {"vcmpv_ne_float", aclrtlaunch_bench_vcmpv_ne_float_i, nullptr},
    {"vcmpv_ne_half", aclrtlaunch_bench_vcmpv_ne_half_i, nullptr},
    {"vcmpv_ne_s32t", aclrtlaunch_bench_vcmpv_ne_s32t_i, nullptr},
    {"vcmpvs_eq_float", aclrtlaunch_bench_vcmpvs_eq_float_i, nullptr},
    {"vcmpvs_eq_half", aclrtlaunch_bench_vcmpvs_eq_half_i, nullptr},
    {"vcmpvs_eq_s32t", aclrtlaunch_bench_vcmpvs_eq_s32t_i, nullptr},
    {"vcmpvs_ge_float", aclrtlaunch_bench_vcmpvs_ge_float_i, nullptr},
    {"vcmpvs_ge_half", aclrtlaunch_bench_vcmpvs_ge_half_i, nullptr},
    {"vcmpvs_ge_s32t", aclrtlaunch_bench_vcmpvs_ge_s32t_i, nullptr},
    {"vcmpvs_gt_float", aclrtlaunch_bench_vcmpvs_gt_float_i, nullptr},
    {"vcmpvs_gt_half", aclrtlaunch_bench_vcmpvs_gt_half_i, nullptr},
    {"vcmpvs_gt_s32t", aclrtlaunch_bench_vcmpvs_gt_s32t_i, nullptr},
    {"vcmpvs_le_float", aclrtlaunch_bench_vcmpvs_le_float_i, nullptr},
    {"vcmpvs_le_half", aclrtlaunch_bench_vcmpvs_le_half_i, nullptr},
    {"vcmpvs_le_s32t", aclrtlaunch_bench_vcmpvs_le_s32t_i, nullptr},
    {"vcmpvs_lt_float", aclrtlaunch_bench_vcmpvs_lt_float_i, nullptr},
    {"vcmpvs_lt_half", aclrtlaunch_bench_vcmpvs_lt_half_i, nullptr},
    {"vcmpvs_lt_s32t", aclrtlaunch_bench_vcmpvs_lt_s32t_i, nullptr},
    {"vcmpvs_ne_float", aclrtlaunch_bench_vcmpvs_ne_float_i, nullptr},
    {"vcmpvs_ne_half", aclrtlaunch_bench_vcmpvs_ne_half_i, nullptr},
    {"vcmpvs_ne_s32t", aclrtlaunch_bench_vcmpvs_ne_s32t_i, nullptr},
    {"vcvt_bf16f32_ceil", aclrtlaunch_bench_vcvt_bf16f32_ceil_i, nullptr},
    {"vcvt_bf16f32_floor", aclrtlaunch_bench_vcvt_bf16f32_floor_i, nullptr},
    {"vcvt_bf16f32_none", aclrtlaunch_bench_vcvt_bf16f32_none_i, nullptr},
    {"vcvt_bf16f32_rint", aclrtlaunch_bench_vcvt_bf16f32_rint_i, nullptr},
    {"vcvt_bf16f32_round", aclrtlaunch_bench_vcvt_bf16f32_round_i, nullptr},
    {"vcvt_bf16f32_trunc", aclrtlaunch_bench_vcvt_bf16f32_trunc_i, nullptr},
    {"vcvt_f16f32_ceil", aclrtlaunch_bench_vcvt_f16f32_ceil_i, nullptr},
    {"vcvt_f16f32_floor", aclrtlaunch_bench_vcvt_f16f32_floor_i, nullptr},
    {"vcvt_f16f32_none", aclrtlaunch_bench_vcvt_f16f32_none_i, nullptr},
    {"vcvt_f16f32_rint", aclrtlaunch_bench_vcvt_f16f32_rint_i, nullptr},
    {"vcvt_f16f32_round", aclrtlaunch_bench_vcvt_f16f32_round_i, nullptr},
    {"vcvt_f16f32_trunc", aclrtlaunch_bench_vcvt_f16f32_trunc_i, nullptr},
    {"vcvt_f16s16_ceil", aclrtlaunch_bench_vcvt_f16s16_ceil_i, nullptr},
    {"vcvt_f16s16_floor", aclrtlaunch_bench_vcvt_f16s16_floor_i, nullptr},
    {"vcvt_f16s16_none", aclrtlaunch_bench_vcvt_f16s16_none_i, nullptr},
    {"vcvt_f16s16_rint", aclrtlaunch_bench_vcvt_f16s16_rint_i, nullptr},
    {"vcvt_f16s16_round", aclrtlaunch_bench_vcvt_f16s16_round_i, nullptr},
    {"vcvt_f16s16_trunc", aclrtlaunch_bench_vcvt_f16s16_trunc_i, nullptr},
    {"vcvt_f32bf16_ceil", aclrtlaunch_bench_vcvt_f32bf16_ceil_i, nullptr},
    {"vcvt_f32bf16_floor", aclrtlaunch_bench_vcvt_f32bf16_floor_i, nullptr},
    {"vcvt_f32bf16_none", aclrtlaunch_bench_vcvt_f32bf16_none_i, nullptr},
    {"vcvt_f32bf16_rint", aclrtlaunch_bench_vcvt_f32bf16_rint_i, nullptr},
    {"vcvt_f32bf16_round", aclrtlaunch_bench_vcvt_f32bf16_round_i, nullptr},
    {"vcvt_f32bf16_trunc", aclrtlaunch_bench_vcvt_f32bf16_trunc_i, nullptr},
    {"vcvt_f32f16_ceil", aclrtlaunch_bench_vcvt_f32f16_ceil_i, nullptr},
    {"vcvt_f32f16_floor", aclrtlaunch_bench_vcvt_f32f16_floor_i, nullptr},
    {"vcvt_f32f16_none", aclrtlaunch_bench_vcvt_f32f16_none_i, nullptr},
    {"vcvt_f32f16_rint", aclrtlaunch_bench_vcvt_f32f16_rint_i, nullptr},
    {"vcvt_f32f16_round", aclrtlaunch_bench_vcvt_f32f16_round_i, nullptr},
    {"vcvt_f32f16_trunc", aclrtlaunch_bench_vcvt_f32f16_trunc_i, nullptr},
    {"vcvt_f32s32_ceil", aclrtlaunch_bench_vcvt_f32s32_ceil_i, nullptr},
    {"vcvt_f32s32_floor", aclrtlaunch_bench_vcvt_f32s32_floor_i, nullptr},
    {"vcvt_f32s32_none", aclrtlaunch_bench_vcvt_f32s32_none_i, nullptr},
    {"vcvt_f32s32_rint", aclrtlaunch_bench_vcvt_f32s32_rint_i, nullptr},
    {"vcvt_f32s32_round", aclrtlaunch_bench_vcvt_f32s32_round_i, nullptr},
    {"vcvt_f32s32_trunc", aclrtlaunch_bench_vcvt_f32s32_trunc_i, nullptr},
    {"vcvt_s16f16_ceil", aclrtlaunch_bench_vcvt_s16f16_ceil_i, nullptr},
    {"vcvt_s16f16_floor", aclrtlaunch_bench_vcvt_s16f16_floor_i, nullptr},
    {"vcvt_s16f16_none", aclrtlaunch_bench_vcvt_s16f16_none_i, nullptr},
    {"vcvt_s16f16_rint", aclrtlaunch_bench_vcvt_s16f16_rint_i, nullptr},
    {"vcvt_s16f16_round", aclrtlaunch_bench_vcvt_s16f16_round_i, nullptr},
    {"vcvt_s16f16_trunc", aclrtlaunch_bench_vcvt_s16f16_trunc_i, nullptr},
    {"vcvt_s32f32_ceil", aclrtlaunch_bench_vcvt_s32f32_ceil_i, nullptr},
    {"vcvt_s32f32_floor", aclrtlaunch_bench_vcvt_s32f32_floor_i, nullptr},
    {"vcvt_s32f32_none", aclrtlaunch_bench_vcvt_s32f32_none_i, nullptr},
    {"vcvt_s32f32_rint", aclrtlaunch_bench_vcvt_s32f32_rint_i, nullptr},
    {"vcvt_s32f32_round", aclrtlaunch_bench_vcvt_s32f32_round_i, nullptr},
    {"vcvt_s32f32_trunc", aclrtlaunch_bench_vcvt_s32f32_trunc_i, nullptr},
    {"vdiv_float", aclrtlaunch_bench_vdiv_float_i, aclrtlaunch_bench_vdiv_float_d},
    {"vdiv_half", aclrtlaunch_bench_vdiv_half_i, aclrtlaunch_bench_vdiv_half_d},
    {"vexp_float", aclrtlaunch_bench_vexp_float_i, aclrtlaunch_bench_vexp_float_d},
    {"vexp_half", aclrtlaunch_bench_vexp_half_i, aclrtlaunch_bench_vexp_half_d},
    {"vgather_float", aclrtlaunch_bench_vgather_float_i, nullptr},
    {"vgather_half", aclrtlaunch_bench_vgather_half_i, nullptr},
    {"vln_float", aclrtlaunch_bench_vln_float_i, aclrtlaunch_bench_vln_float_d},
    {"vln_half", aclrtlaunch_bench_vln_half_i, aclrtlaunch_bench_vln_half_d},
    {"vmadd_float", aclrtlaunch_bench_vmadd_float_i, aclrtlaunch_bench_vmadd_float_d},
    {"vmadd_half", aclrtlaunch_bench_vmadd_half_i, aclrtlaunch_bench_vmadd_half_d},
    {"vmax_float", aclrtlaunch_bench_vmax_float_i, aclrtlaunch_bench_vmax_float_d},
    {"vmax_half", aclrtlaunch_bench_vmax_half_i, aclrtlaunch_bench_vmax_half_d},
    {"vmax_s16t", aclrtlaunch_bench_vmax_s16t_i, aclrtlaunch_bench_vmax_s16t_d},
    {"vmax_s32t", aclrtlaunch_bench_vmax_s32t_i, aclrtlaunch_bench_vmax_s32t_d},
    {"vmaxs_float", aclrtlaunch_bench_vmaxs_float_i, aclrtlaunch_bench_vmaxs_float_d},
    {"vmaxs_half", aclrtlaunch_bench_vmaxs_half_i, aclrtlaunch_bench_vmaxs_half_d},
    {"vmaxs_s16t", aclrtlaunch_bench_vmaxs_s16t_i, aclrtlaunch_bench_vmaxs_s16t_d},
    {"vmaxs_s32t", aclrtlaunch_bench_vmaxs_s32t_i, aclrtlaunch_bench_vmaxs_s32t_d},
    {"vmin_float", aclrtlaunch_bench_vmin_float_i, aclrtlaunch_bench_vmin_float_d},
    {"vmin_half", aclrtlaunch_bench_vmin_half_i, aclrtlaunch_bench_vmin_half_d},
    {"vmin_s16t", aclrtlaunch_bench_vmin_s16t_i, aclrtlaunch_bench_vmin_s16t_d},
    {"vmin_s32t", aclrtlaunch_bench_vmin_s32t_i, aclrtlaunch_bench_vmin_s32t_d},
    {"vmins_float", aclrtlaunch_bench_vmins_float_i, aclrtlaunch_bench_vmins_float_d},
    {"vmins_half", aclrtlaunch_bench_vmins_half_i, aclrtlaunch_bench_vmins_half_d},
    {"vmins_s16t", aclrtlaunch_bench_vmins_s16t_i, aclrtlaunch_bench_vmins_s16t_d},
    {"vmins_s32t", aclrtlaunch_bench_vmins_s32t_i, aclrtlaunch_bench_vmins_s32t_d},
    {"vmul_float", aclrtlaunch_bench_vmul_float_i, aclrtlaunch_bench_vmul_float_d},
    {"vmul_half", aclrtlaunch_bench_vmul_half_i, aclrtlaunch_bench_vmul_half_d},
    {"vmul_s16t", aclrtlaunch_bench_vmul_s16t_i, aclrtlaunch_bench_vmul_s16t_d},
    {"vmul_s32t", aclrtlaunch_bench_vmul_s32t_i, aclrtlaunch_bench_vmul_s32t_d},
    {"vmuls_float", aclrtlaunch_bench_vmuls_float_i, aclrtlaunch_bench_vmuls_float_d},
    {"vmuls_half", aclrtlaunch_bench_vmuls_half_i, aclrtlaunch_bench_vmuls_half_d},
    {"vmuls_s16t", aclrtlaunch_bench_vmuls_s16t_i, aclrtlaunch_bench_vmuls_s16t_d},
    {"vmuls_s32t", aclrtlaunch_bench_vmuls_s32t_i, aclrtlaunch_bench_vmuls_s32t_d},
    {"vor_16t", aclrtlaunch_bench_vor_16t_i, aclrtlaunch_bench_vor_16t_d},
    {"vor_32t", aclrtlaunch_bench_vor_32t_i, aclrtlaunch_bench_vor_32t_d},
    {"vor_u16t", aclrtlaunch_bench_vor_u16t_i, aclrtlaunch_bench_vor_u16t_d},
    {"vor_u32t", aclrtlaunch_bench_vor_u32t_i, aclrtlaunch_bench_vor_u32t_d},
    {"vreduce_max_float", aclrtlaunch_bench_vreduce_max_float_i, nullptr},
    {"vreduce_max_half", aclrtlaunch_bench_vreduce_max_half_i, nullptr},
    {"vreduce_min_float", aclrtlaunch_bench_vreduce_min_float_i, nullptr},
    {"vreduce_min_half", aclrtlaunch_bench_vreduce_min_half_i, nullptr},
    {"vreduce_sum_float", aclrtlaunch_bench_vreduce_sum_float_i, nullptr},
    {"vreduce_sum_half", aclrtlaunch_bench_vreduce_sum_half_i, nullptr},
    {"vrelu_float", aclrtlaunch_bench_vrelu_float_i, aclrtlaunch_bench_vrelu_float_d},
    {"vrelu_half", aclrtlaunch_bench_vrelu_half_i, aclrtlaunch_bench_vrelu_half_d},
    {"vrsqrt_float", aclrtlaunch_bench_vrsqrt_float_i, aclrtlaunch_bench_vrsqrt_float_d},
    {"vrsqrt_half", aclrtlaunch_bench_vrsqrt_half_i, aclrtlaunch_bench_vrsqrt_half_d},
    {"vsel_spr_float", aclrtlaunch_bench_vsel_spr_float_i, nullptr},
    {"vsel_spr_half", aclrtlaunch_bench_vsel_spr_half_i, nullptr},
    {"vsel_tt_float", aclrtlaunch_bench_vsel_tt_float_i, nullptr},
    {"vsel_tt_half", aclrtlaunch_bench_vsel_tt_half_i, nullptr},
    {"vshl_16t", aclrtlaunch_bench_vshl_16t_i, aclrtlaunch_bench_vshl_16t_d},
    {"vshl_32t", aclrtlaunch_bench_vshl_32t_i, aclrtlaunch_bench_vshl_32t_d},
    {"vshl_u16t", aclrtlaunch_bench_vshl_u16t_i, aclrtlaunch_bench_vshl_u16t_d},
    {"vshl_u32t", aclrtlaunch_bench_vshl_u32t_i, aclrtlaunch_bench_vshl_u32t_d},
    {"vshr_16t", aclrtlaunch_bench_vshr_16t_i, aclrtlaunch_bench_vshr_16t_d},
    {"vshr_32t", aclrtlaunch_bench_vshr_32t_i, aclrtlaunch_bench_vshr_32t_d},
    {"vshr_u16t", aclrtlaunch_bench_vshr_u16t_i, aclrtlaunch_bench_vshr_u16t_d},
    {"vshr_u32t", aclrtlaunch_bench_vshr_u32t_i, aclrtlaunch_bench_vshr_u32t_d},
    {"vsqrt_float", aclrtlaunch_bench_vsqrt_float_i, aclrtlaunch_bench_vsqrt_float_d},
    {"vsqrt_half", aclrtlaunch_bench_vsqrt_half_i, aclrtlaunch_bench_vsqrt_half_d},
    {"vsub_float", aclrtlaunch_bench_vsub_float_i, aclrtlaunch_bench_vsub_float_d},
    {"vsub_half", aclrtlaunch_bench_vsub_half_i, aclrtlaunch_bench_vsub_half_d},
    {"vsub_s16t", aclrtlaunch_bench_vsub_s16t_i, aclrtlaunch_bench_vsub_s16t_d},
    {"vsub_s32t", aclrtlaunch_bench_vsub_s32t_i, aclrtlaunch_bench_vsub_s32t_d},
    {"vxor_16t", aclrtlaunch_bench_vxor_16t_i, aclrtlaunch_bench_vxor_16t_d},
    {"vxor_u16t", aclrtlaunch_bench_vxor_u16t_i, aclrtlaunch_bench_vxor_u16t_d},
};

int main()
{
    if (aclInit(nullptr) != ACL_SUCCESS) return 1;
    if (aclrtSetDevice(0) != ACL_SUCCESS) return 1;
    aclrtStream stream = nullptr;
    aclrtCreateStream(&stream);
    const size_t xySize = 256 * 4, zSize = 8 * 256 * 4, iSize = 8 * 256 * 4;
    uint16_t *xH; int64_t *iH;
    aclrtMallocHost((void **)&xH, xySize);
    aclrtMallocHost((void **)&iH, 8);
    for (size_t k = 0; k < 256; k++) xH[k] = 0x3C00;
    uint8_t *xD, *yD, *zD, *iD;
    aclrtMalloc((void **)&xD, xySize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&yD, xySize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&zD, zSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&iD, iSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMemcpy(xD, xySize, xH, xySize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(yD, xySize, xH, xySize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(iD, iSize, xH, xySize, ACL_MEMCPY_HOST_TO_DEVICE);

    const uint32_t R1 = 50000, R2 = 500000;
    const int TRIALS = 5;
    printf("%-28s %12s %12s\n", "变体", "II(cyc/条)", "延迟(cyc/条)");
    for (auto &e : entries) {
        auto readCyc = [&](LaunchFn fn, uint32_t r) -> double {
            if (!fn) return -1;
            int64_t zero = 0;
            aclrtMemcpy(iD, 8, &zero, 8, ACL_MEMCPY_HOST_TO_DEVICE);
            fn(1, stream, xD, yD, zD, iD, r);
            aclrtSynchronizeStream(stream);
            aclrtMemcpy(iH, 8, iD, 8, ACL_MEMCPY_DEVICE_TO_HOST);
            return (double)iH[0];
        };
        double ii_sum = 0, lat_sum = 0;
        int ii_n = 0, lat_n = 0;
        for (int t = 0; t < TRIALS; t++) {
            double c1 = readCyc(e.indep, R1), c2 = readCyc(e.indep, R2);
            if (c1 > 0 && c2 > c1) { ii_sum += (c2 - c1) / (double)(R2 - R1) / 8.0; ii_n++; }
            double d1 = readCyc(e.dep, R1), d2 = readCyc(e.dep, R2);
            if (d1 > 0 && d2 > d1) { lat_sum += (d2 - d1) / (double)(R2 - R1); lat_n++; }
        }
        double ii = ii_n > 0 ? ii_sum / ii_n * 36.0 : -1;
        double lat = lat_n > 0 ? lat_sum / lat_n * 36.0 : -1;
        printf("%-28s %12.3f %12.3f\n", e.name, ii, lat);
    }
    aclrtFree(xD); aclrtFree(yD); aclrtFree(zD); aclrtFree(iD);
    aclrtFreeHost(xH); aclrtFreeHost(iH);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();
    return 0;
}
