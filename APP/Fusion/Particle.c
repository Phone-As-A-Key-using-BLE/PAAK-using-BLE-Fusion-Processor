/**
 * @file Particle.c
 * @brief Implements the Particle Filter functions for localization and tracking.
 * 
 * @author Anas Hamed, Mohamad Waleed and Ahmed Yousery
 * @date [10/2/2025] d/m/y
 */

 #include "Particle.h"

 /**
  * @brief Initializes the particles with a given initial position and spread.
  *
  * This function sets the initial position of particles based on a normal distribution
  * around a given (x, y) position and assigns them equal weights.
  *
  * @param particles Array of particles to be initialized.
  * @param x Initial x-coordinate.
  * @param y Initial y-coordinate.
  * @param spread Initial spread range for particle distribution.
  */
 void Master_initialize_particles(Particle particles[NUM_PARTICLES], float64 x, float64 y, float64 spread) {
     srand_custom(0);  // Seed the random generator for variability
     int i;
     for (i = 0; i < NUM_PARTICLES; i++) {
         particles[i].x = x + ((simple_rand() % (int)(spread * 100)) / 100.0 - spread / 2);
         particles[i].y = y + ((simple_rand() % (int)(spread * 100)) / 100.0 - spread / 2);
         particles[i].weight = 1.0 / NUM_PARTICLES;  // Initially all particles have equal weight
     }
 }
 
 /**
  * @brief Updates the particle weights based on the new measurement.
  *
  * This function adjusts the weight of each particle by calculating its probability 
  * relative to the given measurement, using an inverse distance weighting approach.
  *
  * @param particles Array of particles to be updated.
  * @param newMeasurement New measurement used for weight update.
  */
 void Master_update_particles(Particle particles[NUM_PARTICLES], Measurement_Type newMeasurement) {
     float32 weight_sum = 0.0;
     float32 wt = 1.0;
     uint32 i;
 
     for (i = 0; i < NUM_PARTICLES; i++) {
         Particle *p = &particles[i];
         wt = 1.0;
 
         float32 power = (newMeasurement.type == 0) ? 2.0 :
                         (newMeasurement.type == 1) ? 1.0 : 0.5;
 
         float32 distance = sqrt(pow((newMeasurement.x - p->x), 2) + pow((newMeasurement.y - p->y), 2));
         float32 denominator = pow(distance, power);
 
         if (denominator != 0) {
             wt *= (1 / denominator);
         }
 
         p->weight = wt;
         weight_sum += wt;
     }
 
     // Normalize the weights
     for (i = 0; i < NUM_PARTICLES; i++) {
         particles[i].weight /= weight_sum;
     }
 }
 
 /**
  * @brief Predicts the next state of the particles based on motion and noise.
  *
  * This function uses a motion model with random noise to update particle positions.
  *
  * @param particles Array of particles to be predicted.
  */
 void Master_prediction(Particle particles[NUM_PARTICLES]) {
     float32 std_x = 1.5;  /**< Standard deviation for x-axis noise */
     float32 std_y = 1.5;  /**< Standard deviation for y-axis noise */
     float32 time = 0.1;   /**< Time step (100ms) */
     uint32 i;
 
     for (i = 0; i < NUM_PARTICLES; ++i) {
         Particle *p = &particles[i];
 
         // Generate random noise for position updates
         float32 error_x = generate_normal_random(0, std_x);
         float32 error_y = generate_normal_random(0, std_y);
 
         // Update acceleration with noise
         float32 new_acc_x = p->accelerationX + generate_normal_random(0, std_x / 10);
         float32 new_acc_y = p->accelerationY + generate_normal_random(0, std_y / 10);
 
         // Update velocity using the midpoint method
         float32 new_vel_x = p->velocityX + (p->accelerationX + new_acc_x) / 2 * time;
         float32 new_vel_y = p->velocityY + (p->accelerationY + new_acc_y) / 2 * time;
 
         // Update position with velocity and noise
         p->x += new_vel_x * time + 0.5 * new_acc_x * time * time + error_x;
         p->y += new_vel_y * time + 0.5 * new_acc_y * time * time + error_y;
 
         // Clamp velocity and acceleration to avoid unrealistic values
         #define MAX_VELOCITY 5.0
         #define MAX_ACCELERATION 2.0
 
         p->velocityX = fminf(fmaxf(new_vel_x, -MAX_VELOCITY), MAX_VELOCITY);
         p->velocityY = fminf(fmaxf(new_vel_y, -MAX_VELOCITY), MAX_VELOCITY);
         p->accelerationX = fminf(fmaxf(new_acc_x, -MAX_ACCELERATION), MAX_ACCELERATION);
         p->accelerationY = fminf(fmaxf(new_acc_y, -MAX_ACCELERATION), MAX_ACCELERATION);
     }
 }
 
 /**
  * @brief Resamples particles using the Systematic Resampling method.
  *
  * This function replaces the current set of particles with a new set chosen based on their weights.
  *
  * @param particles Array of particles to be resampled.
  */
 void Master_resample(Particle particles[NUM_PARTICLES]) {
     Particle resampled_particles[NUM_PARTICLES];
     float32 weights[NUM_PARTICLES];
     float32 sum_of_weights = 0;
     uint32 i;
 
     // Compute cumulative sum of weights
     for (i = 0; i < NUM_PARTICLES; i++) {
         weights[i] = particles[i].weight;
         sum_of_weights += particles[i].weight;
     }
 
     for (i = 1; i < NUM_PARTICLES; i++) {
         weights[i] += weights[i - 1];
     }
 
     // Perform systematic resampling
     float32 step = sum_of_weights / NUM_PARTICLES;
     float32 pick = ((float32)simple_rand() / RAND_MAX) * step;
     uint32 j = 0;
 
     for (i = 0; i < NUM_PARTICLES; i++) {
         while (pick > weights[j]) {
             j++;
         }
         resampled_particles[i] = particles[j];
         pick += step;
     }
 
     // Copy resampled particles back and normalize weights
     for (i = 0; i < NUM_PARTICLES; i++) {
         particles[i] = resampled_particles[i];
         particles[i].weight = 1.0 / NUM_PARTICLES;
     }
 }
 
 /**
  * @brief Estimates the most probable position using weighted averaging.
  *
  * This function computes the estimated position based on the weighted average of all particles.
  *
  * @param particles Array of particles used for estimation.
  * @param coordinates Output array where estimated x and y coordinates are stored.
  */
 void Master_estimate(Particle particles[NUM_PARTICLES], float32 coordinates[2]) {
     float32 sum_weights_x = 0;
     float32 sum_weights_y = 0;
     float32 total_weight = 0;
     uint32 i;
 
     for (i = 0; i < NUM_PARTICLES; i++) {
         sum_weights_x += particles[i].x * particles[i].weight;
         sum_weights_y += particles[i].y * particles[i].weight;
         total_weight += particles[i].weight;
     }
 
     if (total_weight > 0) {
         coordinates[0] = sum_weights_x / total_weight;
         coordinates[1] = sum_weights_y / total_weight;
     } else {
         // Handle edge case where total weight is zero
         coordinates[0] = 0;
         coordinates[1] = 0;
     }
 }
 
