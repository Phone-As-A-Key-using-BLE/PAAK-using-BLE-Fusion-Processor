#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "measurement.h"
#include "helper_function.h"
#include "std_types.h"
#include"systicktimer.h"


#define NUM_PARTICLES  50
 #define BLE_RANGE 5
 #define PLOT_GRAPH 0

typedef struct Particle{

	float32 x;
	float32 y;
	float32 weight;
	float32 velocityX;     //if considering Gaussian Random Acceleration
	float32 accelerationX; //if considering Gaussian Random Acceleration
	float32 velocityY;     //if considering Gaussian Random Acceleration
	float32 accelerationY; //if considering Gaussian Random Acceleration
}Particle;


void Master_initialize_particles(Particle particles[NUM_PARTICLES], float64 x, float64 y, float64 spread);
//void Master_initialize_particles(Particle particles[NUM_PARTICLES]);

void Master_update_particles(Particle particles[NUM_PARTICLES], Measurement_Type newMeasurement);

void Master_prediction(Particle particles[NUM_PARTICLES]);

void Master_resample(Particle particles[NUM_PARTICLES]);

void Master_estimate(Particle particles[NUM_PARTICLES],float32 coordinates[2]);

#endif
