
#include "Particle.h"



void Master_initialize_particles(Particle particles[NUM_PARTICLES], float64 x, float64 y, float64 spread) {
    srand_custom(0);  // Seed the random generator for variability
    int i;
    for (i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = x + ((simple_rand() % (int)(spread * 100)) / 100.0 - spread / 2);
        particles[i].y = y + ((simple_rand() % (int)(spread * 100)) / 100.0 - spread / 2);
        particles[i].weight = 1.0 / NUM_PARTICLES;  // Initially all particles have equal weight
    }
}

// void Master_initialize_particles(Particle particles[NUM_PARTICLES]) {
//
//     uint8_t mean_x = 0; // Assuming the car is centered at origin
//     uint8_t mean_y = 0; // Assuming the car is centered at origin
//     uint8_t stddev_x = BLE_RANGE; // Broader distribution across the car's width
//     uint8_t stddev_y = BLE_RANGE; // Broader distribution as device could be up to 50 meters away
//     uint32 i;
//     for ( i = 0; i < NUM_PARTICLES; i++) {
//         do {
//             particles[i].x = generate_normal_random(mean_x, stddev_x);
//             particles[i].y = generate_normal_random(mean_y, stddev_y);
//         }while (fabs(particles[i].x) > BLE_RANGE || fabs(particles[i].y) > BLE_RANGE);
//
//         particles[i].weight = 1.0 / NUM_PARTICLES; // Initialize weights evenly
//         particles[i].accelerationX =generate_normal_random(0, 1.5); //as the standard deviation value in prediction step
//         particles[i].accelerationY =generate_normal_random(0, 1.5); //as the standard deviation value in prediction step
//         particles[i].velocityX =0;
//         particles[i].velocityY =0;
//
//
// 	}
// }


void Master_update_particles(Particle particles[NUM_PARTICLES], Measurement_Type newMeasurement){

    float32 std_x; //init for standard deviation for particle x value
    float32 std_y; //init for standard deviation for particle y value
    float32 weight_sum = 0.0; //used for weight normalizing
    float32 wt = 1.0;
    uint32 i;
    for( i = 0; i < NUM_PARTICLES; i++){
        Particle *p = &particles[i];
        wt = 1.0;
 

    
        float32 power = 0;

        if(newMeasurement.type==0){
            power = 2.0;


        }else if(newMeasurement.type==1){
            power = 1.0;

        }else if(newMeasurement.type==2){
            power = 0.5;
        }
         float32 numerator = 1;
         float32 denominator = pow(sqrt(pow((newMeasurement.x - particles[i].x), 2) + pow((newMeasurement.y - particles[i].y), 2)),power);

        wt *= (numerator/denominator);
        p->weight = wt;
        weight_sum += wt;

    }

    //Normalize Weights
    // printf("total weight  = %f\n",weight_sum);

    // printf("p0 weight before = %f\n",particles[0].weight);

    for ( i = 0; i < NUM_PARTICLES; i++) {
        Particle *p = &particles[i];
        p->weight /= weight_sum;
    }
    

}


// void Master_prediction(Particle particles[NUM_PARTICLES]){

//     /////////////---------------Gaussian Random Acceleration---------------/////////////
//     float32 std_x = 1.5; //standard deviation for particle x value of Acc
//     float32 std_y = 1.5; //standard deviation for particle y value of Acc
//     float32 time = 0.1; //time to update the particle positions
//         for(uint32 i = 0; i < NUM_PARTICLES; ++i){
//             Particle *p = &particles[i]; // get address of particle to update

//             //X-direction
//             float32 errorX = generate_normal_random(0, std_x);
//             float32 new_acc_x = p->accelerationX + errorX;
//             float32 new_vel_x = p->velocityX + new_acc_x * time;  //Newton first equation
//             float32 new_x = p->x + new_vel_x * time + new_acc_x * time * time;  //Newton second equation without 1/2

//             //Y-direction
//             float32 errorY = generate_normal_random(0, std_y);
//             float32 new_acc_y = p->accelerationY + errorY;
//             float32 new_vel_y = p->velocityY + new_acc_y * time;
//             float32 new_y = p->y + new_vel_y * time + new_acc_y * time * time;


//             p->x = new_x;
//             p->y = new_y;
//             p->velocityX = new_vel_x;
//             p->velocityY = new_vel_y;
//             p->accelerationX = new_acc_x;
//             p->accelerationY = new_acc_y;

//         }



// }


