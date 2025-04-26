#include "Trilateration.h"



// Function to multiply two matrices A[m x n] and B[n x p] to result[m x p]
void matrix_multiply(float64 *A, float64 *B, float64 *result, sint32 m, sint32 n, sint32 p) {
    sint32 i,j,k;
    for( i = 0; i < m; i++) {
        for(j = 0; j < p; j++) {
            float64 sum = 0;
            for(k = 0; k < n; k++) {
                sum += A[i*n + k] * B[k*p + j];
            }
            result[i*p + j] = sum;
        }
    }
}

// Function to transpose a matrix A[n x m] to result[m x n]
void matrix_transpose(float64 *A, float64 *result, sint32 n, sint32 m) {
    sint32 i,j;
    for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            result[j*n + i] = A[i*m + j];
        }
    }
}

// Function to invert a 2x2 matrix
sint32 matrix_inverse(float64 *A, float64 *result) {
    float64 det = A[0] * A[3] - A[1] * A[2];
//    printf("A[0]=%f, A[3]=%f, A[1]=%f, A[2]=%f, det = %f\n", A[0], A[3], A[1], A[2], det);

    if(det == 0) return -1; // Singular matrix, can't invert

    float64 invDet = 1.0 / det;
    result[0] = A[3] * invDet;
    result[1] = -A[1] * invDet;
    result[2] = -A[2] * invDet;
    result[3] = A[0] * invDet;
    return 0;
}

// Function to solve the least squares problem for trilateration
Measurement_Type Master_trilaterate_position(object_list arr_object_list[NUM_OF_ANCHORS]) {
    sint32 i;
	Measurement_Type devicePosition;
    for( i=0;i<NUM_OF_ANCHORS; i++){
        object_list *p = &arr_object_list[i];

        if(p->mesgId == 0 ){
            devicePosition.x=-1;
            devicePosition.y=-1;
            devicePosition.type=-1;
            // SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\r\ntrilate has anchor %d with mesgId == 0\r\n", i);
            return devicePosition;
        }
    }
    // SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\r\ntrilate has No anchor with mesgId == 0\r\n");
    arr_object_list[0].mesgId = 0;
	arr_object_list[1].mesgId = 0;
	arr_object_list[2].mesgId = 0;

    float64 A[NUM_OF_ANCHORS*2], B[NUM_OF_ANCHORS], AT[2*NUM_OF_ANCHORS], ATA[4], ATB[2], ATA_inv[4];

    // Anchor coordinates 140cm x 80cm
    Measurement anchorCoordinates[NUM_OF_ANCHORS] = {
        {0.7,-0.4},    // Anchor1
        {-0.7,0},   // Anchor2
        {0.7,0.4}   // Anchor3
    };
//(x-x1)^2+(y-y1^2)=r^2
    // Construct matrices A and B

    for(i = 0; i < NUM_OF_ANCHORS; i++) {
        A[i*2] = 2 * (anchorCoordinates[i].x - anchorCoordinates[NUM_OF_ANCHORS-1].x);
        A[i*2 + 1] = 2 * (anchorCoordinates[i].y - anchorCoordinates[NUM_OF_ANCHORS-1].y);
        B[i] = pow(arr_object_list[i].distance, 2) - pow(arr_object_list[NUM_OF_ANCHORS-1].distance, 2)
               - pow(anchorCoordinates[i].x, 2) - pow(anchorCoordinates[i].y, 2)
               + pow(anchorCoordinates[NUM_OF_ANCHORS-1].x, 2) + pow(anchorCoordinates[NUM_OF_ANCHORS-1].y, 2);
    }

    // Compute transposition of A to AT
    matrix_transpose(A, AT, NUM_OF_ANCHORS, 2);

    // Compute ATA and ATB for normal equations
    matrix_multiply(AT, A, ATA, 2, NUM_OF_ANCHORS, 2);
    matrix_multiply(AT, B, ATB, 2, NUM_OF_ANCHORS, 1);

    if(matrix_inverse(ATA, ATA_inv) != 0) {

            devicePosition.x=-1;
            devicePosition.y=-1;
            devicePosition.type=-1;
            return devicePosition;
    }

    // Compute least squares solution
    float64 solution[2];
    matrix_multiply(ATA_inv, ATB, solution, 2, 2, 1);

    devicePosition.x=solution[0]*-1;
    devicePosition.y=solution[1]*-1;
    devicePosition.type=0;

    return devicePosition;
}

