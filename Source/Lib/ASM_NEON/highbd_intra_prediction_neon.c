/*
 * Copyright (c) 2023, Alliance for Open Media. All rights reserved.
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include <arm_neon.h>
#include <assert.h>

#include "definitions.h"
#include "common_dsp_rtcd.h"

void svt_av1_filter_intra_edge_high_neon(uint16_t *p, int sz, int strength) {
    if (!strength)
        return;
    assert(sz >= 0 && sz <= 129);

    DECLARE_ALIGNED(16, static const uint16_t, idx[8]) = {0, 1, 2, 3, 4, 5, 6, 7};
    const uint16x8_t index                             = vld1q_u16(idx);

    uint16_t edge[160]; // Max value of sz + enough padding for vector accesses.
    memcpy(edge + 1, p, sz * sizeof(*p));

    // Populate extra space appropriately.
    edge[0]      = edge[1];
    edge[sz + 1] = edge[sz];
    edge[sz + 2] = edge[sz];

    // Don't overwrite first pixel.
    uint16_t *dst = p + 1;
    sz--;

    if (strength == 1) { // Filter: {4, 8, 4}.
        const uint16_t *src = edge + 1;

        while (sz >= 8) {
            uint16x8_t s0 = vld1q_u16(src);
            uint16x8_t s1 = vld1q_u16(src + 1);
            uint16x8_t s2 = vld1q_u16(src + 2);

            // Make use of the identity:
            // (4*a + 8*b + 4*c) >> 4 == (a + (b << 1) + c) >> 2
            uint16x8_t t0  = vaddq_u16(s0, s2);
            uint16x8_t t1  = vaddq_u16(s1, s1);
            uint16x8_t sum = vaddq_u16(t0, t1);
            uint16x8_t res = vrshrq_n_u16(sum, 2);

            vst1q_u16(dst, res);

            src += 8;
            dst += 8;
            sz -= 8;
        }

        if (sz > 0) { // Handle sz < 8 to avoid modifying out-of-bounds values.
            uint16x8_t s0 = vld1q_u16(src);
            uint16x8_t s1 = vld1q_u16(src + 1);
            uint16x8_t s2 = vld1q_u16(src + 2);

            // Make use of the identity:
            // (4*a + 8*b + 4*c) >> 4 == (a + (b << 1) + c) >> 2
            uint16x8_t t0  = vaddq_u16(s0, s2);
            uint16x8_t t1  = vaddq_u16(s1, s1);
            uint16x8_t sum = vaddq_u16(t0, t1);
            uint16x8_t res = vrshrq_n_u16(sum, 2);

            // Mask off out-of-bounds indices.
            uint16x8_t current_dst = vld1q_u16(dst);
            uint16x8_t mask        = vcgtq_u16(vdupq_n_u16(sz), index);
            res                    = vbslq_u16(mask, res, current_dst);

            vst1q_u16(dst, res);
        }
    } else if (strength == 2) { // Filter: {5, 6, 5}.
        const uint16_t *src = edge + 1;

        const uint16x8x3_t filter = {{vdupq_n_u16(5), vdupq_n_u16(6), vdupq_n_u16(5)}};
        while (sz >= 8) {
            uint16x8_t s0 = vld1q_u16(src);
            uint16x8_t s1 = vld1q_u16(src + 1);
            uint16x8_t s2 = vld1q_u16(src + 2);

            uint16x8_t accum = vmulq_u16(s0, filter.val[0]);
            accum            = vmlaq_u16(accum, s1, filter.val[1]);
            accum            = vmlaq_u16(accum, s2, filter.val[2]);
            uint16x8_t res   = vrshrq_n_u16(accum, 4);

            vst1q_u16(dst, res);

            src += 8;
            dst += 8;
            sz -= 8;
        }

        if (sz > 0) { // Handle sz < 8 to avoid modifying out-of-bounds values.
            uint16x8_t s0 = vld1q_u16(src);
            uint16x8_t s1 = vld1q_u16(src + 1);
            uint16x8_t s2 = vld1q_u16(src + 2);

            uint16x8_t accum = vmulq_u16(s0, filter.val[0]);
            accum            = vmlaq_u16(accum, s1, filter.val[1]);
            accum            = vmlaq_u16(accum, s2, filter.val[2]);
            uint16x8_t res   = vrshrq_n_u16(accum, 4);

            // Mask off out-of-bounds indices.
            uint16x8_t current_dst = vld1q_u16(dst);
            uint16x8_t mask        = vcgtq_u16(vdupq_n_u16(sz), index);
            res                    = vbslq_u16(mask, res, current_dst);

            vst1q_u16(dst, res);
        }
    } else { // Filter {2, 4, 4, 4, 2}.
        const uint16_t *src = edge;

        while (sz >= 8) {
            uint16x8_t s0 = vld1q_u16(src);
            uint16x8_t s1 = vld1q_u16(src + 1);
            uint16x8_t s2 = vld1q_u16(src + 2);
            uint16x8_t s3 = vld1q_u16(src + 3);
            uint16x8_t s4 = vld1q_u16(src + 4);

            // Make use of the identity:
            // (2*a + 4*b + 4*c + 4*d + 2*e) >> 4 == (a + ((b + c + d) << 1) + e) >> 3
            uint16x8_t t0  = vaddq_u16(s0, s4);
            uint16x8_t t1  = vaddq_u16(s1, s2);
            t1             = vaddq_u16(t1, s3);
            t1             = vaddq_u16(t1, t1);
            uint16x8_t sum = vaddq_u16(t0, t1);
            uint16x8_t res = vrshrq_n_u16(sum, 3);

            vst1q_u16(dst, res);

            src += 8;
            dst += 8;
            sz -= 8;
        }

        if (sz > 0) { // Handle sz < 8 to avoid modifying out-of-bounds values.
            uint16x8_t s0 = vld1q_u16(src);
            uint16x8_t s1 = vld1q_u16(src + 1);
            uint16x8_t s2 = vld1q_u16(src + 2);
            uint16x8_t s3 = vld1q_u16(src + 3);
            uint16x8_t s4 = vld1q_u16(src + 4);

            // Make use of the identity:
            // (2*a + 4*b + 4*c + 4*d + 2*e) >> 4 == (a + ((b + c + d) << 1) + e) >> 3
            uint16x8_t t0  = vaddq_u16(s0, s4);
            uint16x8_t t1  = vaddq_u16(s1, s2);
            t1             = vaddq_u16(t1, s3);
            t1             = vaddq_u16(t1, t1);
            uint16x8_t sum = vaddq_u16(t0, t1);
            uint16x8_t res = vrshrq_n_u16(sum, 3);

            // Mask off out-of-bounds indices.
            uint16x8_t current_dst = vld1q_u16(dst);
            uint16x8_t mask        = vcgtq_u16(vdupq_n_u16(sz), index);
            res                    = vbslq_u16(mask, res, current_dst);

            vst1q_u16(dst, res);
        }
    }
}
