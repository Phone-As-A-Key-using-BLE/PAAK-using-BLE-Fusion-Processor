#ifndef STD_TYPES_H
#define STD_TYPES_H

/* Boolean Type */
typedef unsigned char boolean;
#define TRUE  (1u)
#define FALSE (0u)

/* Unsigned Integer Types */
typedef unsigned char       uint8;   /*  8-bit  */
typedef unsigned short      uint16;  /* 16-bit  */
typedef unsigned long       uint32;  /* 32-bit  */
typedef unsigned long long  uint64;  /* 64-bit  */

/* Signed Integer Types */
typedef signed char         sint8;   /*  8-bit  */
typedef signed short        sint16;  /* 16-bit  */
typedef signed long         sint32;  /* 32-bit  */
typedef signed long long    sint64;  /* 64-bit  */

/* Floating Point Types */
typedef float               float32;
typedef double              float64;

/* Standard Return Type */
typedef uint8 Std_ReturnType;
#define E_OK       (0u)
#define E_NOT_OK   (1u)

#endif /* STD_TYPES_H */
