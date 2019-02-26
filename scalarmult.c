/*
    This code follows the SUPERCOP crypto_scalarmult convention.

    It implements exception free scalar multiplication over the curve
    `E : y^2 = x^3 - 3*x + 13318`.
*/

#include "ge.h"
#include "mxcsr.h"
#include <stdbool.h>
#include <stdint.h>

#define scalarmult crypto_scalarmult_curve13318_scalarmult
#define select crypto_scalarmult_curve13318_ref12_select
#define ladder crypto_scalarmult_curve13318_ref12_ladder

void crypto_scalarmult_curve13318_ref12_ladder(ge q, const uint8_t *w, const ge ptable[16]);

// Conditionally add an element, assumes dest == {0}
static void cmov(ge dest, const ge src, uint64_t mask)
{
    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 12; j++) {
            union limb {
                double d;
                uint64_t u64;
            };
            union limb __attribute__((aligned(64))) tmp1 = { .d = src[i][j] };
            union limb __attribute__((aligned(64))) tmp2 = { .d = dest[i][j] };
            tmp2.u64 |= tmp1.u64 & mask;
            dest[i][j] = tmp2.d;
        }
    }
}

// Conditionally move the neutral element, assumes dest == {0}
static void cmov_neutral(ge dest, uint64_t mask)
{
    union limb {
        double d;
        uint64_t u64;
    };
    union limb __attribute__((aligned(64))) tmp1 = { .d = 1.0 };
    union limb __attribute__((aligned(64))) tmp2 = { .d = dest[1][0] };
    tmp2.u64 |= tmp1.u64 & mask;
    dest[1][0] = tmp2.d;
}

// Do the table precomputation
static void do_precomputation(ge ptable[16], const ge p)
{
    ge_copy(ptable[0], p);
    ge_double(ptable[1], ptable[0]);
    ge_add(ptable[2], ptable[1], ptable[0]);
    ge_double(ptable[3], ptable[1]);
    ge_add(ptable[4], ptable[3], ptable[0]);
    ge_double(ptable[5], ptable[2]);
    ge_add(ptable[6], ptable[5], ptable[0]);
    ge_double(ptable[7], ptable[3]);
    ge_add(ptable[8], ptable[7], ptable[0]);
    ge_double(ptable[9], ptable[4]);
    ge_add(ptable[10], ptable[9], ptable[0]);
    ge_double(ptable[11], ptable[5]);
    ge_add(ptable[12], ptable[11], ptable[0]);
    ge_double(ptable[13], ptable[6]);
    ge_add(ptable[14], ptable[13], ptable[0]);
    ge_double(ptable[15], ptable[7]);
}

