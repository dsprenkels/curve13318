#include "fe12.h"
#include <stdint.h>
#include <xmmintrin.h>

static inline uint32_t load_3(const uint8_t *in)
{
    uint32_t ret;
    ret = (uint32_t) in[0];
    ret |= ((uint32_t) in[1]) << 8;
    ret |= ((uint32_t) in[2]) << 16;
    return ret;
}

static inline uint32_t load_2(const uint8_t *in)
{
    uint32_t ret;
    ret = (uint32_t) in[0];
    ret |= ((uint32_t) in[1]) << 8;
    return ret;
}

void fe12_frombytes(fe12 z, const uint8_t *in)
{
    uint32_t z0  = load_3(in);
    uint32_t z1  = load_3(in +  3) << 2;
    uint32_t z2  = load_2(in +  6) << 5;
    uint32_t z3  = load_3(in +  8);
    uint32_t z4  = load_3(in + 11) << 3;
    uint32_t z5  = load_2(in + 14) << 5;
    uint32_t z6  = load_3(in + 16);
    uint32_t z7  = load_3(in + 19) << 3;
    uint32_t z8  = load_2(in + 22) << 6;
    uint32_t z9  = load_3(in + 24);
    uint32_t z10 = load_3(in + 27) << 3;
    uint32_t z11 = load_2(in + 30) << 6;

    uint32_t carry11 = z11 >> 21;  z0 += 19 * carry11; z11 &= 0x1FFFFF;
    uint32_t  carry1 =  z1 >> 21;  z2 += carry1; z1 &= 0x1FFFFF;
    uint32_t  carry3 =  z3 >> 21;  z4 += carry3; z3 &= 0x1FFFFF;
    uint32_t  carry5 =  z5 >> 21;  z6 += carry5; z5 &= 0x1FFFFF;
    uint32_t  carry7 =  z7 >> 21;  z8 += carry7; z7 &= 0x1FFFFF;
    uint32_t  carry9 =  z9 >> 21; z10 += carry9; z9 &= 0x1FFFFF;

    uint32_t  carry0 =  z0 >> 22;  z1 += carry0; z0 &= 0x3FFFFF;
    uint32_t  carry2 =  z2 >> 21;  z2 += carry2; z2 &= 0x1FFFFF;
    uint32_t  carry4 =  z4 >> 22;  z5 += carry4; z4 &= 0x3FFFFF;
    uint32_t  carry6 =  z6 >> 21;  z7 += carry6; z6 &= 0x1FFFFF;
    uint32_t  carry8 =  z8 >> 22;  z9 += carry8; z8 &= 0x3FFFFF;
    uint32_t carry10 = z10 >> 21; z11 += carry10; z10 &= 0x1FFFFF;

    z[0] =  (double)z0;
    z[1] =  (double)z1 * 0x1p22;
    z[2] =  (double)z2 * 0x1p43;
    z[3] =  (double)z3 * 0x1p64;
    z[4] =  (double)z4 * 0x1p85;
    z[5] =  (double)z5 * 0x1p107;
    z[6] =  (double)z6 * 0x1p128;
    z[7] =  (double)z7 * 0x1p149;
    z[8] =  (double)z8 * 0x1p170;
    z[9] =  (double)z9 * 0x1p192;
    z[10] = (double)z10 * 0x1p213;
    z[11] = (double)z11 * 0x1p234;
}

