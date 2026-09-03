#ifndef RNG_H
#define RNG_H

/**
 * @brief Seeds the random number generator.
 * @param seed The seed value.
 * @return true if the seed was set successfully, false otherwise.
 */
bool rng_xorshift32_seed(uint32_t seed);

/**
 * @brief Generates a random number based on the XORShift 32 algorithm.
 * @param[out] out_random_number Pointer to store the generated random number.
 * @return true if the random number was generated successfully, false otherwise.
 */
bool rng_xorshift32_get(uint32_t * out_random_number);

#endif // RNG_H
