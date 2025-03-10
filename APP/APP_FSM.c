/*
 * APP_FSM.c
 *
 *  Created on: Feb 27, 2025
 *      Author: 
 */
#include <string.h>
#include <stdio.h>
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/CAN_MasterReceive.h"
#include "APP/CAN_App.h"
#include "APP/APP_FSM.h"

char buffer[128]; // for debugging

APP_tenuStates currentState = STATE_IDLE;

uint8_t Global_PEDone[CAN_ANCHOR_MAX]={0,0,0};

uint8_t Global_u8CurrentAnchor=0;
uint8_t Global_u8SuccessPE=0; // Success Passive Entry
uint8_t Global_u8FirstTime=1; 
uint8_t Global_u8PERetryCount=0;


void APP_voidFSMHandler(APP_tenuEvents Copy_structEvent)
{
    switch (currentState)
    {
    // **IDLE STATE**: System is waiting for user input
    case STATE_IDLE:
        if (Copy_structEvent == EVENT_OWNER_PAIRING_BUTTON_PRESSED)
        {
            // Transition to owner pairing process
            currentState = STATE_START_OWNER_PAIRING;
            UART_SendMessage("\n[INFO] Owner pairing button pressed. Initiating owner pairing process...\n");
            APP_voidFSMHandler(EVENT_SEND_OWNER_PAIRING_COMMAND);
        }
        break;

    // **START OWNER PAIRING**: Send pairing command to primary anchor
    case STATE_START_OWNER_PAIRING:
        if (Copy_structEvent == EVENT_SEND_OWNER_PAIRING_COMMAND)
        {
            currentState = STATE_WAITING_FOR_BONDING_DATA;
            UART_SendMessage("\n[INFO] Sending owner pairing command to primary anchor...\n");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_OWNER_PAIRING, CAN_PRIMARY_ANCHOR, 0);
        }
        break;

    // **WAITING FOR BONDING DATA**: Wait for bonding data from the primary anchor
    case STATE_WAITING_FOR_BONDING_DATA:
        if (Copy_structEvent == EVENT_BONDING_DATA_RECEIVED)
        {
            currentState = STATE_WAITING_FOR_PRIMARY_WAKEUP;
            UART_SendMessage("\n[INFO] Bonding data received. Waiting for wake-up signal from primary anchor...\n");
            CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_PRIMARY_ANCHOR, 0);
        }
        break;

    // **WAITING FOR PRIMARY WAKEUP**: System waits for a wake-up event
    case STATE_WAITING_FOR_PRIMARY_WAKEUP:
        if (Copy_structEvent == EVENT_PRIMARY_WAKEUP_RECEIVED)
        {
            currentState = STATE_PRIMARY_PE;
            UART_SendMessage("\n[INFO] Wake-up signal received. Triggering Passive Entry (PE) on primary anchor...\n");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, 0);
        }
        break;

    // **PRIMARY PASSIVE ENTRY (PE)**: Handle success or failure of PE
    case STATE_PRIMARY_PE:
        if (Copy_structEvent == EVENT_PRIMARY_PE_SUCCESSFUL)
        {
            currentState = STATE_PRIMARY_TDM;
            UART_SendMessage("\n[SUCCESS] Primary Passive Entry successful. Proceeding to Trigger Distance Measurement (TDM)...\n");
            Global_PEDone[CAN_PRIMARY_ANCHOR] = 1;
        }
        else if (Copy_structEvent == EVENT_PRIMARY_PE_FAILED)
        {
            currentState = STATE_WAITING_FOR_BONDING_DATA;
            UART_SendMessage("\n[ERROR] Primary Passive Entry failed. Reverting to bonding data state to reset and trigger PE again...\n");
        }
        break;

    // **PRIMARY TDM (Distance Measurement)**: Evaluate wake-up decision
    case STATE_PRIMARY_TDM:
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
        {
            currentState = STATE_WAKEUP_DECISION_MAKING;
            UART_SendMessage("\n[INFO] Distance measurement completed. Evaluating wake-up decision...\n");

            if (!Global_u8FirstTime)
            {
                // Trigger distance measurement when the device is out of range
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, CAN_PRIMARY_ANCHOR);
                Global_u8FirstTime = 0;
            }

            uint8_t Loc_u8Distance = CAN_structGetDistanceData().distanceIntegerPart;
            if (Loc_u8Distance <= APP_DISTANCE_TRIGGER_THRESHOLD)
            {
                APP_voidFSMHandler(EVENT_DISTANCE_BELOW_THRESHOLD);
            }
            else
            {
                APP_voidFSMHandler(EVENT_DISTANCE_ABOVE_THRESHOLD);
            }
        }
        break;

    // **WAKE-UP DECISION MAKING**: Decide next action based on distance
    case STATE_WAKEUP_DECISION_MAKING:
        if (Copy_structEvent == EVENT_DISTANCE_ABOVE_THRESHOLD)
        {
            currentState = STATE_PRIMARY_TDM;
            UART_SendMessage("\n[INFO] Distance above threshold. Retrying distance measurement...\n");
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
        }
        else if (Copy_structEvent == EVENT_DISTANCE_BELOW_THRESHOLD)
        {
            currentState = STATE_SECONDARY_PE;
            UART_SendMessage("\n[INFO] Device is in range. Initiating Secondary Passive Entry (PE)...\n");
            Global_u8FirstTime = 1;
            APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE);
        }
        break;

    // **SECONDARY PASSIVE ENTRY (PE)**: Handle multiple anchors for PE
    case STATE_SECONDARY_PE:
        if (Copy_structEvent == EVENT_DEVICE_IN_RANGE && Global_u8FirstTime)
        {
            // Initialize PE process
            Global_u8CurrentAnchor = 0;
            Global_u8SuccessPE = 0;
            Global_u8FirstTime = 0;
            Global_u8PERetryCount = 0;
        }

        // Send Passive Entry command
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
        snprintf(buffer, sizeof(buffer), "\n[INFO] Sending Passive Entry command to anchor %d...\n", Global_u8CurrentAnchor);
        UART_SendMessage(buffer);

        // Treat PE success as distance received
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
        {
            Global_u8SuccessPE++;
            Global_PEDone[Global_u8CurrentAnchor] = 1;

            snprintf(buffer, sizeof(buffer), "\n[SUCCESS] Distance received from anchor %d (Passive Entry assumed success)...\n", Global_u8CurrentAnchor);
            UART_SendMessage(buffer);

            Global_u8CurrentAnchor++;
            Global_u8PERetryCount = 0;

            if (Global_u8CurrentAnchor >= CAN_ANCHOR_MAX)
            {
                Global_u8CurrentAnchor = 0;

                if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS)
                {
                    currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
                    UART_SendMessage("\n[INFO] Minimum distance readings met. Proceeding to vehicle-level decision making...\n");
                    APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                    break;
                }
            }

            while (Global_u8CurrentAnchor < CAN_ANCHOR_MAX && Global_PEDone[Global_u8CurrentAnchor])
            {
                Global_u8CurrentAnchor++;
            }

            if (Global_u8CurrentAnchor < CAN_ANCHOR_MAX)
            {
                snprintf(buffer, sizeof(buffer), "\n[INFO] Moving to next available anchor %d...\n", Global_u8CurrentAnchor);
                UART_SendMessage(buffer);
                // Send Passive Entry command
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
            }
        }
        // Handle PE Failure
        else if (Copy_structEvent == EVENT_SECONDARY_PE_FAILED)
        {
            if (Global_u8PERetryCount < MAX_PE_RETRIES)
            {
                Global_u8PERetryCount++;
                snprintf(buffer, sizeof(buffer), "\n[WARNING] Passive Entry failed on anchor %d. Retrying attempt %d/%d...\n",
                         Global_u8CurrentAnchor, Global_u8PERetryCount, MAX_PE_RETRIES);
                UART_SendMessage(buffer);
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
            }
            else
            {
                snprintf(buffer, sizeof(buffer), "\n[ERROR] Passive Entry failed on anchor %d after maximum retries. Moving to next anchor...\n", Global_u8CurrentAnchor);
                UART_SendMessage(buffer);
                Global_u8CurrentAnchor++;
                Global_u8PERetryCount = 0;
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
            }
        }
        break;

    // **VEHICLE LEVEL DECISION MAKING**: Evaluate final position of the device
    case STATE_VEHICLE_LEVEL_DECISION_MAKING:
        UART_SendMessage("\n[INFO] Executing vehicle-level decision-making process...\n");
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
        {
            uint8_t Loc_u8Distance = CAN_structGetDistanceData().distanceIntegerPart;
            if (Loc_u8Distance <= APP_DISTANCE_TRIGGER_THRESHOLD)
            {
                UART_SendMessage("\n[INFO] Device is in range, start fusion algo...\n");
                currentState = STATE_FUSION_ALGO;
                APP_voidFSMHandler(EVENT_DISTANCE_BELOW_THRESHOLD);
            }
            else
            {
                UART_SendMessage("\n[INFO] Device is out of range, return to Wake up decision making...\n");
                currentState = STATE_WAKEUP_DECISION_MAKING;
                APP_voidFSMHandler(EVENT_DISTANCE_ABOVE_THRESHOLD);
            }
        }
        break;

    // **FUSION ALGORITHM STATE**: Execute sensor fusion
    case STATE_FUSION_ALGO:
        UART_SendMessage("\n[INFO] Fusion Algorithm is done, returning to vehicle decision-making...\n");
        currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
        APP_voidFSMHandler(EVENT_FINAL_DISTANCE);
        break;

    default:
        UART_SendMessage("[ERROR] Unknown system state encountered.\n");
        break;
    }
}



