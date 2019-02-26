/*
The type for a normal floating point Field Element with 12 limbs

In our case, the field is GF(2^255 - 19). The layout of this type is based on
[NEONCrypto2012], but instead we use double precision floating point values.
I use "floating point radix 2^21.25", i.e. alternating 2^22 and 2^21.

In other words, an element (t :: fe12) represents the integer:

    t[0] + t[1] + t[2] + t[3] + t[4] + ... + t[11]

with:
    t[ 0] is divisible by 1
    t[ 1] is divisible by 2^22
    t[ 2] is divisible by 2^43
    t[ 3] is divisible by 2^64
    t[ 4] is divisible by 2^85
    t[ 5] is divisible by 2^107
    t[ 6] is divisible by 2^128
    t[ 7] is divisible by 2^149
    t[ 8] is divisible by 2^170
    t[ 9] is divisible by 2^192
    t[10] is divisible by 2^213
    t[11] is divisible by 2^234

We must make sure that the we do not lose any precision. We do this by carrying
the top part from a limb to the next one, similar to how we would to it when we
would be handling large integers.

[NEONCrypto2012]:
Bernstein, D. J. & Schwabe, P. Prouff, E. & Schaumont, P. (Eds.)
"NEON Crypto Cryptographic Hardware and Embedded Systems"
*/

#ifndef REF12_FE12_H_
#define REF12_FE12_H_

#include <stdint.h>

typedef double fe12[12];
typedef double fe12_frozen[6];

#define fe12_frombytes crypto_scalarmult_curve13318_ref12_fe12_frombytes
#define fe12_zero crypto_scalarmult_curve13318_ref12_fe12_zero
#define fe12_one crypto_scalarmult_curve13318_ref12_fe12_one
#define fe12_copy crypto_scalarmult_curve13318_ref12_fe12_copy
#define fe12_add crypto_scalarmult_curve13318_ref12_fe12_add
#define fe12_sub crypto_scalarmult_curve13318_ref12_fe12_sub
#define fe12_mul_small crypto_scalarmult_curve13318_ref12_fe12_mul_small
#define fe12_squeeze crypto_scalarmult_curve13318_ref12_fe12_squeeze
#define fe12_mul crypto_scalarmult_curve13318_ref12_fe12_mul_karatsuba
#define fe12_mul_schoolbook crypto_scalarmult_curve13318_ref12_fe12_mul_schoolbook
#define fe12_square crypto_scalarmult_curve13318_ref12_fe12_square_karatsuba
#define fe12_freeze crypto_scalarmult_curve13318_ref12_fe12_freeze
#define fe12_add_b crypto_scalarmult_curve13318_ref12_fe12_add_b
#define fe12_mul_b crypto_scalarmult_curve13318_ref12_fe12_mul_b

/*
Set a fe12 value to zero
*/
static inline void fe12_zero(fe12 z) {
    for (unsigned int i = 0; i < 12; i++) z[i] = 0;
}

/*
Set a fe12 value to one
*/
static inline void fe12_one(fe12 z) {
    z[0] = 1;
    for (unsigned int i = 1; i < 12; i++) z[i] = 0;
}

/*
Copy a fe12 value to another fe12 type
*/
static inline void fe12_copy(fe12 dest, const fe12 src) {
    for (unsigned int i = 0; i < 12; i++) dest[i] = src[i];
}

/*
Add `rhs` into `z`
*/
static inline void fe12_add(fe12 z, fe12 lhs, fe12 rhs) {
    for (unsigned int i = 0; i < 12; i++) z[i] = lhs[i] + rhs[i];
}

/*
Subtract `rhs` from `lhs` and store the result in `z`
*/
static inline void fe12_sub(fe12 z, fe12 lhs, fe12 rhs) {
    for (unsigned int i = 0; i < 12; i++) z[i] = lhs[i] - rhs[i];
}

/*
Multiply z by a small constant
*/
static inline void fe12_mul_small(fe12 z, const double n) {
    for (unsigned int i = 0; i < 12; i++) z[i] = n * z[i];
}

/*
Parse 32 bytes into a `fe12` type
*/
extern void fe12_frombytes(fe12 element, const uint8_t *bytes);

/*
Carry ripple this field element
*/
extern void fe12_squeeze(fe12 element);

/*
Multiply two field elements,
*/
extern void fe12_mul(fe12 dest, const fe12 op1, const fe12 op2);

/*
Square a field element
*/
extern void fe12_square(fe12 dest, const fe12 element);

/*
Reduce an element s.t. the result is always in [0, 2^255-19⟩. Input must
be squeezed
*/
extern void fe12_freeze(fe12_frozen out, const fe12 element);

/*
Add 13318 to `z`
*/
static inline void fe12_add_b(fe12 z) {
    z[0] += 13318;
}

/*
Multiply `f` by 13318 and store the result in `h`
*/
static inline void fe12_mul_b(fe12 h, fe12 f) {
    fe12_copy(h, f);
    fe12_mul_small(h, 13318);
}

#endif /* REF12_FE12_H_ */