void fe12_squeeze(fe12 z)
{
    // Interleave two carry chains (8 rounds):
    //   - a: z[0] -> z[1] -> z[2] -> z[3] -> z[4]  -> z[5]  -> z[6] -> z[7]
    //   - b: z[6] -> z[7] -> z[8] -> z[9] -> z[10] -> z[11] -> z[0] -> z[1]
    //
    // Precondition:
    //   - For all limbs x in z : |x| <= 0.99 * 2^53
    //
    // Postcondition:
    //   - All significands fit in b + 1 bits (b = 22, 21, 21, etc.)

    double t0, t1;
    t0 = z[0] + 0x3p73 - 0x3p73; // Round 1a
    z[0] -= t0;
    z[1] += t0;
    t1 = z[6] + 0x3p200 - 0x3p200; // Round 1b
    z[6] -= t1;
    z[7] += t1;
    t0 = z[1] + 0x3p94 - 0x3p94; // Round 2a
    z[1] -= t0;
    z[2] += t0;
    t1 = z[7] + 0x3p221 - 0x3p221; // Round 2b
    z[7] -= t1;
    z[8] += t1;
    t0 = z[2] + 0x3p115 - 0x3p115; // Round 3a
    z[2] -= t0;
    z[3] += t0;
    t1 = z[8] + 0x3p243 - 0x3p243; // Round 3b
    z[8] -= t1;
    z[9] += t1;
    t0 = z[3] + 0x3p136 - 0x3p136; // Round 4a
    z[3] -= t0;
    z[4] += t0;
    t1 = z[9] + 0x3p264 - 0x3p264; // Round 4b
    z[9] -= t1;
    z[10] += t1;
    t0 = z[4] + 0x3p158 - 0x3p158; // Round 5a
    z[4] -= t0;
    z[5] += t0;
    t1 = z[10] + 0x3p285 - 0x3p285; // Round 5b
    z[10] -= t1;
    z[11] += t1;
    t0 = z[5] + 0x3p179 - 0x3p179; // Round 6a
    z[5] -= t0;
    z[6] += t0;
    t1 = z[11] + 0x3p306 - 0x3p306; // Round 6b
    z[11] -= t1;
    z[0] += 0x13p-255 * t1; // 19 * 2^-255
    t0 = z[6] + 0x3p200 - 0x3p200; // Round 7a
    z[6] -= t0;
    z[7] += t0;
    t1 = z[0] + 0x3p73 - 0x3p73; // Round 7b
    z[0] -= t1;
    z[1] += t1;
    t0 = z[7] + 0x3p221 - 0x3p221; // Round 8a
    z[7] -= t0;
    z[8] += t0;
    t1 = z[1] + 0x3p94 - 0x3p94; // Round 8b
    z[1] -= t1;
    z[2] += t1;
}

static inline double unset_bit59(const double x)
{
    union {
        double f64;
        uint64_t u64;
    } tmp = { .f64 = x };
    tmp.u64 &= 0xF7FFFFFFFFFFFFFF;
    return tmp.f64;
}

