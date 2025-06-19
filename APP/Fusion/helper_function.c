/**
 * @file helper_function.c
 * @brief Implementation of helper functions including random number generation.
 *
 * This file contains functions for generating random numbers, including a linear congruential generator,
 * normal distribution generator using the Box-Muller transform, and a function for generating random numbers in a range.
 * 
 * @author Anas Hamed
 * @date [3/2/2025] d/m/y
 */

 #include "helper_function.h"

 
 /*********** GLOBAL VARIABLES ************/
 int FIRST_TIME = 1;  /**< Flag to initialize the random seed only once. */
 
 /**
  * @brief Custom seed for the random number generator.
  */
 static unsigned int rand_seed = 0;
 
 /**
  * @brief Generates a pseudo-random number using a Linear Congruential Generator (LCG).
  *
  * @return A pseudo-random unsigned integer.
  */
 unsigned int simple_rand(void) {
     rand_seed = (rand_seed * 1103515245 + 12345) % (1 << 31);
     return rand_seed % (RAND_MAX + 1);
 }
 
 /**
  * @brief Seeds the custom random number generator.
  *
  * @param seed The seed value for the random number generator.
  */
 void srand_custom(unsigned int seed) {
     rand_seed = seed;
 }
 
 /**
  * @brief Generates a normally distributed random number using the Box-Muller transform.
  *
  * The generated number falls within the range [mean - 3 * stddev, mean + 3 * stddev].
  *
  * @param mean The mean value of the distribution.
  * @param stddev The standard deviation of the distribution.
  * @return A normally distributed floating-point number.
  */
 float32 generate_normal_random(float32 mean, float32 stddev) {
     if (FIRST_TIME) {
         srand_custom(0);
         FIRST_TIME = 0;
     }
     float32 u, v, s;
     do {
         u = ((float32)simple_rand() / (RAND_MAX)) * 2 - 1;
         v = ((float32)simple_rand() / (RAND_MAX)) * 2 - 1;
         s = u * u + v * v;
     } while (s >= 1 || s == 0);
     
     float32 mul = sqrt(-2.0 * log(s) / s); // Box-Muller Transform
     return mean + stddev * u * mul;
 }
 
 /**
  * @brief Generates a random integer within a specified range.
  *
  * @param min The minimum value of the range (inclusive).
  * @param max The maximum value of the range (exclusive).
  * @return A randomly generated integer within the specified range.
  */
 uint32 rand_double_range(uint32 min, uint32 max) {
     return min + (simple_rand() / (RAND_MAX / (max - min)));
 }
 