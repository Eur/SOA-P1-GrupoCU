
#include <stdbool.h>
#include <stdint.h>

#include "rng.h"

/*
 * Setting the seed as 0 to indicate that
 * the seed has not been set yet. This is important
 * because the random number generator should not
 * generate random numbers until it has been seeded.
 */
static uint32_t rng_seed = 0;

bool rng_xorshift32_seed(uint32_t seed) {

    /*
     * Even though a seed of 0 is invalid for this
     * algorithm, we still set the seed to 0, but
     * we return false to indicate the the seeding
     * was not successful.
     */
    rng_seed = seed;
    return seed != 0;
}

bool rng_xorshift32_get(uint32_t * out_random_number) {

    if (rng_seed == 0) {
        return false;
    }

    /*
     * Copying the seed to manipulate it according
     * to the XORShift 32 algorithm.
     */
    uint32_t x = rng_seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;


    rng_seed = x; 
    /*
     * Assigning the generated pseudorandom number to
     * the out parameter
     */
    *out_random_number = x;

    return true;
}