// /*
//  * APP_FSM.c
//  *
//  *  Created on: Feb 27, 2025
//  *      Author: mh_sm
//  */
// #include <stdio.h>
// #include "UART/uart.h"
// #include "APP/can_msg_types.h"
// #include "APP/CAN_Send.h"
// #include "APP/CAN_MasterReceive.h"
// #include "APP/CAN_App.h"
// #include "APP/APP_FSM.h"

// static APP_tenuStates currentState = STATE_IDLE;

// uint8_t Global_PEDone[CAN_ANCHOR_MAX]={0,0,0};

// uint8_t Global_u8CurrentAnchor=0;
// uint8_t Global_u8SuccessPE=0; // Success Passive Entry
// uint8_t Global_u8FirstTime=1; // Success Passive Entry


// void APP_voidFSMHandler(APP_tenuEvents Copy_structEvent)
// {
//     switch (currentState)
//     {
//     case STATE_IDLE:
//         if (Copy_structEvent == EVENT_OWNER_PAIRING_BUTTON_PRESSED)
//         {
//             currentState = STATE_START_OWNER_PAIRING;
//             UART_SendMessage("\nTransition to STATE_START_OWNER_PAIRING\n");
//             APP_voidFSMHandler(EVENT_SEND_OWNER_PAIRING_COMMAND);
//         }
//         break;

