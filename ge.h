/*
Group element in our curve E : y^2 = x^3 - 3*x + 13318

Because of the limitations of the Renes-Costello-Batina addition formulas, a
point on E is represented by its projective coordinates, i.e. (X : Y : Z).
*/

#ifndef CURVE13318_REF12_GE_H_
#define CURVE13318_REF12_GE_H_

#include "fe12.h"
#include "fe10.h"

typedef fe12 ge[3];

#define ge_neutral crypto_scalarmult_curve13318_ref12_ge_neutral
#define ge_copy crypto_scalarmult_curve13318_ref12_ge_copy
#define ge_cneg crypto_scalarmult_curve13318_ref12_ge_cneg
#define ge_frombytes crypto_scalarmult_curve13318_ref12_ge_frombytes
#define ge_tobytes crypto_scalarmult_curve13318_ref12_ge_tobytes
#define ge_add_c crypto_scalarmult_curve13318_ref12_ge_add_c
#define ge_add crypto_scalarmult_curve13318_ref12_ge_add
#define ge_double crypto_scalarmult_curve13318_ref12_ge_double
#define ge_double_c crypto_scalarmult_curve13318_ref12_ge_double_c

/*
Write all zeros to p
*/
static inline void ge_zero(ge p)
{
    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 12; j++) {
            p[i][j] = 0;
        }
    }
}

/*
Write the neutral element (0 : 1 : 0) to point
*/
static inline void ge_neutral(ge point) {
    fe12_zero(point[0]);
    fe12_one(point[1]);
    fe12_zero(point[2]);
}

/*
Copy a ge value to another ge type
*/
static inline void ge_copy(ge dest, const ge src) {
    fe12_copy(dest[0], src[0]);
    fe12_copy(dest[1], src[1]);
    fe12_copy(dest[2], src[2]);
}

/*
Conditionally negate a ge value. `c` must be exactly 0 or 1
*/
static inline void ge_cneg(ge point, uint8_t c) {
    const double n = 1 - 2*c;
    fe12_mul_small(point[1], n);
}

/*
Parse a bytestring into a point on the curve

Arguments:
  - point   Output point
  - bytes   Input bytes
Returns:
  0 on succes, nonzero on failure
*/
int ge_frombytes(ge point, const uint8_t *bytes);

/*
Convert a projective point on the curve to its byte representation

Arguments:
  - bytes   Output bytes
  - point   Output point
Returns:
  0 on succes, nonzero on failure
*/
void ge_tobytes(uint8_t *bytes, ge point);

/*
Add two `point_1` and `point_2` into `dest`.
*/
void ge_add(ge dest, const ge point_1, const ge point_2);

/*
Double `point` into `dest`.
*/
void ge_double(ge dest, const ge point);

#endif /* CURVE13318_REF12_GE_H_ */
