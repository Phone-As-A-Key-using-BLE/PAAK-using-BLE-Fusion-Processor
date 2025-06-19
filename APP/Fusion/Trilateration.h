/**
 * @file Trilateration.h
 * @brief Trilateration calculations for position estimation.
 * 
 * @author Anas Hamed, Mohamad Waleed and Ahmed Yousery
 * @date [5/2/2025] d/m/y
 */


#ifndef TRILATERATION_H_
#define TRILATERATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "measurement.h"
#include "std_types.h"

#define NUM_OF_ANCHORS 3 /**< Number of anchor nodes used for trilateration */

/**
 * @brief Structure representing an object detected by anchors.
 */
typedef struct  {
    float32 distance; /**< Measured distance from the anchor to the object in cm */
    uint8 mesgId;     /**< Message ID associated with the measurement */
} object_list;

/**
 * @brief Multiplies two matrices (A * B = result).
 *
 * @param A Pointer to the first matrix.
 * @param B Pointer to the second matrix.
 * @param result Pointer to the resulting matrix.
 * @param m Number of rows in matrix A.
 * @param n Number of columns in matrix A and rows in matrix B.
 * @param p Number of columns in matrix B.
 */
void matrix_multiply(float64 *A, float64 *B, float64 *result, sint32 m, sint32 n, sint32 p);

/**
 * @brief Computes the transpose of a matrix.
 *
 * @param A Pointer to the original matrix.
 * @param result Pointer to the transposed matrix.
 * @param n Number of rows in the original matrix.
 * @param m Number of columns in the original matrix.
 */
void matrix_transpose(float64 *A, float64 *result, sint32 n, sint32 m);

/**
 * @brief Computes the inverse of a square matrix.
 *
 * @param A Pointer to the original matrix.
 * @param result Pointer to the matrix where the inverse will be stored.
 * @return 1 if the inversion is successful, 0 if the matrix is singular.
 */
sint32 matrix_inverse(float64 *A, float64 *result);

/**
 * @brief Estimates the position of an object using trilateration.
 *
 * This function takes the distances from multiple anchors and estimates the
 * object's (x, y) position.
 *
 * @param arr_object_list Array of object_list structures containing distance measurements.
 * @return The estimated position as a Measurement_Type structure.
 */
Measurement_Type Master_trilaterate_position(object_list* arr_object_list);

#endif /* TRILATERATION_H_ */
