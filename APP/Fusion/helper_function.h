
#ifndef     HELPER_FUNCTION_H_
#define     HELPER_FUNCTION_H_

#include  <float.h>
#include  <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>    
#include "std_types.h"
#define NUM_ITERATIONS 10

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

unsigned int simple_rand(void);
void srand_custom(unsigned int seed) ;
float32 generate_normal_random(float32 mean, float32 stddev);
uint32 rand_double_range(uint32 min, uint32 max);


double calculate_y(double d1, double d2);


void calculate_x(double d1, double d2, double* x1, double* x2);

double select_correct_x(double x1, double x2);


double simulate_anchor3_distance(double x, double y);

#endif /* HELPER_FUNCTION_H_ */
