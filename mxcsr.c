// Helper functions for setting and restoring the MxCsr register
//
// Author: Daan Sprenkels <hello@dsprenkels.com>

#include "mxcsr.h"
#include <stdbool.h>
#include <xmmintrin.h>

const unsigned int app_mxcsr = 0x1f80;

unsigned int replace_mxcsr() {
    const unsigned prev = _mm_getcsr();
    _mm_setcsr(app_mxcsr);
    return prev;
}

bool restore_mxcsr(const unsigned int restore_val) {
    unsigned int mxcsr = _mm_getcsr();
    _mm_setcsr(restore_val);
    mxcsr &= ~(1 << 5); // We do not care about the precision flag
    return mxcsr == app_mxcsr;
}
