/*
 * APP_FSM.c
 *
 *  Created on: Feb 27, 2025
 *      Author: mh_sm
 */
#include <stdio.h>
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/CAN_MasterReceive.h"
#include "APP/CAN_App.h"
#include "APP/APP_FSM.h"

static APP_tenuStates currentState = STATE_IDLE;

bool Global_PEDone[CAN_ANCHOR_MAX];

uint8_t Global_u8CurrentAnchor;
uint8_t Global_u8SuccessPE; // Success Passive Entry

void APP_voidFSMHandler(APP_tenuEvents Copy_structEvent)
{
    switch (currentState)
    {
    case STATE_IDLE:
        if (Copy_structEvent == EVENT_OWNER_PAIRING_BUTTON_PRESSED)
        {
            currentState = STATE_START_OWNER_PAIRING;
            UART_SendMessage("Transition to STATE_START_OWNER_PAIRING");
            APP_voidFSMHandler(EVENT_SEND_OWNER_PAIRING_COMMAND);
        }
        break;

    case STATE_START_OWNER_PAIRING:
        if (Copy_structEvent == EVENT_SEND_OWNER_PAIRING_COMMAND)
        {
            currentState = STATE_WAITING_FOR_BONDING_DATA;
            UART_SendMessage("Transition to STATE_WAITING_FOR_BONDING_DATA");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_OWNER_PAIRING, CAN_PRIMARY_ANCHOR, 0);
        }
        break;

    case STATE_WAITING_FOR_BONDING_DATA:
        if (Copy_structEvent == EVENT_BONDING_DATA_RECEIVED)
        { // Triggered from CAN_MasterReceive
            currentState = STATE_PRIMARY_PE;
            UART_SendMessage("Transition to STATE_PRIMARY_PE");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, 0);
        }
        break;

    case STATE_PRIMARY_PE:
        if (Copy_structEvent == EVENT_PRIMARY_PE_SUCCESSFUL)
        { // Triggered from CAN_MasterReceive
            currentState = STATE_PRIMARY_TDM;
            UART_SendMessage("Transition to STATE_PRIMARY_TDM");
            Global_PEDone[CAN_PRIMARY_ANCHOR] = 1;
        }
        break;

    case STATE_PRIMARY_TDM:
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
        { // Triggered from CAN_MasterReceive
            currentState = STATE_WAKEUP_DECISION_MAKING;
            UART_SendMessage("Transition to STATE_WAKEUP_DECISION_MAKING");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, 0); // First time the distance will be sent automatically but then when distance is above threshold this will be needed
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

    case STATE_WAKEUP_DECISION_MAKING:
        if (Copy_structEvent == EVENT_DISTANCE_ABOVE_THRESHOLD)
        {
            currentState = STATE_PRIMARY_TDM;
            UART_SendMessage("Transition back to STATE_PRIMARY_TDM");
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
        }
        else if (Copy_structEvent == EVENT_DISTANCE_BELOW_THRESHOLD)
        {
            currentState = STATE_SECONDARY_PE;
            UART_SendMessage("Transition to STATE_SECONDARY_PE");
            APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE);
        }
        break;

        //        case STATE_SECONDARY_PE: // Loop all anchors to make each one do passive entry
        //
        //            Global_u8CurrentAnchor++;
        //            if (Copy_structEvent== EVENT_DEVICE_IN_RANGE){
        //                // This case will be entered first time only and it will send CAN_COMMAND_TRIGGER_PASSIVE_ENTRY
        //            }
        //            else if(Copy_structEvent == EVENT_SECONDARY_PE_SUCCESSFUL){ // Triggered from CAN_MasterReceive
        //                Global_u8SuccessPE++;
        //                Global_PEDone[Global_u8CurrentAnchor-1] = 1;
        //                if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS && Global_u8CurrentAnchor > CAN_ANCHOR_MAX) {
        //                    currentState = STATE_SECONDARY_TDM;
        //                    UART_SendMessage("Transition to STATE_SECONDARY_TDM");
        //                } else {
        //                    UART_SendMessage("Staying in STATE_SECONDARY_PE");
        //                }
        //            }
        //            else if(Copy_structEvent == EVENT_SECONDARY_PE_FAILED){     // Triggered from CAN_MasterReceive
        //                if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS && Global_u8CurrentAnchor > CAN_ANCHOR_MAX) {
        //                    currentState = STATE_SECONDARY_TDM;
        //                    UART_SendMessage("Transition to STATE_SECONDARY_TDM");
        //                } else if(Global_u8SuccessPE < APP_MINUMUM_DISTANCE_READINGS && Global_u8CurrentAnchor > CAN_ANCHOR_MAX){
        //                    Global_u8CurrentAnchor = 1;
        //                    Global_u8SuccessPE = 0;
        //                } else {
        //                    UART_SendMessage("Staying in STATE_SECONDARY_PE");
        //                }
        //            }
        //            else{
        //
        //            }
        //            if (Global_u8CurrentAnchor < CAN_ANCHOR_MAX) {
        //                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
        //            }
        //            break;
    case STATE_SECONDARY_PE:
        if (Copy_structEvent == EVENT_DEVICE_IN_RANGE)
        {
            // First time entering, reset tracking variables
            Global_u8CurrentAnchor = 0;
            Global_u8SuccessPE = 0;
        }

        // If PE was successful, mark anchor as done
        if (Copy_structEvent == EVENT_SECONDARY_PE_SUCCESSFUL)
        {
            Global_u8SuccessPE++;
            Global_PEDone[Global_u8CurrentAnchor] = 1;
        }

        // Move to the next anchor
        Global_u8CurrentAnchor++;

        // Always check all anchors, even if we already reached minimum success
        if (Global_u8CurrentAnchor >= CAN_ANCHOR_MAX)
        {
            Global_u8CurrentAnchor = 0; // Restart from the first anchor

            // Only transition after all anchors have been checked at least once
            if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS)
            {
                currentState = STATE_SECONDARY_TDM;
                UART_SendMessage("Transition to STATE_SECONDARY_TDM");
                Global_u8SuccessPE = 1;
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                break;
            }
        }

        // Skip already checked anchors
        while (Global_u8CurrentAnchor < CAN_ANCHOR_MAX && Global_PEDone[Global_u8CurrentAnchor])
        {
            Global_u8CurrentAnchor++;
        }

        // If we still have an anchor left to check, send PE command
        if (Global_u8CurrentAnchor < CAN_ANCHOR_MAX)
        {
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
        }
        break;

    case STATE_SECONDARY_TDM:
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE) {
            // Find the next anchor that has completed PE but not yet measured
            while (Global_u8SuccessPE < CAN_ANCHOR_MAX && !Global_PEDone[Global_u8SuccessPE]) {
                Global_u8SuccessPE++;  // Move to the next anchor
            }

            if (Global_u8SuccessPE < CAN_ANCHOR_MAX) {
                // Send distance measurement command for the current successful anchor
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8SuccessPE, 0);
                Global_u8SuccessPE++; // Move to the next one for the next event trigger
            } else {
                // All successful anchors measured, transition to next state
                currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
                UART_SendMessage("Transition to STATE_VEHICLE_LEVEL_DECISION_MAKING");
            }
        }
        break;

    case STATE_VEHICLE_LEVEL_DECISION_MAKING:
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
        {
            currentState = STATE_FUSION_ALGO;
            UART_SendMessage("Transition to STATE_FUSION_ALGO");
        }
        else if (Copy_structEvent == EVENT_DISTANCE_ABOVE_THRESHOLD)
        {
            currentState = STATE_WAKEUP_DECISION_MAKING;
            UART_SendMessage("Transition to STATE_WAKEUP_DECISION_MAKING");
        }
        break;

    case STATE_FUSION_ALGO:
        if (Copy_structEvent == EVENT_FINAL_DISTANCE)
        {
            currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
            UART_SendMessage("Transition back to STATE_VEHICLE_LEVEL_DECISION_MAKING");
        }
        break;

    default:
        UART_SendMessage("Unknown state");
        break;
    }
}
