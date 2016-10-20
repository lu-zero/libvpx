/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <asm/cputable.h>
#include <linux/auxvec.h>
#endif

#include "./vpx_config.h"
#include "vpx_ports/ppc.h"

int ppc_simd_caps(void) {
  int result = 0;
#if CONFIG_RUNTIME_CPU_DETECT
#if defined(__linux__)
  int fd;
  ssize_t count;
  unsigned int i;
  uint64_t buf[64] = {0};

  fd = open("/proc/self/auxv", O_RDONLY);
  if (fd < 0) {
    return 0;
  }

  while ((count = read(fd, buf, sizeof(buf))) > 0) {
    for (i = 0; i < (count / sizeof(*buf)); i += 2) {
      if (buf[i] == AT_HWCAP) {
#if HAVE_VSX
        if (buf[i + 1] & PPC_FEATURE_HAS_VSX) {
          result |= HAS_VSX;
        }
#endif  // HAVE_VSX
        goto out_close;
      } else if (buf[i] == AT_NULL) {
        goto out_close;
      }
    }
  }
out_close:
  close(fd);
#endif  // __linux__
#endif  // CONFIG_RUNTIME_CPU_DETECT
  return result;
}