// Decode the key bytes into windows and ripple the subtraction carry
static inline void compute_windows(uint8_t w[51], uint8_t *zeroth_window, const uint8_t *e)
{
    w[50] = e[ 0] & 0x1F;
    w[49] = ((e[ 1] << 3) | (e[ 0] >> 5)) & 0x1F;
    w[49] += ((w[50] >> 5) ^ (w[50] >> 4)) & 0x1;
    w[48] = (e[ 1] >> 2) & 0x1F;
    w[48] += ((w[49] >> 5) ^ (w[49] >> 4)) & 0x1;
    w[47] = ((e[ 2] << 1) | (e[ 1] >> 7)) & 0x1F;
    w[47] += ((w[48] >> 5) ^ (w[48] >> 4)) & 0x1;
    w[46] = ((e[ 3] << 4) | (e[ 2] >> 4)) & 0x1F;
    w[46] += ((w[47] >> 5) ^ (w[47] >> 4)) & 0x1;
    w[45] = (e[ 3] >> 1) & 0x1F;
    w[45] += ((w[46] >> 5) ^ (w[46] >> 4)) & 0x1;
    w[44] = ((e[ 4] << 2) | (e[ 3] >> 6)) & 0x1F;
    w[44] += ((w[45] >> 5) ^ (w[45] >> 4)) & 0x1;
    w[43] = (e[ 4] >> 3) & 0x1F;
    w[43] += ((w[44] >> 5) ^ (w[44] >> 4)) & 0x1;
    w[42] = e[ 5] & 0x1F;
    w[42] += ((w[43] >> 5) ^ (w[43] >> 4)) & 0x1;
    w[41] = ((e[ 6] << 3) | (e[ 5] >> 5)) & 0x1F;
    w[41] += ((w[42] >> 5) ^ (w[42] >> 4)) & 0x1;
    w[40] = (e[ 6] >> 2) & 0x1F;
    w[40] += ((w[41] >> 5) ^ (w[41] >> 4)) & 0x1;
    w[39] = ((e[ 7] << 1) | (e[ 6] >> 7)) & 0x1F;
    w[39] += ((w[40] >> 5) ^ (w[40] >> 4)) & 0x1;
    w[38] = ((e[ 8] << 4) | (e[ 7] >> 4)) & 0x1F;
    w[38] += ((w[39] >> 5) ^ (w[39] >> 4)) & 0x1;
    w[37] = (e[ 8] >> 1) & 0x1F;
    w[37] += ((w[38] >> 5) ^ (w[38] >> 4)) & 0x1;
    w[36] = ((e[ 9] << 2) | (e[ 8] >> 6)) & 0x1F;
    w[36] += ((w[37] >> 5) ^ (w[37] >> 4)) & 0x1;
    w[35] = (e[ 9] >> 3) & 0x1F;
    w[35] += ((w[36] >> 5) ^ (w[36] >> 4)) & 0x1;
    w[34] = e[10] & 0x1F;
    w[34] += ((w[35] >> 5) ^ (w[35] >> 4)) & 0x1;
    w[33] = ((e[11] << 3) | (e[10] >> 5)) & 0x1F;
    w[33] += ((w[34] >> 5) ^ (w[34] >> 4)) & 0x1;
    w[32] = (e[11] >> 2) & 0x1F;
    w[32] += ((w[33] >> 5) ^ (w[33] >> 4)) & 0x1;
    w[31] = ((e[12] << 1) | (e[11] >> 7)) & 0x1F;
    w[31] += ((w[32] >> 5) ^ (w[32] >> 4)) & 0x1;
    w[30] = ((e[13] << 4) | (e[12] >> 4)) & 0x1F;
    w[30] += ((w[31] >> 5) ^ (w[31] >> 4)) & 0x1;
    w[29] = (e[13] >> 1) & 0x1F;
    w[29] += ((w[30] >> 5) ^ (w[30] >> 4)) & 0x1;
    w[28] = ((e[14] << 2) | (e[13] >> 6)) & 0x1F;
    w[28] += ((w[29] >> 5) ^ (w[29] >> 4)) & 0x1;
    w[27] = (e[14] >> 3) & 0x1F;
    w[27] += ((w[28] >> 5) ^ (w[28] >> 4)) & 0x1;
    w[26] = e[15] & 0x1F;
    w[26] += ((w[27] >> 5) ^ (w[27] >> 4)) & 0x1;
    w[25] = ((e[16] << 3) | (e[15] >> 5)) & 0x1F;
    w[25] += ((w[26] >> 5) ^ (w[26] >> 4)) & 0x1;
    w[24] = (e[16] >> 2) & 0x1F;
    w[24] += ((w[25] >> 5) ^ (w[25] >> 4)) & 0x1;
    w[23] = ((e[17] << 1) | (e[16] >> 7)) & 0x1F;
    w[23] += ((w[24] >> 5) ^ (w[24] >> 4)) & 0x1;
    w[22] = ((e[18] << 4) | (e[17] >> 4)) & 0x1F;
    w[22] += ((w[23] >> 5) ^ (w[23] >> 4)) & 0x1;
    w[21] = (e[18] >> 1) & 0x1F;
    w[21] += ((w[22] >> 5) ^ (w[22] >> 4)) & 0x1;
    w[20] = ((e[19] << 2) | (e[18] >> 6)) & 0x1F;
    w[20] += ((w[21] >> 5) ^ (w[21] >> 4)) & 0x1;
    w[19] = (e[19] >> 3) & 0x1F;
    w[19] += ((w[20] >> 5) ^ (w[20] >> 4)) & 0x1;
    w[18] = e[20] & 0x1F;
    w[18] += ((w[19] >> 5) ^ (w[19] >> 4)) & 0x1;
    w[17] = ((e[21] << 3) | (e[20] >> 5)) & 0x1F;
    w[17] += ((w[18] >> 5) ^ (w[18] >> 4)) & 0x1;
    w[16] = (e[21] >> 2) & 0x1F;
    w[16] += ((w[17] >> 5) ^ (w[17] >> 4)) & 0x1;
    w[15] = ((e[22] << 1) | (e[21] >> 7)) & 0x1F;
    w[15] += ((w[16] >> 5) ^ (w[16] >> 4)) & 0x1;
    w[14] = ((e[23] << 4) | (e[22] >> 4)) & 0x1F;
    w[14] += ((w[15] >> 5) ^ (w[15] >> 4)) & 0x1;
    w[13] = (e[23] >> 1) & 0x1F;
    w[13] += ((w[14] >> 5) ^ (w[14] >> 4)) & 0x1;
    w[12] = ((e[24] << 2) | (e[23] >> 6)) & 0x1F;
    w[12] += ((w[13] >> 5) ^ (w[13] >> 4)) & 0x1;
    w[11] = (e[24] >> 3) & 0x1F;
    w[11] += ((w[12] >> 5) ^ (w[12] >> 4)) & 0x1;
    w[10] = e[25] & 0x1F;
    w[10] += ((w[11] >> 5) ^ (w[11] >> 4)) & 0x1;
    w[ 9] = ((e[26] << 3) | (e[25] >> 5)) & 0x1F;
    w[ 9] += ((w[10] >> 5) ^ (w[10] >> 4)) & 0x1;
    w[ 8] = (e[26] >> 2) & 0x1F;
    w[ 8] += ((w[ 9] >> 5) ^ (w[ 9] >> 4)) & 0x1;
    w[ 7] = ((e[27] << 1) | (e[26] >> 7)) & 0x1F;
    w[ 7] += ((w[ 8] >> 5) ^ (w[ 8] >> 4)) & 0x1;
    w[ 6] = ((e[28] << 4) | (e[27] >> 4)) & 0x1F;
    w[ 6] += ((w[ 7] >> 5) ^ (w[ 7] >> 4)) & 0x1;
    w[ 5] = (e[28] >> 1) & 0x1F;
    w[ 5] += ((w[ 6] >> 5) ^ (w[ 6] >> 4)) & 0x1;
    w[ 4] = ((e[29] << 2) | (e[28] >> 6)) & 0x1F;
    w[ 4] += ((w[ 5] >> 5) ^ (w[ 5] >> 4)) & 0x1;
    w[ 3] = (e[29] >> 3) & 0x1F;
    w[ 3] += ((w[ 4] >> 5) ^ (w[ 4] >> 4)) & 0x1;
    w[ 2] = e[30] & 0x1F;
    w[ 2] += ((w[ 3] >> 5) ^ (w[ 3] >> 4)) & 0x1;
    w[ 1] = ((e[31] << 3) | (e[30] >> 5)) & 0x1F;
    w[ 1] += ((w[ 2] >> 5) ^ (w[ 2] >> 4)) & 0x1;
    w[ 0] = (e[31] >> 2) & 0x1F;
    w[ 0] += ((w[ 1] >> 5) ^ (w[ 1] >> 4)) & 0x1;
    *zeroth_window = ((w[0] >> 5) ^ (w[0] >> 4)) & 0x1;
}