//     case STATE_START_OWNER_PAIRING:
//         if (Copy_structEvent == EVENT_SEND_OWNER_PAIRING_COMMAND)
//         {
//             currentState = STATE_WAITING_FOR_BONDING_DATA;
//             UART_SendMessage("\nTransition to STATE_WAITING_FOR_BONDING_DATA\n");
//             CAN_voidSendCommand(CAN_COMMAND_TRIGGER_OWNER_PAIRING, CAN_PRIMARY_ANCHOR, 0);
//         }
//         break;

//     case STATE_WAITING_FOR_BONDING_DATA:
//         if (Copy_structEvent == EVENT_BONDING_DATA_RECEIVED)
//         { // Triggered from CAN_MasterReceive
//             currentState = STATE_WAITING_FOR_PRIMARY_WAKEUP;
//             UART_SendMessage("\nTransition to STATE_WAIT_WAKEUP\n");
//             CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_PRIMARY_ANCHOR, 0);
//         }
//         break;

//     case STATE_WAITING_FOR_PRIMARY_WAKEUP:
//         if (Copy_structEvent == EVENT_BONDING_DATA_RECEIVED)
//         { // Triggered from CAN_MasterReceive
//             currentState = STATE_PRIMARY_PE;
//             UART_SendMessage("\nTransition to STATE_PRIMARY_PE\n");
//             CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, 0);
//         }
//         break;

