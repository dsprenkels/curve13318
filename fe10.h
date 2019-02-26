/*
The type for the alternative integer Field Element with 10 limbs

In the fe10 case, the field is still GF(2^255 - 19). The layout is also based
on [NEONCrypto2012]. It uses "radix 2^25.5", i.e. alternating 2^26 and 2^26.
In other words, an element (t :: fe10) represents the integer:

t[0] + 2^26*t[1] + 2^51*t[2] + 2^77*t[3] + 2^102*t[4] + ... + 2^230*t[9]

[NEONCrypto2012]:
Bernstein, D. J. & Schwabe, P. Prouff, E. & Schaumont, P. (Eds.)
"NEON Crypto Cryptographic Hardware and Embedded Systems"
*/

#ifndef REF12_FE10_H_
#define REF12_FE10_H_

#include <stdint.h>

static const uint64_t _2P0 = 0x07FFFFDA;
static const uint64_t _2PRestB25 = 0x03FFFFFE;
static const uint64_t _2PRestB26 = 0x07FFFFFE;
static const uint64_t _4P0 = 0x0FFFFFB4;
static const uint64_t _4PRestB25 = 0x07FFFFFC;
static const uint64_t _4PRestB26 = 0x0FFFFFFC;
static const uint64_t _CURVE13318_B = 13318;
static const uint64_t _MASK25 = 0xFFFFFFFFFE000000;
static const uint64_t _MASK26 = 0xFFFFFFFFFC000000;
static const uint64_t _MASK51 = 0xFFF8000000000000;

typedef uint64_t fe10[10];
typedef uint64_t fe10_frozen[5];

#define fe10_frombytes crypto_scalarmult_curve13318_ref12_fe10_frombytes
#define fe10_tobytes crypto_scalarmult_curve13318_ref12_fe10_tobytes
#define fe10_zero crypto_scalarmult_curve13318_ref12_fe10_zero
#define fe10_one crypto_scalarmult_curve13318_ref12_fe10_one
#define fe10_copy crypto_scalarmult_curve13318_ref12_fe10_copy
#define fe10_add crypto_scalarmult_curve13318_ref12_fe10_add
#define fe10_add2p crypto_scalarmult_curve13318_ref12_fe10_add2p
#define fe10_mul crypto_scalarmult_curve13318_ref12_fe10_mul
#define fe10_square crypto_scalarmult_curve13318_ref12_fe10_square
#define fe10_carry crypto_scalarmult_curve13318_ref12_fe10_carry
#define fe10_invert crypto_scalarmult_curve13318_ref12_fe10_invert
#define fe10_add_b crypto_scalarmult_curve13318_ref12_fe10_add_b
#define fe10_mul_b crypto_scalarmult_curve13318_ref12_fe10_mul_b
#define fe10_reduce crypto_scalarmult_curve13318_ref12_fe10_reduce

/*
Set a fe10 value to zero
*/
static inline void fe10_zero(fe10 z) {
    for (unsigned int i = 0; i < 10; i++) z[i] = 0;
}

/*
Set a fe10 value to one
*/
static inline void fe10_one(fe10 z) {
    z[0] = 1;
    for (unsigned int i = 1; i < 10; i++) z[i] = 0;
}

/*
Copy a fe10 value to another fe10 type
*/
static inline void fe10_copy(fe10 dest, const fe10 src) {
    for (unsigned int i = 0; i < 10; i++) dest[i] = src[i];
}

/*
Add `rhs` into `z`
*/
static inline void fe10_add(fe10 z, fe10 lhs, fe10 rhs) {
    for (unsigned int i = 0; i < 10; i++) z[i] = lhs[i] + rhs[i];
}

/*
Add 2*p to the field element `z`, this ensures that:
    - z limbs will be at least 2^26 resp. 2^25
*/
static inline void fe10_add2p(fe10 z) {
    z[0] += _2P0;
    z[1] += _2PRestB25;
    z[2] += _2PRestB26;
    z[3] += _2PRestB25;
    z[4] += _2PRestB26;
    z[5] += _2PRestB25;
    z[6] += _2PRestB26;
    z[7] += _2PRestB25;
    z[8] += _2PRestB26;
    z[9] += _2PRestB25;
}

/*
Subtract `rhs` from `z`. This function does *not* work if any of the resulting
limbs underflow! Ensure that this is not occurs by adding additional carry
rippling and using `fe10_add2p`.
*/
static inline void fe10_sub(fe10 z, fe10 lhs, fe10 rhs) {
    for (unsigned int i = 0; i < 10; i++) z[i] = lhs[i] - rhs[i];
}

/*
Parse 32 bytes into a `fe10` type
*/
extern void fe10_frombytes(fe10 element, const uint8_t *bytes);

/*
Store a field element type into memory
*/
extern void fe10_tobytes(uint8_t *bytes, fe10 element);

/*
Multiply two field elements,
*/
extern void fe10_mul(fe10 dest, const fe10 op1, const fe10 op2);

/*
Square a field element
*/
extern void fe10_square(fe10 dest, const fe10 element);

/*
Reduce this vectorized elements modulo 2^25.5
*/
extern void fe10_carry(fe10 element);

/*
Invert an element modulo 2^255 - 19
*/
extern void fe10_invert(fe10 dest, const fe10 element);

/*
Reduce an element s.t. the result is always in [0, 2^255-19⟩
*/
extern void fe10_reduce(fe10_frozen out, const fe10 element);

/*
Add 13318 to `z`
*/
static inline void fe10_add_b(fe10 z) {
    z[0] += _CURVE13318_B;
}

/*
Multiply `z` by 13318
*/
static inline void fe10_mul_b(fe10 z, fe10 op) {
    for (unsigned int i = 0; i < 10; i++) z[i] = op[i] * _CURVE13318_B;
    fe10_carry(z);
}

#endif // REF12_FE10_H_
