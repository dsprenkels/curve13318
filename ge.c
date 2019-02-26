#include "fe_convert.h"
#include "ge.h"
#include <stdbool.h>

static bool ge_affine_point_on_curve(ge p)
{
    // Use the general curve equation to check if this point is on the curve
    // y^2 = x^3 - 3*x + 13318
    // TODO(dsprenkels) Implement this function using radix-2^51 arithmetic.
    fe10 x, y, lhs, rhs, t0;
    fe10_frozen result;
    convert_fe12_to_fe10(x, p[0]);
    convert_fe12_to_fe10(y, p[1]);
    fe10_square(lhs, y);     // y^2
    fe10_square(t0, x);      // x^2
    fe10_mul(rhs, t0, x);    // x^3
    fe10_zero(t0);            // 0
    fe10_add2p(t0);           // 0
    fe10_sub(t0, t0, x);     // -x
    fe10_add(rhs, rhs, t0);   // x^3 - x
    fe10_add(rhs, rhs, t0);   // x^3 - 2*x
    fe10_add(rhs, rhs, t0);   // x^3 - 3*x
    fe10_add_b(rhs);          // x^3 - 3*x + 13318
    fe10_carry(rhs);
    fe10_add2p(lhs);          // Still y^2
    fe10_sub(lhs, lhs, rhs);  // (==0) or (!=0) mod p
    fe10_carry(lhs);
    fe10_reduce(result, lhs); // 0 or !0

    uint64_t nonzero = 0;
    for (unsigned int i = 0; i < 5; i++) nonzero |= result[i];
    return nonzero == 0;
}

int ge_frombytes(ge p, const uint8_t *s)
{
    // XXX(dsprenkels) I should think about input coordinates larger than p.
    // In this case, we should definitely ditch the last (256'th) bit if it
    // is set one of the coordinates. However, I am not really sure if we lose
    // any guarantees if an input coordinates is larger or equal to p, but
    // smaller than 2^255, i.e. `p <= x <= 2^255`.
    // We do not risk going out of bounds here, and the computation will go
    // along as if just reduced by p, which cannot force into any invalid
    // points (as long as we have checked with `ge_affine_point_on_curve`).

    fe12_frombytes(p[0], &s[0]);
    fe12_frombytes(p[1], &s[32]);

    // Handle point at infinity encoded by (0, 0)
    uint64_t infinity = 1;
    for (unsigned int i = 0; i < 12; i++) infinity &= p[0][i] == 0;
    for (unsigned int i = 0; i < 12; i++) infinity &= p[1][i] == 0;

    // Set y to 1 if we are at the point at infinity
    p[1][0] += 1 * infinity;
    // Initialize z to 1 (or 0 if infinity)
    p[2][0]  = 1 * !infinity;
    for (unsigned int i = 1; i < 12; i++) p[2][i] = 0;

    // Check if this point is valid
    if (!infinity & !ge_affine_point_on_curve(p)) return -1;
    return 0;
}

void ge_tobytes(uint8_t *s, ge p)
{
    /*
    This function actually deals with the point at infinity, encoded as (0, 0).
    Namely, if `z` (`p[2]`) is zero, because of the implementation of
    `fe10_invert`, `z_inverse` will also be 0. And so, the coordinates that are
    encoded into `s` are 0.
    */
    fe51 x, y, z, x_affine, y_affine, z_inverse;

    // Move to fe10, because 4x parallelization is not possible anymore
    convert_fe12_to_fe51(&x, p[0]);
    convert_fe12_to_fe51(&y, p[1]);
    convert_fe12_to_fe51(&z, p[2]);

    // Convert to affine coordinates
    fe51_invert(&z_inverse, &z);
    fe51_mul(&x_affine, &x, &z_inverse);
    fe51_mul(&y_affine, &y, &z_inverse);

    // Write the affine numbers to the buffer
    fe51_pack(&s[ 0], &x_affine);
    fe51_pack(&s[32], &y_affine);
}