//     case STATE_PRIMARY_PE:
//         if (Copy_structEvent == EVENT_PRIMARY_PE_SUCCESSFUL)
//         { // Triggered from CAN_MasterReceive
//             currentState = STATE_PRIMARY_TDM;
//             UART_SendMessage("\nTransition to STATE_PRIMARY_TDM\n");
//             Global_PEDone[CAN_PRIMARY_ANCHOR] = 1;
//         }
//         else if (Copy_structEvent == EVENT_PRIMARY_PE_FAILED)
//         { // Triggered from CAN_MasterReceive
//             currentState = STATE_WAITING_FOR_BONDING_DATA;
//             UART_SendMessage("\nTransition to STATE_WAITING_FOR_BONDING_DATA\n");
//         }
//         break;

//     case STATE_PRIMARY_TDM:
//         if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
//         { // Triggered from CAN_MasterReceive
//             currentState = STATE_WAKEUP_DECISION_MAKING;
//             UART_SendMessage("\nTransition to STATE_WAKEUP_DECISION_MAKING\n");
//             CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, CAN_PRIMARY_ANCHOR); // First time the distance will be sent automatically but then when distance is above threshold this will be needed
//             uint8_t Loc_u8Distance = CAN_structGetDistanceData().distanceIntegerPart;
//             if (Loc_u8Distance <= APP_DISTANCE_TRIGGER_THRESHOLD)
//             {
//                 APP_voidFSMHandler(EVENT_DISTANCE_BELOW_THRESHOLD);
//             }
//             else
//             {
//                 APP_voidFSMHandler(EVENT_DISTANCE_ABOVE_THRESHOLD);
//             }
//         }
//         break;

//     case STATE_WAKEUP_DECISION_MAKING:
//         if (Copy_structEvent == EVENT_DISTANCE_ABOVE_THRESHOLD)
//         {
//             currentState = STATE_PRIMARY_TDM;
//             UART_SendMessage("\nTransition back to STATE_PRIMARY_TDM\n");
//             APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
//         }
//         else if (Copy_structEvent == EVENT_DISTANCE_BELOW_THRESHOLD)
//         {
//             currentState = STATE_SECONDARY_PE;
//             UART_SendMessage("\nTransition to STATE_SECONDARY_PE\n");
//             APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE);
//             Global_u8FirstTime = 1;
//         }
//         break;

//         //        case STATE_SECONDARY_PE: // Loop all anchors to make each one do passive entry
//         //
//         //            Global_u8CurrentAnchor++;
//         //            if (Copy_structEvent== EVENT_DEVICE_IN_RANGE){
//         //                // This case will be entered first time only and it will send CAN_COMMAND_TRIGGER_PASSIVE_ENTRY
//         //            }
//         //            else if(Copy_structEvent == EVENT_SECONDARY_PE_SUCCESSFUL){ // Triggered from CAN_MasterReceive
//         //                Global_u8SuccessPE++;
//         //                Global_PEDone[Global_u8CurrentAnchor-1] = 1;
//         //                if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS && Global_u8CurrentAnchor > CAN_ANCHOR_MAX) {
//         //                    currentState = STATE_SECONDARY_TDM;
//         //                    UART_SendMessage("Transition to STATE_SECONDARY_TDM");
//         //                } else {
//         //                    UART_SendMessage("Staying in STATE_SECONDARY_PE");
//         //                }
//         //            }
//         //            else if(Copy_structEvent == EVENT_SECONDARY_PE_FAILED){     // Triggered from CAN_MasterReceive
//         //                if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS && Global_u8CurrentAnchor > CAN_ANCHOR_MAX) {
//         //                    currentState = STATE_SECONDARY_TDM;
//         //                    UART_SendMessage("Transition to STATE_SECONDARY_TDM");
//         //                } else if(Global_u8SuccessPE < APP_MINUMUM_DISTANCE_READINGS && Global_u8CurrentAnchor > CAN_ANCHOR_MAX){
//         //                    Global_u8CurrentAnchor = 1;
//         //                    Global_u8SuccessPE = 0;
//         //                } else {
//         //                    UART_SendMessage("Staying in STATE_SECONDARY_PE");
//         //                }
//         //            }
//         //            else{
//         //
//         //            }
//         //            if (Global_u8CurrentAnchor < CAN_ANCHOR_MAX) {
//         //                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
//         //            }
//         //            break;
//     case STATE_SECONDARY_PE:
//         if(Copy_structEvent != EVENT_RECEIVE_DISTANCE){
//             if (Copy_structEvent == EVENT_DEVICE_IN_RANGE && Global_u8FirstTime)
//             {
//                 // First time entering, reset tracking variables
//                 Global_u8CurrentAnchor = 0;
//                 Global_u8SuccessPE = 1;
//                 Global_u8FirstTime = 0;
//             }

