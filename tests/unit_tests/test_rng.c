// System imports
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Local project imports
#include "rng.h"

// Unit test framework imports
#include "unit_test_infra.h"


TEST(test_rng_seed_and_get) {
    uint32_t random_number;

    // Test seeding with a valid seed
    ASSERT(rng_xorshift32_seed(12345) == true, "seeding with valid seed should succeed");

    // Test getting a random number after seeding
    ASSERT(rng_xorshift32_get(&random_number) == true, "getting random number after seeding should succeed");

    // Test seeding with an invalid seed (0)
    ASSERT(rng_xorshift32_seed(0) == false, "seeding with 0 should fail");

    // Test getting a random number without seeding
    ASSERT(rng_xorshift32_get(&random_number) == false, "getting random number without seeding should fail");

    return 0;
}

TEST(test_rng_multiple_gets) {
    uint32_t random_number1, random_number2;

    // Seed the RNG
    ASSERT(rng_xorshift32_seed(67890) == true, "seeding with valid seed should succeed");

    // Get the first random number
    ASSERT(rng_xorshift32_get(&random_number1) == true, "getting first random number should succeed");

    // Get the second random number
    ASSERT(rng_xorshift32_get(&random_number2) == true, "getting second random number should succeed");

    // Ensure that the two random numbers are different
    ASSERT(random_number1 == random_number2, "two consecutive random numbers with the same seed should be the same in XORShift32 algorithm");

    return 0;
}

int main(void) {

    printf("=== Random Number Generator xorshift32 — Unit tests ===\n\n");

    RUN(test_rng_seed_and_get);
    RUN(test_rng_multiple_gets);

    return 0;
}
