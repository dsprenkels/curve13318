/*
As you may have noticed. This library uses floating points to compute its finite field arithmetic.
In the carry ripple we force the loss of precision using the tricks from [Hash127]. The caller of
this library could have set the rounding mode of the CPU to something different from what we would
expect. So here we *explicitly* set the rounding mode to 'round-to-nearest'.

We do not expect floating point exceptions during the scalar multiplication. Those would most
likely indicate a bug in the software. However, *if* any of these would occur, we do not want them
to interrupt the execution, as this would probably introduce a severe (timing) oracle.
*/

#ifndef REF12_MXCSR_H_
#define REF12_MXCSR_H_

#include <stdbool.h>

#define replace_mxcsr crypto_scalarmult_curve13318_ref12_replace_mxcsr
#define restore_mxcsr crypto_scalarmult_curve13318_ref12_restore_mxcsr

// Replaces the MxCsr with the value desired by our algorithm and save the old value to [rdi]
//
// Returns the old value in the MxCsr. The caller should save this value to retore it later.
//
// This function
//   - Sets the rounding mode to 'round-to-nearest'
//   - Clears any exceptions
//   - Disables any exceptions
unsigned int replace_mxcsr(void);

// Restores the MxCsr with the value in [rdi] and check if the overridden MxCsr is intact.
//
// Arguments:
//   - [rdi]: Pointer to where the old MxCsr value had been saved
//
// Returns true if the MxCsr is unaltered. When false is returned, this probably indicates a bug
// and in this case the caller should abort the computation.
bool restore_mxcsr(const unsigned int);

#endif /* REF12_MXCSR_H_ */