//             // If PE was successful, mark anchor as done
//             if (Copy_structEvent == EVENT_SECONDARY_PE_SUCCESSFUL)
//             {
//                 CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, 0);
//                 Global_u8SuccessPE++;
//                 Global_PEDone[Global_u8CurrentAnchor] = 1;

//                 // Transition to waiting for wake-up event
//                 currentState = STATE_WAITING_FOR_SECONDARY_WAKEUP;
//                 UART_SendMessage("\nWaiting for wake-up notification...\n");
//                 break;
//             }
//         }
//     break;

//     case STATE_WAITING_FOR_SECONDARY_WAKEUP:
//         if (Copy_structEvent == EVENT_SECONDARY_WAKEUP_RECEIVED)
//         {
//             UART_SendMessage("\nWake-up received. Resuming...\n");

//             // Move to the next anchor
//             Global_u8CurrentAnchor++;

//             // Always check all anchors, even if we already reached minimum success
//             if (Global_u8CurrentAnchor >= CAN_ANCHOR_MAX)
//             {
//                 Global_u8CurrentAnchor = 0; // Restart from the first anchor

//                 // Only transition after all anchors have been checked at least once
//                 if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS)
//                 {
//                     currentState = STATE_SECONDARY_TDM;
//                     UART_SendMessage("\nTransition to STATE_SECONDARY_TDM\n");
//                     Global_u8SuccessPE = 1;
//                     APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
//                     break;
//                 }
//             }

//             // Skip already checked anchors
//             while (Global_u8CurrentAnchor < CAN_ANCHOR_MAX && Global_PEDone[Global_u8CurrentAnchor])
//             {
//                 Global_u8CurrentAnchor++;
//             }

//             // If we still have an anchor left to check, send PE command
//             if (Global_u8CurrentAnchor < CAN_ANCHOR_MAX)
//             {
//                 CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
//                 currentState = STATE_SECONDARY_PE; // Return to normal PE process
//             }
//         }
//     break;


//     case STATE_SECONDARY_TDM:
//         if (Global_u8FirstTime)
//         {
//             // First time entering, reset tracking variables
//             Global_u8CurrentAnchor = 0;
//             Global_u8SuccessPE = 1;
//             Global_u8FirstTime = 0;
//         }
//         if (Copy_structEvent == EVENT_RECEIVE_DISTANCE) {
//             // Find the next anchor that has completed PE but not yet measured
//             while (Global_u8SuccessPE < CAN_ANCHOR_MAX && !Global_PEDone[Global_u8SuccessPE]) {
//                 Global_u8SuccessPE++;  // Move to the next anchor
//             }

//             if (Global_u8SuccessPE < CAN_ANCHOR_MAX) {
//                 // Send distance measurement command for the current successful anchor
//                 CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8SuccessPE, 0);
//                 Global_u8SuccessPE++; // Move to the next one for the next event trigger
//             } else {
//                 // All successful anchors measured, transition to next state
//                 currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
//                 UART_SendMessage("\nTransition to STATE_VEHICLE_LEVEL_DECISION_MAKING\n");
//             }
//         }
//         break;

//     case STATE_VEHICLE_LEVEL_DECISION_MAKING:
//         if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
//         {
//             currentState = STATE_FUSION_ALGO;
//             UART_SendMessage("\nTransition to STATE_FUSION_ALGO\n");
//         }
//         else if (Copy_structEvent == EVENT_DISTANCE_ABOVE_THRESHOLD)
//         {
//             currentState = STATE_WAKEUP_DECISION_MAKING;
//             UART_SendMessage("\nTransition to STATE_WAKEUP_DECISION_MAKING\n");
//         }
//         break;

//     case STATE_FUSION_ALGO:
//         if (Copy_structEvent == EVENT_FINAL_DISTANCE)
//         {
//             currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
//             UART_SendMessage("\nTransition back to STATE_VEHICLE_LEVEL_DECISION_MAKING\n");
//         }
//         break;

//     default:
//         UART_SendMessage("Unknown state");
//         break;
//     }
// }
