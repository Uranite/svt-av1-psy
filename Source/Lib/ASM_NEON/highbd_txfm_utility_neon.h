/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at https://www.aomedia.org/license/software-license. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at https://www.aomedia.org/license/patent-license.
 */

#ifndef _HIGHBD_TXFM_UTILITY_NEON_H
#define _HIGHBD_TXFM_UTILITY_NEON_H

#include <arm_neon.h>

static INLINE int32x4_t half_btf_neon(const int32x4_t *w0, const int32x4_t *n0, const int32x4_t *w1,
                                      const int32x4_t *n1, int32_t bit) {
    int32x4_t x, y;
    x = vmulq_s32(*w0, *n0);
    y = vmulq_s32(*w1, *n1);
    x = vaddq_s32(x, y);
    x = vrshlq_s32(x, vdupq_n_s32(-bit));
    return x;
}

static INLINE int32x4_t half_btf_0_neon(const int32x4_t *w0, const int32x4_t *n0, int32_t bit) {
    int32x4_t x;
    x = vmulq_s32(*w0, *n0);
    x = vrshlq_s32(x, vdupq_n_s32(-bit));
    return x;
}

#endif // _HIGHBD_TXFM_UTILITY_NEON_H
