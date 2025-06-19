/**
 * @file measurement.h
 * @brief Defines measurement structures used in the system.
 *
 * This header file contains the definitions of structures used 
 * for representing measurements in the system.
 * 
 * @author Ahmed Yousery
 * @date [3/2/2025] d/m/y
 */

 #ifndef MEASUREMENT_H_
 #define MEASUREMENT_H_
 
 #include "std_types.h"
 
 /**
  * @struct Measurement
  * @brief Represents a measurement with x and y coordinates.
  */
 typedef struct Measurement {
     float32 x;  /**< X-coordinate of the measurement */
     float32 y;  /**< Y-coordinate of the measurement */
 } Measurement;
 
 /**
  * @struct Measurement_Type
  * @brief Represents a measurement with an additional type field.
  */
 typedef struct Measurement_Type {
     float32 x;    /**< X-coordinate of the measurement */
     float32 y;    /**< Y-coordinate of the measurement */
     uint32 type;  /**< Type of measurement */
 } Measurement_Type;
 
 #endif /* MEASUREMENT_H_ */
 