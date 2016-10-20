/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <altivec.h>
#include "vpx_dsp_rtcd.h"
#include "transpose_vsx.h"

static void vpx_hadamard_8x8_s16_one_pass(vector signed short v[8]) {
  const vector signed short b0 = vec_add(v[0], v[1]);
  const vector signed short b1 = vec_sub(v[0], v[1]);
  const vector signed short b2 = vec_add(v[2], v[3]);
  const vector signed short b3 = vec_sub(v[2], v[3]);
  const vector signed short b4 = vec_add(v[4], v[5]);
  const vector signed short b5 = vec_sub(v[4], v[5]);
  const vector signed short b6 = vec_add(v[6], v[7]);
  const vector signed short b7 = vec_sub(v[6], v[7]);

  const vector signed short c0 = vec_add(b0, b2);
  const vector signed short c1 = vec_add(b1, b3);
  const vector signed short c2 = vec_sub(b0, b2);
  const vector signed short c3 = vec_sub(b1, b3);
  const vector signed short c4 = vec_add(b4, b6);
  const vector signed short c5 = vec_add(b5, b7);
  const vector signed short c6 = vec_sub(b4, b6);
  const vector signed short c7 = vec_sub(b5, b7);

  v[0] = vec_add(c0, c4);
  v[1] = vec_sub(c2, c6);
  v[2] = vec_sub(c0, c4);
  v[3] = vec_add(c2, c6);
  v[4] = vec_add(c3, c7);
  v[5] = vec_sub(c3, c7);
  v[6] = vec_sub(c1, c5);
  v[7] = vec_add(c1, c5);
}

void vpx_hadamard_8x8_vsx(const int16_t *src_diff, int src_stride,
                          int16_t *coeff) {
  vector signed short v[8];

  v[0] = vec_vsx_ld(0, src_diff);
  v[1] = vec_vsx_ld(0, src_diff + src_stride);
  v[2] = vec_vsx_ld(0, src_diff + ( 2 * src_stride));
  v[3] = vec_vsx_ld(0, src_diff + ( 3 * src_stride));
  v[4] = vec_vsx_ld(0, src_diff + ( 4 * src_stride));
  v[5] = vec_vsx_ld(0, src_diff + ( 5 * src_stride));
  v[6] = vec_vsx_ld(0, src_diff + ( 6 * src_stride));
  v[7] = vec_vsx_ld(0, src_diff + ( 7 * src_stride));

  vpx_transpose_8x8_s16(v);

  vpx_hadamard_8x8_s16_one_pass(v);

  vpx_transpose_8x8_s16(v);

  vpx_hadamard_8x8_s16_one_pass(v);

  vec_vsx_st(v[0], 0, coeff);
  vec_vsx_st(v[1], 0, coeff + 8);
  vec_vsx_st(v[2], 0, coeff + 16);
  vec_vsx_st(v[3], 0, coeff + 24);
  vec_vsx_st(v[4], 0, coeff + 32);
  vec_vsx_st(v[5], 0, coeff + 40);
  vec_vsx_st(v[6], 0, coeff + 48);
  vec_vsx_st(v[7], 0, coeff + 56);
}

void vpx_hadamard_16x16_vsx(const int16_t *src_diff, int src_stride,
                            int16_t *coeff) {
  int i;
  const vector unsigned short ones = vec_splat_u16(1);

  /* Rearrange 16x16 to 8x32 and remove stride.
   * Top left first. */
  vpx_hadamard_8x8_vsx(src_diff, src_stride, coeff);
  /* Top right. */
  vpx_hadamard_8x8_vsx(src_diff + 8 + 0 * src_stride, src_stride, coeff + 64);
  /* Bottom left. */
  vpx_hadamard_8x8_vsx(src_diff + 0 + 8 * src_stride, src_stride, coeff + 128);
  /* Bottom right. */
  vpx_hadamard_8x8_vsx(src_diff + 8 + 8 * src_stride, src_stride, coeff + 192);

  /* Overlay the 8x8 blocks and combine. */
  for (i = 0; i < 64; i += 8) {
    const vector signed short a0 = vec_vsx_ld(0, coeff);
    const vector signed short a1 = vec_vsx_ld(0, coeff + 64);
    const vector signed short a2 = vec_vsx_ld(0, coeff + 128);
    const vector signed short a3 = vec_vsx_ld(0, coeff + 192);

    /* Prevent the result from escaping int16_t. */
    const vector signed short b0 = vec_sra(a0,ones);
    const vector signed short b1 = vec_sra(a1,ones);
    const vector signed short b2 = vec_sra(a2,ones);
    const vector signed short b3 = vec_sra(a3,ones);

    const vector signed short c0 = vec_add(b0, b1);
    const vector signed short c2 = vec_add(b2, b3);
    const vector signed short c1 = vec_sub(b0, b1);
    const vector signed short c3 = vec_sub(b2, b3);

    const vector signed short d0 = vec_add(c0, c2);
    const vector signed short d1 = vec_add(c1, c3);
    const vector signed short d2 = vec_sub(c0, c2);
    const vector signed short d3 = vec_sub(c1, c3);

    vec_vsx_st(d0, 0, coeff);
    vec_vsx_st(d1, 0, coeff + 64);
    vec_vsx_st(d2, 0, coeff + 128);
    vec_vsx_st(d3, 0, coeff + 192);

    coeff += 8;
  }
}
