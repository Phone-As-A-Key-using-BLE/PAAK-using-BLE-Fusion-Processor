/**
 * @file particle.h
 * @brief Defines the Particle structure and Particle Filter functions.
 *
 * This header file contains the definition of the Particle structure 
 * and the function prototypes for implementing the Particle Filter algorithm.
 * 
 * @author Anas Hamed, Mohamad Waleed and Ahmed Yousery
 * @date [10/2/2025] d/m/y
 */

 #ifndef PARTICLE_H_
 #define PARTICLE_H_
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <math.h>
 #include "measurement.h"
 #include "helper_function.h"
 #include "std_types.h"

 
 #define NUM_PARTICLES  50  /**< Number of particles used in the filter */
 
 /**
	* @struct Particle
	* @brief Represents a single particle in the Particle Filter.
	*/
 typedef struct Particle {
		 float32 x;            /**< X-coordinate of the particle */
		 float32 y;            /**< Y-coordinate of the particle */
		 float32 weight;       /**< Weight of the particle (probability) */
		 float32 velocityX;    /**< Velocity in the X direction (for Gaussian acceleration) */
		 float32 accelerationX;/**< Acceleration in the X direction (for Gaussian acceleration) */
		 float32 velocityY;    /**< Velocity in the Y direction (for Gaussian acceleration) */
		 float32 accelerationY;/**< Acceleration in the Y direction (for Gaussian acceleration) */
 } Particle;
 
 /**
	* @brief Initializes the particles with a given initial position and spread.
	*
	* @param particles Array of particles to be initialized.
	* @param x Initial x-coordinate.
	* @param y Initial y-coordinate.
	* @param spread Initial spread range for particle distribution.
	*/
 void Master_initialize_particles(Particle particles[NUM_PARTICLES], float64 x, float64 y, float64 spread);
 
 /**
	* @brief Updates the particles based on a new measurement.
	*
	* @param particles Array of particles to be updated.
	* @param newMeasurement New measurement used for update.
	*/
 void Master_update_particles(Particle particles[NUM_PARTICLES], Measurement_Type newMeasurement);
 
 /**
	* @brief Predicts the next state of the particles based on motion model.
	*
	* @param particles Array of particles to be predicted.
	*/
 void Master_prediction(Particle particles[NUM_PARTICLES]);
 
 /**
	* @brief Resamples the particles based on their weights.
	*
	* @param particles Array of particles to be resampled.
	*/
 void Master_resample(Particle particles[NUM_PARTICLES]);
 
 /**
	* @brief Estimates the final position based on the particles.
	*
	* @param particles Array of particles used for estimation.
	* @param coordinates Output array where estimated x and y coordinates are stored.
	*/
 void Master_estimate(Particle particles[NUM_PARTICLES], float32 coordinates[2]);
 
 #endif /* PARTICLE_H_ */
 
