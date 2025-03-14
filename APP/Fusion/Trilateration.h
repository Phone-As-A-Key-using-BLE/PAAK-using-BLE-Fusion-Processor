#ifndef TRILATERATION_H_
#define TRILATERATION_H_

// #include "helper_function.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "measurement.h"
#include "std_types.h"
// #include "can_msg_types.h"

#define NUM_OF_ANCHORS 3


typedef struct object_list
{
	// uint8_t deviceId : 4;	/* Maximum 16 devices to be registered for one car */
	float32 distance ;		/* Maximum 2048 cm ~ 20 m that the BLE can reach (to be checked again in the open space)*/	
	uint8 mesgId;
}object_list;

//For RSSI and ToF
void matrix_multiply(float64 *A, float64 *B, float64 *result, sint32 m, sint32 n, sint32 p);
void matrix_transpose(float64 *A, float64 *result, sint32 n, sint32 m);
sint32 matrix_inverse(float64 *A, float64 *result);

Measurement_Type Master_trilaterate_position(object_list arr_object_list[NUM_OF_ANCHORS]);


#endif /* SENSOR_FUSION_H_ */
