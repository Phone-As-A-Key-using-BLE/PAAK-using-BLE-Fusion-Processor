/**
 * @file helper_function.h
 * @brief Header file containing helper functions for random number generation.
 *
 * This file provides function prototypes for generating random numbers,
 * normal distributions, and handling seeding for randomness in the system.
 *
 * @author Anas Hamed
 * @date [3/2/2025] d/m/y
 */

 #ifndef HELPER_FUNCTION_H_
 #define HELPER_FUNCTION_H_
 
 #include <float.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <string.h>
 #include <math.h>
 #include <time.h>
 #include "std_types.h"
 

 #ifndef M_PI
 /** @brief Definition of Pi if not available. */
 #define M_PI 3.14159265358979323846
 #endif
 
 /**
  * @brief Generates a pseudo-random number using a custom implementation.
  *
  * @return A randomly generated unsigned integer.
  */
 unsigned int simple_rand(void);
 
 /**
  * @brief Seeds the custom random number generator.
  *
  * @param seed The seed value to initialize the random number generator.
  */
 void srand_custom(unsigned int seed);
 
 /**
  * @brief Generates a normally distributed random number.
  *
  * @param mean The mean value of the normal distribution.
  * @param stddev The standard deviation of the normal distribution.
  * @return A random floating-point number following a normal distribution.
  */
 float32 generate_normal_random(float32 mean, float32 stddev);
 
 /**
  * @brief Generates a random integer within a specified range.
  *
  * @param min The minimum value (inclusive).
  * @param max The maximum value (inclusive).
  * @return A random integer within the given range.
  */
 uint32 rand_double_range(uint32 min, uint32 max);
 
 #endif /* HELPER_FUNCTION_H_ */
 