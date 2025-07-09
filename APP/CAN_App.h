#ifndef CAN_APP_H_
#define CAN_APP_H_

/* Anchor IDs */
#define CAN_MASTER_NODE     0
#define CAN_ANCHOR_1        1
#define CAN_ANCHOR_2        2
#define CAN_ANCHOR_3        3
#define CAN_ANCHOR_MAX      2

/* Configuration */
#define CAN_ANCHOR_ID       CAN_MASTER_NODE
#define CAN_PRIMARY_ANCHOR  CAN_ANCHOR_1

/* Base CAN ID */
#define CAN_BASE_ID 0x100

/* Incremental CAN IDs */
enum {
    CAN_ID_BONDING_DATA = CAN_BASE_ID,      // 0x100
    CAN_ID_OWNER_PK,
    CAN_ID_START_FRIEND_SHARING,
    CAN_ID_COMMANDS,                        // 0x101
    CAN_ID_DISTANCE_TDM_A1,                 // 0x102
    CAN_ID_DISTANCE_TDM_A2,                 // 0x103
    CAN_ID_DISTANCE_TDM_A3,                 // 0x104
    CAN_ID_STATUS_A1,                       // 0x105
    CAN_ID_STATUS_A2,                       // 0x106
    CAN_ID_STATUS_A3,                       // 0x107
    CAN_ID_WAKEUP_NOTIFICATION_A1,          // 0x108
    CAN_ID_WAKEUP_NOTIFICATION_A2,          // 0x109
    CAN_ID_WAKEUP_NOTIFICATION_A3,          // 0x10A
    CAN_ID_RSSI_A1,                         // 0x10B
    CAN_ID_RSSI_A2,                         // 0x10C
    CAN_ID_RSSI_A3,                         // 0x10D
    CAN_ID_CERTIFICATE,                     // 0x10E
    CAN_ID_VERIFIERS,                       // 0x10F
};

#endif /* CAN_APP_H_ */
