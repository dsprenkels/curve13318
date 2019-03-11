#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

extern int crypto_scalarmult_curve13318_scalarmult(uint8_t*, const uint8_t*, const uint8_t*);

static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

int main(int argc, char *argv[])
{
    unsigned long long start, diff, blank = 58; // blank was measure by me, by hand

    uint8_t out[64] = {0};
    const uint8_t key[32] = {1};
    const uint8_t in[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 179, 43, 106, 247, 206, 176, 201, 77, 137, 224, 122, 176, 76, 93, 29, 69, 190, 137, 17, 103, 105, 172, 236, 172, 225, 72, 243, 7, 94, 128, 240, 17};

    for (unsigned int i = 0; i < 1000; i++) {
        start = rdtsc();
        int ret = crypto_scalarmult_curve13318_scalarmult(out, key, in);
        diff = rdtsc() - start - blank;
        assert(ret == 0);
        printf("%llu\n", diff);
    }

    return 0;
}
