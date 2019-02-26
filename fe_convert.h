/*
Convert a fe12 floating point representation to fe10 integer representation
*/
#ifndef REF12_FE_CONVERT_H_
#define REF12_FE_CONVERT_H_

#include "fe12.h"
#include "fe10.h"
#include "fe51.h"
#include <stdint.h>

static const uint64_t _MASK42 = 0xFFFFFC0000000000;
static const uint64_t _MASK43 = 0xFFFFF80000000000;

#define convert_fe12_to_fe10 crypto_scalarmult_curve13318_ref12_convert_fe12_to_fe10
#define convert_fe12_to_fe51 crypto_scalarmult_curve13318_ref12_convert_fe12_to_fe51

void convert_fe12_to_fe10(fe10 out, const fe12 in);
void convert_fe12_to_fe51(fe51 *out, const fe12 in);

#endif /* REF12_FE_CONVERT_H_ */