void fe12_mul(fe12 C, const fe12 A, const fe12 B)
{
    double l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    double h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10;
    double m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10;

    // Compute L
    l0  = A[0] * B[0]; // Round 1/6
    l1  = A[0] * B[1];
    l2  = A[0] * B[2];
    l3  = A[0] * B[3];
    l4  = A[0] * B[4];
    l5  = A[0] * B[5];
    l1 += A[1] * B[0]; // Round 2/6
    l2 += A[1] * B[1];
    l3 += A[1] * B[2];
    l4 += A[1] * B[3];
    l5 += A[1] * B[4];
    l6  = A[1] * B[5];
    l2 += A[2] * B[0]; // Round 3/6
    l3 += A[2] * B[1];
    l4 += A[2] * B[2];
    l5 += A[2] * B[3];
    l6 += A[2] * B[4];
    l7  = A[2] * B[5];
    l3 += A[3] * B[0]; // Round 4/6
    l4 += A[3] * B[1];
    l5 += A[3] * B[2];
    l6 += A[3] * B[3];
    l7 += A[3] * B[4];
    l8  = A[3] * B[5];
    l4 += A[4] * B[0]; // Round 5/6
    l5 += A[4] * B[1];
    l6 += A[4] * B[2];
    l7 += A[4] * B[3];
    l8 += A[4] * B[4];
    l9  = A[4] * B[5];
    l5 += A[5] * B[0]; // Round 6/6
    l6 += A[5] * B[1];
    l7 += A[5] * B[2];
    l8 += A[5] * B[3];
    l9 += A[5] * B[4];
    l10 = A[5] * B[5];

    // Precompute reduced A,B values
    // For x7-x10, we know for sure that they are:
    //   - either 0 -> exponent of double is 0b00000000000
    //   - 0 modulo 2^149 and smaller than 2^237 and as such, the 7'th exponent bit is always set.
    //     Thus we can use a mask operation for these values to divide them by 2*128.
    const double  A6_shr = 0x1p-128 * A[ 6];
    const double  A7_shr = unset_bit59(A[7]);
    const double  A8_shr = unset_bit59(A[8]);
    const double  A9_shr = unset_bit59(A[9]);
    const double A10_shr = unset_bit59(A[10]);
    const double A11_shr = 0x1p-128 * A[11];
    const double  B6_shr = 0x1p-128 * B[ 6];
    const double  B7_shr = unset_bit59(B[7]);
    const double  B8_shr = unset_bit59(B[8]);
    const double  B9_shr = unset_bit59(B[9]);
    const double B10_shr = unset_bit59(B[10]);
    const double B11_shr = 0x1p-128 * B[11];

    // Compute H
    h0  =  A6_shr *  B6_shr; // Round 1/6
    h1  =  A6_shr *  B7_shr;
    h2  =  A6_shr *  B8_shr;
    h3  =  A6_shr *  B9_shr;
    h4  =  A6_shr * B10_shr;
    h5  =  A6_shr * B11_shr;
    h1 +=  A7_shr *  B6_shr; // Round 2/6
    h2 +=  A7_shr *  B7_shr;
    h3 +=  A7_shr *  B8_shr;
    h4 +=  A7_shr *  B9_shr;
    h5 +=  A7_shr * B10_shr;
    h6  =  A7_shr * B11_shr;
    h2 +=  A8_shr *  B6_shr; // Round 3/6
    h3 +=  A8_shr *  B7_shr;
    h4 +=  A8_shr *  B8_shr;
    h5 +=  A8_shr *  B9_shr;
    h6 +=  A8_shr * B10_shr;
    h7  =  A8_shr * B11_shr;
    h3 +=  A9_shr *  B6_shr; // Round 4/6
    h4 +=  A9_shr *  B7_shr;
    h5 +=  A9_shr *  B8_shr;
    h6 +=  A9_shr *  B9_shr;
    h7 +=  A9_shr * B10_shr;
    h8  =  A9_shr * B11_shr;
    h4 += A10_shr *  B6_shr; // Round 5/6
    h5 += A10_shr *  B7_shr;
    h6 += A10_shr *  B8_shr;
    h7 += A10_shr *  B9_shr;
    h8 += A10_shr * B10_shr;
    h9  = A10_shr * B11_shr;
    h5 += A11_shr *  B6_shr; // Round 6/6
    h6 += A11_shr *  B7_shr;
    h7 += A11_shr *  B8_shr;
    h8 += A11_shr *  B9_shr;
    h9 += A11_shr * B10_shr;
    h10 = A11_shr * B11_shr;

    // Compute M_hat
    const double mA0 = A[0] -  A6_shr;
    const double mA1 = A[1] -  A7_shr;
    const double mA2 = A[2] -  A8_shr;
    const double mA3 = A[3] -  A9_shr;
    const double mA4 = A[4] - A10_shr;
    const double mA5 = A[5] - A11_shr;
    const double mB0 =  B6_shr - B[0];
    const double mB1 =  B7_shr - B[1];
    const double mB2 =  B8_shr - B[2];
    const double mB3 =  B9_shr - B[3];
    const double mB4 = B10_shr - B[4];
    const double mB5 = B11_shr - B[5];
    m0  = mA0 * mB0; // Round 1/6
    m1  = mA0 * mB1;
    m2  = mA0 * mB2;
    m3  = mA0 * mB3;
    m4  = mA0 * mB4;
    m5  = mA0 * mB5;
    m1 += mA1 * mB0; // Round 2/6
    m2 += mA1 * mB1;
    m3 += mA1 * mB2;
    m4 += mA1 * mB3;
    m5 += mA1 * mB4;
    m6  = mA1 * mB5;
    m2 += mA2 * mB0; // Round 3/6
    m3 += mA2 * mB1;
    m4 += mA2 * mB2;
    m5 += mA2 * mB3;
    m6 += mA2 * mB4;
    m7  = mA2 * mB5;
    m3 += mA3 * mB0; // Round 4/6
    m4 += mA3 * mB1;
    m5 += mA3 * mB2;
    m6 += mA3 * mB3;
    m7 += mA3 * mB4;
    m8  = mA3 * mB5;
    m4 += mA4 * mB0; // Round 5/6
    m5 += mA4 * mB1;
    m6 += mA4 * mB2;
    m7 += mA4 * mB3;
    m8 += mA4 * mB4;
    m9  = mA4 * mB5;
    m5 += mA5 * mB0; // Round 6/6
    m6 += mA5 * mB1;
    m7 += mA5 * mB2;
    m8 += mA5 * mB3;
    m9 += mA5 * mB4;
    m10 = mA5 * mB5;

    // Sum up the accs into C
    C[ 0] =  l0 + 0x26 * (0x1p-128 * ( m6 +  l6 +  h6) + h0);
    C[ 1] =  l1 + 0x26 * (0x1p-128 * ( m7 +  l7 +  h7) + h1);
    C[ 2] =  l2 + 0x26 * (0x1p-128 * ( m8 +  l8 +  h8) + h2);
    C[ 3] =  l3 + 0x26 * (0x1p-128 * ( m9 +  l9 +  h9) + h3);
    C[ 4] =  l4 + 0x26 * (0x1p-128 * (m10 + l10 + h10) + h4);
    C[ 5] =  l5 + 0x26 *                                 h5 ;
    C[ 6] =  l6 + 0x1p+128 * ( m0 + l0 + h0) + 0x26*h6;
    C[ 7] =  l7 + 0x1p+128 * ( m1 + l1 + h1) + 0x26*h7;
    C[ 8] =  l8 + 0x1p+128 * ( m2 + l2 + h2) + 0x26*h8;
    C[ 9] =  l9 + 0x1p+128 * ( m3 + l3 + h3) + 0x26*h9;
    C[10] = l10 + 0x1p+128 * ( m4 + l4 + h4) + 0x26*h10;
    C[11] =       0x1p+128 * ( m5 + l5 + h5);
}