void ge_add_c(ge p3, const ge p1, const ge p2)
{
    fe12 x1, y1, z1, x2, y2, z2, x3, y3, z3, t0, t1, t2, t3, t4;
    fe12_copy(x1, p1[0]);
    fe12_copy(y1, p1[1]);
    fe12_copy(z1, p1[2]);
    fe12_copy(x2, p2[0]);
    fe12_copy(y2, p2[1]);
    fe12_copy(z2, p2[2]);

    /*
    The next chain of procedures is *exactly* Algorithm 4 from the
    Renes-Costello-Batina addition laws. [Renes2016]

    fe12_squeeze guarantees that every processed double is always divisible
    by 2^k and bounded by 1.01 * 2^21 * 2^k, with k the limb's offset
    (0, 22, 43, etc.). This theorem (3.2) is proven in [Hash127] by Daniel
    Bernstein, although it needs to be adapted to this instance.
    Precondition of the theorem is that the input to fe12_squeeze is divisible
    by 2^k and bounded by 0.98 * 2^53 * 2^k.

    In other words: Any product limbs produced by fe12_mul (uncarried), must be
    bounded by ±0.98 * 2^53. In fe12_mul, the lowest limb is multiplied by the
    largest value, namely ±(11*19 + 1)*x*y = ±210*x*y for x the largest possible
    22-bit limbs. This means that the summed limb bits of the 2 multiplied
    operands cannot exceed ±0.98 * 2^53 / 210. Rounded down this computes to
    ~±2^45.2 > ±1.1*2^45. So if we restrict ourselves to a multiplied upper bound
    of ±1.1*2^45, we should be all right.

    We would manage this by multiplying 2^21 values with 2^24 values
    (because 21 + 24 ≤ 45), but for example 2^23 * 2^23 is *forbidden* as it
    may overflow (23 + 23 > 45).
    */

    /*   #: Instruction number as mentioned in the paper */
              // Assume forall x in {x, y, z} : |x| ≤ 1.01 * 2^21
              fe12_mul(t0, x1, x2); // |t0| ≤ 1.68 * 2^49
              fe12_mul(t1, y1, y2); // |t1| ≤ 1.68 * 2^49
              fe12_mul(t2, z1, z2); // |t2| ≤ 1.68 * 2^49
              fe12_add(t3, x1, y1); // |t3| ≤ 1.01 * 2^22
    /*  5 */  fe12_add(t4, x2, y2); // |t4| ≤ 1.01 * 2^22
              fe12_mul(t3, t3, t4); // |t3| ≤ 1.68 * 2^51
              fe12_add(t4, t0, t1); // |t4| ≤ 1.68 * 2^50
              fe12_sub(t3, t3, t4); // |t3| ≤ 1.26 * 2^52
              fe12_add(t4, y1, z1); // |t4| ≤ 1.01 * 2^22
    /* 10 */  fe12_add(x3, y2, z2); // |x3| ≤ 1.01 * 2^22
              fe12_mul(t4, t4, x3); // |t4| ≤ 1.01 * 2^23
              fe12_add(x3, t1, t2); // |x3| ≤ 1.26 * 2^51
              fe12_sub(t4, t4, x3); // |t4| ≤ 1.27 * 2^51
              fe12_add(x3, x1, z1); // |x3| ≤ 1.01 * 2^22
    /* 15 */  fe12_add(y3, x2, z2); // |y3| ≤ 1.01 * 2^22
              fe12_mul(x3, x3, y3); // |x3| ≤ 1.68 * 2^51
              fe12_add(y3, t0, t2); // |y3| ≤ 1.68 * 2^50
              fe12_sub(y3, x3, y3); // |y3| ≤ 1.26 * 2^52
    /* __ */  fe12_squeeze(y3);     // squeeze |y3| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(t0);     // squeeze |t1| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(t1);     // squeeze |t1| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(t2);     // squeeze |t2| ≤ 1.01 * 2^21
              fe12_mul_b(z3, t2);   // |z3| ≤ 1.65 * 2^34
    /* 20 */  fe12_sub(x3, y3, z3); // |x3| ≤ 1.66 * 2^34
              fe12_add(z3, x3, x3); // |z3| ≤ 1.66 * 2^35
              fe12_add(x3, x3, z3); // |x3| ≤ 1.25 * 2^36
              fe12_sub(z3, t1, x3); // |z3| ≤ 1.26 * 36
              fe12_add(x3, t1, x3); // |x3| ≤ 1.26 * 36
    /* 25 */  fe12_mul_b(y3, y3);   // |y3| ≤ 1.65 * 2^34
              fe12_add(t1, t2, t2); // |t1| ≤ 1.01 * 2^22
              fe12_add(t2, t1, t2); // |t2| ≤ 1.52 * 2^22
              fe12_sub(y3, y3, t2); // |y3| ≤ 1.66 * 2^34
              fe12_sub(y3, y3, t0); // |y3| ≤ 1.67 * 2^34
    /* 30 */  fe12_add(t1, y3, y3); // |t1| ≤ 1.67 * 2^35
              fe12_add(y3, t1, y3); // |y3| ≤ 1.26 * 2^36
              fe12_add(t1, t0, t0); // |t1| ≤ 1.01 * 2^22
              fe12_add(t0, t1, t0); // |t0| ≤ 1.52 * 2^22
              fe12_sub(t0, t0, t2); // |t0| ≤ 1.52 * 2^23
    /* __ */  fe12_squeeze(t4);     // squeeze |t4| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(x3);     // squeeze |x3| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(y3);     // squeeze |y3| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(z3);     // squeeze |z3| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(t0);     // squeeze |z3| ≤ 1.01 * 2^21
    /* 35 */  fe12_mul(t1, t4, y3); // |t1| ≤ 1.68 * 2^49
              fe12_mul(t2, t0, y3); // |t2| ≤ 1.26 * 2^52
              fe12_mul(y3, x3, z3); // |y3| ≤ 1.68 * 2^49
    /* __ */  fe12_squeeze(t3);     // squeeze |t3| ≤ 1.01 * 2^21
              fe12_add(y3, y3, t2); // |y3| ≤ 1.47 * 2^52
              fe12_mul(x3, x3, t3); // |x3| ≤ 1.68 * 2^49
    /* 40 */  fe12_sub(x3, x3, t1); // |x3| ≤ 1.68 * 2^50
              fe12_mul(z3, z3, t4); // |z3| ≤ 1.68 * 2^49
              fe12_mul(t1, t3, t0); // |t1| ≤ 1.68 * 2^49
              fe12_add(z3, z3, t1); // |z3| ≤ 1.68 * 2^50

    // Squeeze x3..z3 for next time
    fe12_squeeze(x3);
    fe12_squeeze(y3);
    fe12_squeeze(z3);

    fe12_copy(p3[0], x3);
    fe12_copy(p3[1], y3);
    fe12_copy(p3[2], z3);
}

