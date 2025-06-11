/*
 * CAN_App.h
 *
 *  Created on: Feb 19, 2025
 *      Author: mh_sm
 */

#ifndef CAN_APP_H_
#define CAN_APP_H_

/* Anchor IDs */
#define CAN_MASTER_NODE         0
#define CAN_ANCHOR_1            1
#define CAN_ANCHOR_2            2
#define CAN_ANCHOR_3            3
#define CAN_ANCHOR_MAX          2


/* Configuration */
#define CAN_ANCHOR_ID           CAN_MASTER_NODE
#define CAN_ENABLE_HANDOVER     0
#define CAN_PRIMARY_ANCHOR      CAN_ANCHOR_1

/* Common Message IDs */
#define CAN_BASE_ID                         0x100
#define CAN_ID_BONDING_DATA                 (CAN_BASE_ID + 0)
#define CAN_ID_HANDOVER                     (CAN_BASE_ID + 1)
#define CAN_ID_COMMANDS                     (CAN_BASE_ID + 2)

#if (CAN_ANCHOR_ID == CAN_PRIMARY_ANCHOR)
    #define CAN_ID_RANGING_TYPE             (CAN_BASE_ID + 19)
    #define CAN_ID_RANGING_TYPE_MASTER      (CAN_BASE_ID + 18)
#endif

/* Anchor-specific Message IDs */
#if (CAN_ANCHOR_ID == CAN_ANCHOR_1)
    #define CAN_ID_DISTANCE_TDM             (CAN_BASE_ID + 3)
    #define CAN_ID_DISTANCE_PE              (CAN_BASE_ID + 4)
    #define CAN_ID_PE_STATUS    	        (CAN_BASE_ID + 9)
    #define CAN_ID_WAKEUP_NOTIFICATION      (CAN_BASE_ID + 12)
    #define CAN_ID_RSSI                     (CAN_BASE_ID + 15)

#elif (CAN_ANCHOR_ID == CAN_ANCHOR_2)
    #define CAN_ID_DISTANCE_TDM             (CAN_BASE_ID + 5)
    #define CAN_ID_DISTANCE_PE              (CAN_BASE_ID + 6)
    #define CAN_ID_PE_STATUS    	        (CAN_BASE_ID + 10)
    #define CAN_ID_WAKEUP_NOTIFICATION      (CAN_BASE_ID + 13)
    #define CAN_ID_RSSI                     (CAN_BASE_ID + 16)


#elif (CAN_ANCHOR_ID == CAN_ANCHOR_3)
    #define CAN_ID_DISTANCE_TDM             (CAN_BASE_ID + 7)
    #define CAN_ID_DISTANCE_PE              (CAN_BASE_ID + 8)
    #define CAN_ID_PE_STATUS	            (CAN_BASE_ID + 11)
    #define CAN_ID_WAKEUP_NOTIFICATION      (CAN_BASE_ID + 14)
    #define CAN_ID_RSSI                     (CAN_BASE_ID + 17)

#elif (CAN_ANCHOR_ID == CAN_MASTER_NODE)
    #define CAN_ID_DISTANCE_TDM_A1          (CAN_BASE_ID + 3)
    #define CAN_ID_DISTANCE_PE_A1           (CAN_BASE_ID + 4)
    #define CAN_ID_DISTANCE_TDM_A2          (CAN_BASE_ID + 5)
    #define CAN_ID_DISTANCE_PE_A2           (CAN_BASE_ID + 6)
    #define CAN_ID_DISTANCE_TDM_A3          (CAN_BASE_ID + 7)
    #define CAN_ID_DISTANCE_PE_A3           (CAN_BASE_ID + 8)
    #define CAN_ID_STATUS_A1                (CAN_BASE_ID + 9)
    #define CAN_ID_STATUS_A2 	            (CAN_BASE_ID + 10)
    #define CAN_ID_STATUS_A3                (CAN_BASE_ID + 11)
    #define CAN_ID_WAKEUP_NOTIFICATION_A1   (CAN_BASE_ID + 12)
    #define CAN_ID_WAKEUP_NOTIFICATION_A2   (CAN_BASE_ID + 13)
    #define CAN_ID_WAKEUP_NOTIFICATION_A3   (CAN_BASE_ID + 14)
    #define CAN_ID_RSSI_A1                  (CAN_BASE_ID + 15)
    #define CAN_ID_RSSI_A2                  (CAN_BASE_ID + 16)
    #define CAN_ID_RSSI_A3                  (CAN_BASE_ID + 17)
    #define CAN_ID_RANGING_TYPE             (CAN_BASE_ID + 18)
    #define CAN_ID_RANGING_TYPE_A1          (CAN_BASE_ID + 19)
    #define CAN_ID_CERTIFICATE              (CAN_BASE_ID + 20)
    #define CAN_ID_VERIFIERS                (CAN_BASE_ID + 21)


#else
    #error "Invalid CAN_ANCHOR_ID"

#endif

#endif /* CAN_APP_H_ */