void fe12_mul_schoolbook(fe12 dest, const fe12 A, const fe12 B)
{
    fe12 C;

    // Precompute reduced B values
    const double  B1_19 = 0x13p-255 * B[ 1];
    const double  B2_19 = 0x13p-255 * B[ 2];
    const double  B3_19 = 0x13p-255 * B[ 3];
    const double  B4_19 = 0x13p-255 * B[ 4];
    const double  B5_19 = 0x13p-255 * B[ 5];
    const double  B6_19 = 0x13p-255 * B[ 6];
    const double  B7_19 = 0x13p-255 * B[ 7];
    const double  B8_19 = 0x13p-255 * B[ 8];
    const double  B9_19 = 0x13p-255 * B[ 9];
    const double B10_19 = 0x13p-255 * B[10];
    const double B11_19 = 0x13p-255 * B[11];

    // Round  1/12
    C[ 0] = A[0] * B[ 0];
    C[ 1] = A[0] * B[ 1];
    C[ 2] = A[0] * B[ 2];
    C[ 3] = A[0] * B[ 3];
    C[ 4] = A[0] * B[ 4];
    C[ 5] = A[0] * B[ 5];
    C[ 6] = A[0] * B[ 6];
    C[ 7] = A[0] * B[ 7];
    C[ 8] = A[0] * B[ 8];
    C[ 9] = A[0] * B[ 9];
    C[10] = A[0] * B[10];
    C[11] = A[0] * B[11];

    // Round  2/12
    C[ 0] += A[ 1] * B11_19;
    C[ 1] += A[ 1] * B[ 0];
    C[ 2] += A[ 1] * B[ 1];
    C[ 3] += A[ 1] * B[ 2];
    C[ 4] += A[ 1] * B[ 3];
    C[ 5] += A[ 1] * B[ 4];
    C[ 6] += A[ 1] * B[ 5];
    C[ 7] += A[ 1] * B[ 6];
    C[ 8] += A[ 1] * B[ 7];
    C[ 9] += A[ 1] * B[ 8];
    C[10] += A[ 1] * B[ 9];
    C[11] += A[ 1] * B[10];

    // Round  3/12
    C[ 0] += A[ 2] * B10_19;
    C[ 1] += A[ 2] * B11_19;
    C[ 2] += A[ 2] * B[ 0];
    C[ 3] += A[ 2] * B[ 1];
    C[ 4] += A[ 2] * B[ 2];
    C[ 5] += A[ 2] * B[ 3];
    C[ 6] += A[ 2] * B[ 4];
    C[ 7] += A[ 2] * B[ 5];
    C[ 8] += A[ 2] * B[ 6];
    C[ 9] += A[ 2] * B[ 7];
    C[10] += A[ 2] * B[ 8];
    C[11] += A[ 2] * B[ 9];

    // Round  4/12
    C[ 0] += A[ 3] * B9_19;
    C[ 1] += A[ 3] * B10_19;
    C[ 2] += A[ 3] * B11_19;
    C[ 3] += A[ 3] * B[ 0];
    C[ 4] += A[ 3] * B[ 1];
    C[ 5] += A[ 3] * B[ 2];
    C[ 6] += A[ 3] * B[ 3];
    C[ 7] += A[ 3] * B[ 4];
    C[ 8] += A[ 3] * B[ 5];
    C[ 9] += A[ 3] * B[ 6];
    C[10] += A[ 3] * B[ 7];
    C[11] += A[ 3] * B[ 8];

    // Round  5/12
    C[ 0] += A[ 4] * B8_19;
    C[ 1] += A[ 4] * B9_19;
    C[ 2] += A[ 4] * B10_19;
    C[ 3] += A[ 4] * B11_19;
    C[ 4] += A[ 4] * B[ 0];
    C[ 5] += A[ 4] * B[ 1];
    C[ 6] += A[ 4] * B[ 2];
    C[ 7] += A[ 4] * B[ 3];
    C[ 8] += A[ 4] * B[ 4];
    C[ 9] += A[ 4] * B[ 5];
    C[10] += A[ 4] * B[ 6];
    C[11] += A[ 4] * B[ 7];

    // Round  6/12
    C[ 0] += A[ 5] * B7_19;
    C[ 1] += A[ 5] * B8_19;
    C[ 2] += A[ 5] * B9_19;
    C[ 3] += A[ 5] * B10_19;
    C[ 4] += A[ 5] * B11_19;
    C[ 5] += A[ 5] * B[ 0];
    C[ 6] += A[ 5] * B[ 1];
    C[ 7] += A[ 5] * B[ 2];
    C[ 8] += A[ 5] * B[ 3];
    C[ 9] += A[ 5] * B[ 4];
    C[10] += A[ 5] * B[ 5];
    C[11] += A[ 5] * B[ 6];

    // Round  7/12
    C[ 0] += A[ 6] * B6_19;
    C[ 1] += A[ 6] * B7_19;
    C[ 2] += A[ 6] * B8_19;
    C[ 3] += A[ 6] * B9_19;
    C[ 4] += A[ 6] * B10_19;
    C[ 5] += A[ 6] * B11_19;
    C[ 6] += A[ 6] * B[ 0];
    C[ 7] += A[ 6] * B[ 1];
    C[ 8] += A[ 6] * B[ 2];
    C[ 9] += A[ 6] * B[ 3];
    C[10] += A[ 6] * B[ 4];
    C[11] += A[ 6] * B[ 5];

    // Round  8/12
    C[ 0] += A[ 7] * B5_19;
    C[ 1] += A[ 7] * B6_19;
    C[ 2] += A[ 7] * B7_19;
    C[ 3] += A[ 7] * B8_19;
    C[ 4] += A[ 7] * B9_19;
    C[ 5] += A[ 7] * B10_19;
    C[ 6] += A[ 7] * B11_19;
    C[ 7] += A[ 7] * B[ 0];
    C[ 8] += A[ 7] * B[ 1];
    C[ 9] += A[ 7] * B[ 2];
    C[10] += A[ 7] * B[ 3];
    C[11] += A[ 7] * B[ 4];

    // Round  9/12
    C[ 0] += A[ 8] * B4_19;
    C[ 1] += A[ 8] * B5_19;
    C[ 2] += A[ 8] * B6_19;
    C[ 3] += A[ 8] * B7_19;
    C[ 4] += A[ 8] * B8_19;
    C[ 5] += A[ 8] * B9_19;
    C[ 6] += A[ 8] * B10_19;
    C[ 7] += A[ 8] * B11_19;
    C[ 8] += A[ 8] * B[ 0];
    C[ 9] += A[ 8] * B[ 1];
    C[10] += A[ 8] * B[ 2];
    C[11] += A[ 8] * B[ 3];

    // Round 10/12
    C[ 0] += A[ 9] * B3_19;
    C[ 1] += A[ 9] * B4_19;
    C[ 2] += A[ 9] * B5_19;
    C[ 3] += A[ 9] * B6_19;
    C[ 4] += A[ 9] * B7_19;
    C[ 5] += A[ 9] * B8_19;
    C[ 6] += A[ 9] * B9_19;
    C[ 7] += A[ 9] * B10_19;
    C[ 8] += A[ 9] * B11_19;
    C[ 9] += A[ 9] * B[ 0];
    C[10] += A[ 9] * B[ 1];
    C[11] += A[ 9] * B[ 2];

    // Round 11/12
    C[ 0] += A[10] * B2_19;
    C[ 1] += A[10] * B3_19;
    C[ 2] += A[10] * B4_19;
    C[ 3] += A[10] * B5_19;
    C[ 4] += A[10] * B6_19;
    C[ 5] += A[10] * B7_19;
    C[ 6] += A[10] * B8_19;
    C[ 7] += A[10] * B9_19;
    C[ 8] += A[10] * B10_19;
    C[ 9] += A[10] * B11_19;
    C[10] += A[10] * B[ 0];
    C[11] += A[10] * B[ 1];

    // Round 12/12
    C[ 0] += A[11] * B1_19;
    C[ 1] += A[11] * B2_19;
    C[ 2] += A[11] * B3_19;
    C[ 3] += A[11] * B4_19;
    C[ 4] += A[11] * B5_19;
    C[ 5] += A[11] * B6_19;
    C[ 6] += A[11] * B7_19;
    C[ 7] += A[11] * B8_19;
    C[ 8] += A[11] * B9_19;
    C[ 9] += A[11] * B10_19;
    C[10] += A[11] * B11_19;
    C[11] += A[11] * B[ 0];

    fe12_copy(dest, C);
}