void ge_double_c(ge p3, const ge p)
{
    fe12 x, y, z, x3, y3, z3, t0, t1, t2, t3;
    fe12_copy(x, p[0]);
    fe12_copy(y, p[1]);
    fe12_copy(z, p[2]);

    /*
    The same rules and assumptions from ge_add are in force here. Notably, we
    know that a multiplication result is bounded by ±1.01 * 2^21 and that it is
    all right if the product of two fe12_mul operands is not bigger than
    0.98 * 2^53 * 2^k.
    */
    /*   #: Instruction number as mentioned in the paper */
              // Assume forall x in {x1, z1, y1, y2, x2, z2} : |x| ≤ 1.01 * 2^21
              fe12_square(t0, x);   // |t0| ≤ 1.68 * 2^49
              fe12_square(t1, y);   // |t1| ≤ 1.68 * 2^49
              fe12_square(t2, z);   // |t2| ≤ 1.68 * 2^49
              fe12_mul(t3, x, y);   // |t3| ≤ 1.68 * 2^49
    /*  5 */  fe12_add(t3, t3, t3); // |t3| ≤ 1.68 * 2^50
    /* __ */  fe12_squeeze(t2);     // squeeze |t2| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(t3);     // squeeze |t2| ≤ 1.01 * 2^21
              fe12_mul(z3, x, z);   // |z3| ≤ 1.68 * 2^49
              fe12_add(z3, z3, z3); // |z3| ≤ 1.68 * 2^50
              fe12_mul_b(y3, t2);   // |y3| ≤ 1.65 * 2^34
              fe12_sub(y3, y3, z3); // |y3| ≤ 1.69 * 2^50
    /* 10 */  fe12_add(x3, y3, y3); // |x3| ≤ 1.69 * 2^51
              fe12_add(y3, x3, y3); // |y3| ≤ 1.27 * 2^52
              fe12_sub(x3, t1, y3); // |x3| ≤ 1.48 * 2^52
              fe12_add(y3, t1, y3); // |y3| ≤ 1.48 * 2^52
    /* __ */  fe12_squeeze(x3);     // squeeze |x3| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(y3);     // squeeze |y3| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(z3);     // squeeze |z3| ≤ 1.01 * 2^21
              fe12_mul(y3, x3, y3); // |y3| ≤ 1.01 * 2^21
    /* 15 */  fe12_mul(x3, x3, t3); // |x3| ≤ 1.01 * 2^21
              fe12_add(t3, t2, t2); // |t3| ≤ 1.01 * 2^22
              fe12_add(t2, t2, t3); // |t3| ≤ 1.52 * 2^22
              fe12_mul_b(z3, z3);   // |z3| ≤ 1.65 * 2^34
              fe12_sub(z3, z3, t2); // |z3| ≤ 1.66 * 2^34
    /* 20 */  fe12_sub(z3, z3, t0); // |z3| ≤ 1.69 * 2^49
              fe12_add(t3, z3, z3); // |t3| ≤ 1.69 * 2^50
              fe12_add(z3, z3, t3); // |z3| ≤ 1.27 * 2^51
              fe12_add(t3, t0, t0); // |t3| ≤ 1.68 * 2^50
              fe12_add(t0, t3, t0); // |t0| ≤ 1.26 * 2^51
    /* 25 */  fe12_sub(t0, t0, t2); // |t0| ≤ 1.27 * 2^51
    /* __ */  fe12_squeeze(t0);     // squeeze |t0| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(z3);     // squeeze |z3| ≤ 1.01 * 2^21
              fe12_mul(t0, t0, z3); // |t0| ≤ 1.68 * 2^49
              fe12_add(y3, y3, t0); // |y3| ≤ 1.69 * 2^49
              fe12_mul(t0, y, z);   // |t0| ≤ 1.68 * 2^49
              fe12_add(t0, t0, t0); // |t0| ≤ 1.68 * 2^50
    /* __ */  fe12_squeeze(t0);     // squeeze |t0| ≤ 1.01 * 2^21
    /* 30 */  fe12_mul(z3, t0, z3); // |z3| ≤ 1.68 * 2^50
              fe12_sub(x3, x3, z3); // |x3| ≤ 1.69 * 2^50
    /* __ */  fe12_squeeze(t0);     // squeeze |t0| ≤ 1.01 * 2^21
    /* __ */  fe12_squeeze(t1);     // squeeze |t1| ≤ 1.01 * 2^21
              fe12_mul(z3, t0, t1); // |z3| ≤ 1.68 * 2^49
              fe12_add(z3, z3, z3); // |z3| ≤ 1.68 * 2^50
              fe12_add(z3, z3, z3); // |z3| ≤ 1.68 * 2^51

    // Squeeze x3 and z3, otherwise we will get into trouble during the next
    // Addition/doubling
    fe12_squeeze(x3);
    fe12_squeeze(y3);
    fe12_squeeze(z3);

    fe12_copy(p3[0], x3);
    fe12_copy(p3[1], y3);
    fe12_copy(p3[2], z3);
}
