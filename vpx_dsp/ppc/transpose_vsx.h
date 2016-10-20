/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_PPC_TRANSPOSE_VSX_H_
#define VPX_DSP_PPC_TRANSPOSE_VSX_H_

#include <altivec.h>

inline void vpx_transpose_8x8_s16(vector signed short v[8]) {
  vector signed short b0, b1, b2, b3, b4, b5, b6, b7;
  vector signed short c0, c1, c2, c3, c4, c5, c6, c7;

  b0 = vec_mergeh(v[0], v[4]);
  b1 = vec_mergel(v[0], v[4]);
  b2 = vec_mergeh(v[1], v[5]);
  b3 = vec_mergel(v[1], v[5]);
  b4 = vec_mergeh(v[2], v[6]);
  b5 = vec_mergel(v[2], v[6]);
  b6 = vec_mergeh(v[3], v[7]);
  b7 = vec_mergel(v[3], v[7]);

  c0 = vec_mergeh(b0, b4);
  c1 = vec_mergel(b0, b4);
  c2 = vec_mergeh(b1, b5);
  c3 = vec_mergel(b1, b5);
  c4 = vec_mergeh(b2, b6);
  c5 = vec_mergel(b2, b6);
  c6 = vec_mergeh(b3, b7);
  c7 = vec_mergel(b3, b7);

  v[0] = vec_mergeh(c0, c4);
  v[1] = vec_mergel(c0, c4);
  v[2] = vec_mergeh(c1, c5);
  v[3] = vec_mergel(c1, c5);
  v[4] = vec_mergeh(c2, c6);
  v[5] = vec_mergel(c2, c6);
  v[6] = vec_mergeh(c3, c7);
  v[7] = vec_mergel(c3, c7);
}

#endif  // VPX_DSP_PPC_TRANSPOSE_VSX_H_