void fe12_square(fe12 C, const fe12 A)
{
    double l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    double h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10;
    double m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10;

    // Precompute reduced A,B values
    const double  A6_shr = 0x1p-128 * A[ 6];
    const double  A7_shr = unset_bit59(A[ 7]);
    const double  A8_shr = unset_bit59(A[ 8]);
    const double  A9_shr = unset_bit59(A[ 9]);
    const double A10_shr = unset_bit59(A[10]);
    const double A11_shr = 0x1p-128 * A[11];

    // Compute L
    const double l00 = A[0] * A[0];
    const double l01 = A[0] * A[1];
    const double l02 = A[0] * A[2];
    const double l03 = A[0] * A[3];
    const double l04 = A[0] * A[4];
    const double l05 = A[0] * A[5];
    const double l11 = A[1] * A[1];
    const double l12 = A[1] * A[2];
    const double l13 = A[1] * A[3];
    const double l14 = A[1] * A[4];
    const double l15 = A[1] * A[5];
    const double l22 = A[2] * A[2];
    const double l23 = A[2] * A[3];
    const double l24 = A[2] * A[4];
    const double l25 = A[2] * A[5];
    const double l33 = A[3] * A[3];
    const double l34 = A[3] * A[4];
    const double l35 = A[3] * A[5];
    const double l44 = A[4] * A[4];
    const double l45 = A[4] * A[5];
    const double l55 = A[5] * A[5];
    // L: add non-diagonals
    l1 = l01;
    l2 = l02;
    l3 = l03 + l12;
    l4 = l04 + l13;
    l5 = l05 + l14 + l23;
    l6 = l15 + l24;
    l7 = l25 + l34;
    l8 = l35;
    l9 = l45;
    // L: double non-diagonals and add diagonals
    l0  =      l00;
    l1  = l1 +       l1;
    l2  = l2 + l11 + l2;
    l3  = l3 +       l3;
    l4  = l4 + l22 + l4;
    l5  = l5 +       l5;
    l6  = l6 + l33 + l6;
    l7  = l7 +       l7;
    l8  = l8 + l44 + l8;
    l9  = l9 +       l9;
    l10 =      l55;

    // Compute H
    const double h00 =  A6_shr *  A6_shr;
    const double h01 =  A6_shr *  A7_shr;
    const double h02 =  A6_shr *  A8_shr;
    const double h03 =  A6_shr *  A9_shr;
    const double h04 =  A6_shr * A10_shr;
    const double h05 =  A6_shr * A11_shr;
    const double h11 =  A7_shr *  A7_shr;
    const double h12 =  A7_shr *  A8_shr;
    const double h13 =  A7_shr *  A9_shr;
    const double h14 =  A7_shr * A10_shr;
    const double h15 =  A7_shr * A11_shr;
    const double h22 =  A8_shr *  A8_shr;
    const double h23 =  A8_shr *  A9_shr;
    const double h24 =  A8_shr * A10_shr;
    const double h25 =  A8_shr * A11_shr;
    const double h33 =  A9_shr *  A9_shr;
    const double h34 =  A9_shr * A10_shr;
    const double h35 =  A9_shr * A11_shr;
    const double h44 = A10_shr * A10_shr;
    const double h45 = A10_shr * A11_shr;
    const double h55 = A11_shr * A11_shr;
    // H: add non-diagonals
    h1 = h01;
    h2 = h02;
    h3 = h03 + h12;
    h4 = h04 + h13;
    h5 = h05 + h14 + h23;
    h6 = h15 + h24;
    h7 = h25 + h34;
    h8 = h35;
    h9 = h45;
    // H: double non-diagonals and add diagonals
    h0  =      h00;
    h1  = h1 +       h1;
    h2  = h2 + h11 + h2;
    h3  = h3 +       h3;
    h4  = h4 + h22 + h4;
    h5  = h5 +       h5;
    h6  = h6 + h33 + h6;
    h7  = h7 +       h7;
    h8  = h8 + h44 + h8;
    h9  = h9 +       h9;
    h10 =      h55;

    // Compute M_hat
    const double mA0 = (A[0] -  A6_shr);
    const double mA1 = (A[1] -  A7_shr);
    const double mA2 = (A[2] -  A8_shr);
    const double mA3 = (A[3] -  A9_shr);
    const double mA4 = (A[4] - A10_shr);
    const double mA5 = (A[5] - A11_shr);

    const double m00 = mA0 * mA0;
    const double m01 = mA0 * mA1;
    const double m02 = mA0 * mA2;
    const double m03 = mA0 * mA3;
    const double m04 = mA0 * mA4;
    const double m05 = mA0 * mA5;
    const double m11 = mA1 * mA1;
    const double m12 = mA1 * mA2;
    const double m13 = mA1 * mA3;
    const double m14 = mA1 * mA4;
    const double m15 = mA1 * mA5;
    const double m22 = mA2 * mA2;
    const double m23 = mA2 * mA3;
    const double m24 = mA2 * mA4;
    const double m25 = mA2 * mA5;
    const double m33 = mA3 * mA3;
    const double m34 = mA3 * mA4;
    const double m35 = mA3 * mA5;
    const double m44 = mA4 * mA4;
    const double m45 = mA4 * mA5;
    const double m55 = mA5 * mA5;
    // M_hat: add non-diagonals
    m1 = m01;
    m2 = m02;
    m3 = m03 + m12;
    m4 = m04 + m13;
    m5 = m05 + m14 + m23;
    m6 = m15 + m24;
    m7 = m25 + m34;
    m8 = m35;
    m9 = m45;
    // M_hat: double non-diagonals and add diagonals
    m0  =      m00;
    m1  = m1 +       m1;
    m2  = m2 + m11 + m2;
    m3  = m3 +       m3;
    m4  = m4 + m22 + m4;
    m5  = m5 +       m5;
    m6  = m6 + m33 + m6;
    m7  = m7 +       m7;
    m8  = m8 + m44 + m8;
    m9  = m9 +       m9;
    m10 =      m55;

    // Sum up the accs into C
    C[ 0] =  l0 + 0x26p-128 * ( -m6 +  l6 +  h6) + 0x26*h0;
    C[ 1] =  l1 + 0x26p-128 * ( -m7 +  l7 +  h7) + 0x26*h1;
    C[ 2] =  l2 + 0x26p-128 * ( -m8 +  l8 +  h8) + 0x26*h2;
    C[ 3] =  l3 + 0x26p-128 * ( -m9 +  l9 +  h9) + 0x26*h3;
    C[ 4] =  l4 + 0x26p-128 * (-m10 + l10 + h10) + 0x26*h4;
    C[ 5] =  l5                                  + 0x26*h5;
    C[ 6] =  l6 +  0x1p+128 * ( -m0 +  l0 +  h0) + 0x26*h6;
    C[ 7] =  l7 +  0x1p+128 * ( -m1 +  l1 +  h1) + 0x26*h7;
    C[ 8] =  l8 +  0x1p+128 * ( -m2 +  l2 +  h2) + 0x26*h8;
    C[ 9] =  l9 +  0x1p+128 * ( -m3 +  l3 +  h3) + 0x26*h9;
    C[10] = l10 +  0x1p+128 * ( -m4 +  l4 +  h4) + 0x26*h10;
    C[11] =        0x1p+128 * ( -m5 +  l5 +  h5);
}
