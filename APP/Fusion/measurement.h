// #include "can_msg_types.h"

#ifndef    MEASUREMENT_H_
#define    MEASUREMENT_H_
#include "std_types.h"
#define NUM_MEASUREMENTS 135


typedef struct Measurement {
    float32 x;
    float32 y;
} Measurement;

typedef struct Measurement_Type {
    float32 x;
    float32 y;
    uint32 type;
} Measurement_Type;

#endif /* MEASUREMENT_H_ */

