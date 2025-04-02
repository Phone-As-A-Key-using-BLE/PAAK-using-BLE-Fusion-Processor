#include "helper_function.h"
#include"../../MCAL/SYSTICKTIMER/systicktimer.h"

/***********GLOBAL VAR************/
int FIRST_TIME = 1;


// Your custom RNG and srand_custom implementation
static unsigned int rand_seed = 0;  // Initial seed

unsigned int simple_rand(void) {
    rand_seed = (rand_seed * 1103515245 + 12345) % (1 << 31);  // LCG formula
    return rand_seed % (RAND_MAX + 1);  // Ensure range is 0 to RAND_MAX
}

void srand_custom(unsigned int seed) {
    rand_seed = seed;  // Initialize the seed
}


//the range [mean - 3 × stddev, mean + 3 × stddev]
float32 generate_normal_random(float32 mean, float32 stddev) {
    if(FIRST_TIME){
        srand_custom(0);
            FIRST_TIME = 0;
    }
    float32 u = ((float32)simple_rand() / (RAND_MAX)) * 2 - 1; // rand value [-1,1]
    float32 v = ((float32)simple_rand() / (RAND_MAX)) * 2 - 1; // rand value [-1,1]
    float32 s = u * u + v * v;
    while (s >= 1 || s == 0) {
        u = ((float32)simple_rand() / (RAND_MAX)) * 2 - 1;
        v = ((float32)simple_rand() / (RAND_MAX)) * 2 - 1;
        s = u * u + v * v;
    }
    float32 mul = sqrt(-2.0 * log(s) / s); // Box-Muller Transform
    return mean + stddev * u * mul;
}


uint32 rand_double_range(uint32 min, uint32 max) {
    return min + (simple_rand() / (RAND_MAX / (max - min)));
}



//memic third anchor disrance measuerement

double calculate_y(double d1, double d2) {
    return (d2*d2 - d1*d1) / 4.0;
}

void calculate_x(double d1, double d2, double* x1, double* x2) {
    double temp = (d2*d2 - d1*d1)/4.0 - 1;
    double sqrt_val = sqrt(d1*d1 - temp*temp);
    *x1 = 2.0 + sqrt_val;
    *x2 = 2.0 - sqrt(d1*d1 - pow(((d2*d2 - d1*d1)/4.0 - 1), 2));
}

double select_correct_x(double x1, double x2) {
    // Always select larger x for right-side constraint
    return (x1 > x2) ? x1 : x2;
}

double simulate_anchor3_distance(double x, double y) {
    return sqrt(pow(x + 2, 2) + y*y);
}


