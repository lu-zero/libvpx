/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
//  Check Time for VPX Hadamard functions

#include <stdio.h>
#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

// -----------------------------------------------------------------------------

namespace {

typedef void (*VpxHadamardFunc)(const int16_t *src_diff,
                                int src_stride, int16_t *coeff);

DECLARE_ALIGNED(16, int16_t, input_8x8[64]) = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1,-1,-1,-1,-1,
  1, 1,-1,-1,-1,-1, 1, 1,
  1, 1,-1,-1, 1, 1,-1,-1,
  1,-1,-1, 1, 1,-1,-1, 1,
  1,-1,-1, 1,-1, 1, 1,-1,
  1,-1, 1,-1,-1, 1,-1, 1,
  1,-1, 1,-1, 1,-1, 1,-1
};

DECLARE_ALIGNED(16, int16_t, input_16x16[256]) = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1,-1, 1,-1, 1,-1, 1,-1, 1,-1, 1,-1, 1,-1, 1,-1,
  1, 1,-1,-1, 1, 1,-1,-1, 1, 1,-1,-1, 1, 1,-1,-1,
  1,-1,-1, 1, 1,-1,-1, 1, 1,-1,-1, 1, 1,-1,-1, 1,
  1, 1, 1, 1,-1,-1,-1,-1, 1, 1, 1, 1,-1,-1,-1,-1,
  1,-1, 1,-1,-1, 1,-1, 1, 1,-1, 1,-1,-1, 1,-1, 1,
  1, 1,-1,-1,-1,-1, 1, 1, 1, 1,-1,-1,-1,-1, 1, 1,
  1,-1,-1, 1,-1, 1, 1,-1, 1,-1,-1, 1,-1, 1, 1,-1,
  1, 1, 1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,
  1,-1, 1,-1, 1,-1, 1,-1,-1, 1,-1, 1,-1, 1,-1, 1,
  1, 1,-1,-1, 1, 1,-1,-1,-1,-1, 1, 1,-1,-1, 1, 1,
  1,-1,-1, 1, 1,-1,-1, 1,-1, 1, 1,-1,-1, 1, 1,-1,
  1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1,
  1,-1, 1,-1,-1, 1,-1, 1,-1, 1,-1, 1, 1,-1, 1,-1,
  1, 1,-1,-1,-1,-1, 1, 1,-1,-1, 1, 1, 1, 1,-1,-1,
  1,-1,-1, 1,-1, 1, 1,-1,-1, 1, 1,-1, 1,-1,-1, 1,
};

void TestHadamard(const char *name, VpxHadamardFunc const func,
                  const int16_t *input, int strides, int16_t *output,
                  int times) {
  int i, j;
  vpx_usec_timer timer;

  vpx_usec_timer_start(&timer);
  for (i = 0; i < times; ++i) {
    for (j = 0; j <= strides; ++j) {
      func(input, j, output);
    }
  }
  vpx_usec_timer_mark(&timer);

  const int elapsed_time = static_cast<int>(vpx_usec_timer_elapsed(&timer));

  printf("%s[%12d runs]: %d us\n", name, times, elapsed_time);
}

void TestHadamard8x8(VpxHadamardFunc const func, int times) {
  DECLARE_ALIGNED(16, int16_t, output[64]) = {0};
  TestHadamard("Hadamard8x8", func, input_8x8, 8, output, times);
}

void TestHadamard16x16(VpxHadamardFunc const func, int times) {
  DECLARE_ALIGNED(16, int16_t, output[256]) = {0};
  TestHadamard("Hadamard16x16", func, input_16x16, 16, output, times);
}

void TestHadamard8x8_All(VpxHadamardFunc const func) {
  TestHadamard8x8(func, 10);
  TestHadamard8x8(func, 100);
  TestHadamard8x8(func, 1000);
  TestHadamard8x8(func, 10000);
  TestHadamard8x8(func, 100000);
  TestHadamard8x8(func, 1000000);
}

void TestHadamard16x16_All(VpxHadamardFunc const func) {
  TestHadamard16x16(func, 10);
  TestHadamard16x16(func, 100);
  TestHadamard16x16(func, 1000);
  TestHadamard16x16(func, 10000);
  TestHadamard16x16(func, 100000);
  TestHadamard16x16(func, 1000000);
}

}  // namespace

// Defines a test case for |arch| (e.g., C, VSX, ...) passing the hadamard
// to |test_func|. The test name is 'arch.test_func', e.g., C.TestHadamard8x8.
#define HADAMARD_TEST(arch, test_func, hadamard)\
  TEST(arch, test_func) {                       \
    test_func(hadamard);                        \
  }

// -----------------------------------------------------------------------------

// 8x8
HADAMARD_TEST(C, TestHadamard8x8_All, vpx_hadamard_8x8_c);
// 16x16
HADAMARD_TEST(C, TestHadamard16x16_All, vpx_hadamard_16x16_c);

#if HAVE_VSX
// 8x8
HADAMARD_TEST(VSX, TestHadamard8x8_All, vpx_hadamard_8x8_vsx);
// 16x16
HADAMARD_TEST(VSX, TestHadamard16x16_All, vpx_hadamard_16x16_vsx);
#endif  // HAVE_VSX

#include "test/test_libvpx.cc"
