
 #include "Trilateration.h"

 /**
  * @brief Multiplies two matrices A[m x n] and B[n x p] and stores the result in result[m x p].
  * 
  * @param A Pointer to the first matrix.
  * @param B Pointer to the second matrix.
  * @param result Pointer to store the multiplication result.
  * @param m Number of rows in matrix A.
  * @param n Number of columns in matrix A and rows in matrix B.
  * @param p Number of columns in matrix B.
  */
 void matrix_multiply(float64 *A, float64 *B, float64 *result, sint32 m, sint32 n, sint32 p) {
     sint32 i, j, k;
     for (i = 0; i < m; i++) {
         for (j = 0; j < p; j++) {
             float64 sum = 0;
             for (k = 0; k < n; k++) {
                 sum += A[i * n + k] * B[k * p + j];
             }
             result[i * p + j] = sum;
         }
     }
 }
 
 /**
  * @brief Transposes a matrix A[n x m] and stores the result in result[m x n].
  * 
  * @param A Pointer to the input matrix.
  * @param result Pointer to store the transposed matrix.
  * @param n Number of rows in A.
  * @param m Number of columns in A.
  */
 void matrix_transpose(float64 *A, float64 *result, sint32 n, sint32 m) {
     sint32 i, j;
     for (i = 0; i < n; i++) {
         for (j = 0; j < m; j++) {
             result[j * n + i] = A[i * m + j];
         }
     }
 }
 
 /**
  * @brief Inverts a 2x2 matrix.
  * 
  * @param A Pointer to the 2x2 matrix.
  * @param result Pointer to store the inverted matrix.
  * @return 0 if successful, -1 if the matrix is singular.
  */
 sint32 matrix_inverse(float64 *A, float64 *result) {
     float64 det = A[0] * A[3] - A[1] * A[2];
 
     if (det == 0) return -1; // Singular matrix, can't invert
 
     float64 invDet = 1.0 / det;
     result[0] = A[3] * invDet;
     result[1] = -A[1] * invDet;
     result[2] = -A[2] * invDet;
     result[3] = A[0] * invDet;
     return 0;
 }
 
 /**
  * @brief Solves the least squares problem for trilateration.
  * 
  * @param arr_object_list Array of object_list structures containing anchor data.
  * @return The calculated position as a Measurement_Type structure.
  */
 Measurement_Type Master_trilaterate_position(object_list* arr_object_list) {
     sint32 i;
     Measurement_Type devicePosition;
     
     for (i = 0; i < NUM_OF_ANCHORS; i++) {
         object_list *p = &arr_object_list[i];
 
         if (p->mesgId == 0) {
             devicePosition.x = -1;
             devicePosition.y = -1;
             devicePosition.type = -1;
             return devicePosition;
         }
     }
 
     arr_object_list[0].mesgId = 0;
     arr_object_list[1].mesgId = 0;
     arr_object_list[2].mesgId = 0;
 
     float64 A[NUM_OF_ANCHORS * 2], B[NUM_OF_ANCHORS], AT[2 * NUM_OF_ANCHORS], ATA[4], ATB[2], ATA_inv[4];
 
     /**
      * @brief Anchor coordinates used in the trilateration calculation.
      */
     Measurement anchorCoordinates[NUM_OF_ANCHORS] = {
            {-0.75, -0.4},   /**< First Node coordinates */
            {0.75, -0.4},     /**< Second anchor coordinates */
            {0, 0.9}     /**< Third anchor coordinates */
     };
 
     // Construct matrices A and B
     for (i = 0; i < NUM_OF_ANCHORS; i++) {
         A[i * 2] = 2 * (anchorCoordinates[i].x - anchorCoordinates[NUM_OF_ANCHORS - 1].x);
         A[i * 2 + 1] = 2 * (anchorCoordinates[i].y - anchorCoordinates[NUM_OF_ANCHORS - 1].y);
         B[i] = pow(arr_object_list[i].distance, 2) - pow(arr_object_list[NUM_OF_ANCHORS - 1].distance, 2)
                - pow(anchorCoordinates[i].x, 2) - pow(anchorCoordinates[i].y, 2)
                + pow(anchorCoordinates[NUM_OF_ANCHORS - 1].x, 2) + pow(anchorCoordinates[NUM_OF_ANCHORS - 1].y, 2);
     }
 
     // Compute transposition of A to AT
     matrix_transpose(A, AT, NUM_OF_ANCHORS, 2);
 
     // Compute ATA and ATB for normal equations
     matrix_multiply(AT, A, ATA, 2, NUM_OF_ANCHORS, 2);
     matrix_multiply(AT, B, ATB, 2, NUM_OF_ANCHORS, 1);
 
     if (matrix_inverse(ATA, ATA_inv) != 0) {
         devicePosition.x = -1;
         devicePosition.y = -1;
         devicePosition.type = -1;
         return devicePosition;
     }
 
     // Compute least squares solution
     float64 solution[2];
     matrix_multiply(ATA_inv, ATB, solution, 2, 2, 1);
 
     devicePosition.x = solution[0] * -1;
     devicePosition.y = solution[1] * -1;
     devicePosition.type = 0;
 
     return devicePosition;
 }
 