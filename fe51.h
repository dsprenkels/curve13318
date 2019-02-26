/*
   This file is adapted from sandy2x/fe51.h:
*/

#ifndef REF12_FE51_H_
#define REF12_FE51_H_

#include <stdint.h>

#define fe51_pack crypto_scalarmult_curve13318_ref12_fe51_pack
#define fe51_mul crypto_scalarmult_curve13318_ref12_fe51_mul
#define fe51_nsquare crypto_scalarmult_curve13318_ref12_fe51_nsquare
#define fe51_invert crypto_scalarmult_curve13318_ref12_fe51_invert

typedef struct
{
  uint64_t v[5];
}
fe51;

extern void fe51_pack(unsigned char *, const fe51 *);
extern void fe51_mul(fe51 *, const fe51 *, const fe51 *);
extern void fe51_nsquare(fe51 *, const fe51 *, int);
extern void fe51_invert(fe51 *, const fe51 *);

#endif /* REF12_FE51_H_ */
