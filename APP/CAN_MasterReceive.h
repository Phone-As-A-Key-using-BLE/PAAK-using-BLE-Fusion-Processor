/*
 * CAN_MasterReceive.h
 *
 *  Created on: Mar 18, 2024
 *      Author: Ali yasser
 *      Email : engaliyasser7@gmail.con
 *      Phone : 01154784667
 */


#ifndef CAN_MASTER_RECEIVE_H_
#define CAN_MASTER_RECEIVE_H_

#if (CAN_ANCHOR_ID != CAN_MASTER_NODE)
#include "flexcan_interrupt_transfer.h"
#include "app_digital_key_car_anchor_cs.h"
#include "app_localization.h"
#endif

#include "can.h"
#include "can_msg_types.h"
#include "CAN_App.h"

// Structures to store parsed data
typedef struct {
    uint8_t deviceId;
    uint16_t procNo;
    uint8_t distanceIntegerPart;
    uint16_t distanceDecimalPart;
    float dqiPercentage;
} CAN_tstrDistance;

typedef struct {
    CAN_tenumCommands command;
    uint8_t receiverId;
    uint8_t deviceId;
} CAN_tstrCommandData;

// Function declarations
void CAN_voidParseReceivedFrame(const tCANMsgObject* pRxMsg, const uint8_t* rxData);
CAN_tstrDistance CAN_structGetDistanceData(void);
CAN_tstrCommandData CAN_structGetLastCommandData(void);

#endif /* CAN_MASTER_RECEIVE_H_ */