void Master_prediction(Particle particles[NUM_PARTICLES]) {
    float32 std_x = 1.5; // Position noise standard deviation
    float32 std_y = 1.5; 
    float32 time = 0.1;  // 100ms time step
    uint32 i;
    for ( i = 0; i < NUM_PARTICLES; ++i) {
        Particle *p = &particles[i];

        // Random noise for position (more realistic than adding to acceleration)
        float32 error_x = generate_normal_random(0, std_x);
        float32 error_y = generate_normal_random(0, std_y);

        // Update acceleration with noise
        float32 new_acc_x = p->accelerationX + generate_normal_random(0, std_x / 10); 
        float32 new_acc_y = p->accelerationY + generate_normal_random(0, std_y / 10);

        // Update velocity using a midpoint method for better accuracy
        float32 new_vel_x = p->velocityX + (p->accelerationX + new_acc_x) / 2 * time;
        float32 new_vel_y = p->velocityY + (p->accelerationY + new_acc_y) / 2 * time;

        // Update position with velocity and noise
        float32 new_x = p->x + new_vel_x * time + 0.5 * new_acc_x * time * time + error_x;
        float32 new_y = p->y + new_vel_y * time + 0.5 * new_acc_y * time * time + error_y;

        // Clamp values to avoid unrealistic growth
        #define MAX_VELOCITY 5.0  
        #define MAX_ACCELERATION 2.0  

        p->x = new_x;
        p->y = new_y;
        p->velocityX = fminf(fmaxf(new_vel_x, -MAX_VELOCITY), MAX_VELOCITY);
        p->velocityY = fminf(fmaxf(new_vel_y, -MAX_VELOCITY), MAX_VELOCITY);
        p->accelerationX = fminf(fmaxf(new_acc_x, -MAX_ACCELERATION), MAX_ACCELERATION);
        p->accelerationY = fminf(fmaxf(new_acc_y, -MAX_ACCELERATION), MAX_ACCELERATION);
    }
}


void Master_resample(Particle particles[NUM_PARTICLES]) {
     Particle resampled_particles[NUM_PARTICLES];
     float32 weights[NUM_PARTICLES];
     float32 sum_of_weights = 0;
     uint32 i; uint32 j;
     // Calculate sum of weights
     for ( i = 0; i < NUM_PARTICLES; i++) {
         weights[i] = particles[i].weight;
         sum_of_weights += particles[i].weight; // equal 1
     }


     // Generate cumulative weights for sampling
     for ( i = 1; i < NUM_PARTICLES; i++) {
         weights[i] += weights[i - 1];
     }

     // Resample
     float32 pick;
     srand_custom(time(NULL)); // Seed the random number generator.
     for ( i = 0; i < NUM_PARTICLES; i++) {
          pick = (float32)simple_rand() / RAND_MAX * sum_of_weights;
         for ( j = 0; j < NUM_PARTICLES; j++) {
             if (pick <= weights[j]) {
                 resampled_particles[i] = particles[j];
                 break;
             }
         }
     }

     // Copy the resampled particles back
     for ( i = 0; i < NUM_PARTICLES; i++) {
         particles[i] = resampled_particles[i];
     }




    /***********NEW ONE*************** */
//    Particle resampled_particles[NUM_PARTICLES];
//    float32 weights[NUM_PARTICLES];
//    float32 sum_of_weights = 0;
//    uint32 i ;
//    // Calculate sum of weights
//    for ( i = 0; i < NUM_PARTICLES; i++) {
//        weights[i] = particles[i].weight;
//        sum_of_weights += particles[i].weight;
//    }
//
//    // Generate cumulative weights
//    for ( i = 1; i < NUM_PARTICLES; i++) {
//        weights[i] += weights[i - 1];
//    }
//
//    // Resample using Systematic Resampling (better than naive random selection)
//    float32 step = sum_of_weights / NUM_PARTICLES;
//    float32 pick = ((float32)simple_rand() / RAND_MAX) * step;
//    uint32 j = 0;
//
//    for ( i = 0; i < NUM_PARTICLES; i++) {
//        while (pick > weights[j]) {
//            j++;
//       }
//        resampled_particles[i] = particles[j];  // Copy selected particle
//        pick += step;
//    }
//
//    // Copy back resampled particles
//    for ( i = 0; i < NUM_PARTICLES; i++) {
//        particles[i] = resampled_particles[i];
//        particles[i].weight = 1.0 / NUM_PARTICLES;  // Reset weight
//    }
}



void Master_estimate(Particle particles[NUM_PARTICLES],float32 coordinates[2]){



    //Weighted Average
    float32 sum_weights_x = 0;
    float32 sum_weights_y = 0;
    float32 total_weight = 0;
    uint32 i;
    for ( i = 0; i < NUM_PARTICLES; i++) {
        sum_weights_x += particles[i].x * particles[i].weight;
        sum_weights_y += particles[i].y * particles[i].weight;
        total_weight += particles[i].weight;
    }

    Measurement estimated_position;
    if (total_weight > 0) {
        estimated_position.x = sum_weights_x / total_weight;
        estimated_position.y = sum_weights_y / total_weight;
    } else {
        // Handle the case where total weight is zero
        estimated_position.x = 0;
        estimated_position.y = 0;
    }

    coordinates[0] = estimated_position.x ;
    coordinates[1] = estimated_position.y ;
}