// Main secret scalar multiplication
int scalarmult(uint8_t *out, const uint8_t *key, const uint8_t *in)
{
    ge __attribute__((aligned(64))) p, __attribute__((aligned(64))) q;
    ge __attribute__((aligned(64))) ptable[16];
    uint8_t e[32];
    uint8_t w[51], zeroth_window;

    // Prologue: save the MxCsr register state
    const unsigned int saved_mxcsr = replace_mxcsr();

    for (unsigned int i = 0; i < 32; i++) e[i] = key[i];
    e[31] &= 0x7F; // We do not use the 255'th bit from the key

    int err = ge_frombytes(p, in);
    if (err != 0) {
        restore_mxcsr(saved_mxcsr);
        return -1;
    }

    // Prepare for ladder computation
    do_precomputation(ptable, p);
    compute_windows(w, &zeroth_window, key);

    // Do double and add scalar multiplication
    ge_zero(q);
    cmov_neutral(q, -(int64_t)(zeroth_window == 0));
    cmov(q, ptable[0], -(int64_t)(zeroth_window == 1));
    crypto_scalarmult_curve13318_ref12_ladder(q, w, ptable);
    ge_tobytes(out, q);

    // Epilogue: restore the MxCsr register to its original value
    const bool mxcsr_ok = restore_mxcsr(saved_mxcsr);
    if (!mxcsr_ok) return -1;

    return 0;
}
