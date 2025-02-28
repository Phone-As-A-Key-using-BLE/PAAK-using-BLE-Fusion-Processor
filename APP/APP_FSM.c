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

bool Global_PEDone [CAN_ANCHOR_MAX];

uint8_t Loc_u8CurrentAnchor;
uint8_t pe_success_count;

void APP_voidFSMHandler(APP_tenuEvents Copy_structEvent){
    switch (currentState) {
        case STATE_IDLE:
            if (Copy_structEvent== EVENT_OWNER_PAIRING_BUTTON_PRESSED) {
                currentState = STATE_START_OWNER_PAIRING;
                UART_SendMessage("Transition to STATE_START_OWNER_PAIRING");
            }
            break;

        case STATE_START_OWNER_PAIRING:
            if (Copy_structEvent== EVENT_SEND_OWNER_PAIRING_COMMAND) {
                currentState = STATE_WAITING_FOR_BONDING_DATA;
                UART_SendMessage("Transition to STATE_WAITING_FOR_BONDING_DATA");
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_OWNER_PAIRING, CAN_PRIMARY_ANCHOR, 0);
            }
            break;

        case STATE_WAITING_FOR_BONDING_DATA:
            if (Copy_structEvent== EVENT_BONDING_DATA_RECEIVED) {
                currentState = STATE_PRIMARY_PE;
                UART_SendMessage("Transition to STATE_PRIMARY_PE");
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, 0);
            }
            break;

        case STATE_PRIMARY_PE:
            if (Copy_structEvent== EVENT_PRIMARY_PE_SUCCESSFUL) {
                currentState = STATE_PRIMARY_TDM;
                UART_SendMessage("Transition to STATE_PRIMARY_TDM");
                Global_PEDone[CAN_PRIMARY_ANCHOR] = 1;
            }
            break;

        case STATE_PRIMARY_TDM:
            if (Copy_structEvent== EVENT_RECEIVE_DISTANCE) {
                currentState = STATE_WAKEUP_DECISION_MAKING;
                UART_SendMessage("Transition to STATE_WAKEUP_DECISION_MAKING");
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, 0);
                uint8_t Loc_structDistance = CAN_structGetDistanceData().distanceIntegerPart;
                if(Loc_structDistance <= APP_DISTANCE_TRIGGER_THRESHOLD){
                    APP_voidFSMHandler(EVENT_DISTANCE_BELOW_THRESHOLD);
                }else{
                    APP_voidFSMHandler(EVENT_DISTANCE_ABOVE_THRESHOLD);
                }
            }
            break;

        case STATE_WAKEUP_DECISION_MAKING:
            if (Copy_structEvent== EVENT_DISTANCE_ABOVE_THRESHOLD) {
                currentState = STATE_PRIMARY_TDM;
                UART_SendMessage("Transition back to STATE_PRIMARY_TDM");
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);

            } else if (Copy_structEvent== EVENT_DISTANCE_BELOW_THRESHOLD) {
                currentState = STATE_SECONDARY_PE;
                UART_SendMessage("Transition to STATE_SECONDARY_PE");
                APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE);
            }
            break;

        case STATE_SECONDARY_PE:

            Loc_u8CurrentAnchor++;
            if (Copy_structEvent== EVENT_DEVICE_IN_RANGE);
            else if(Copy_structEvent == EVENT_SECONDARY_PE_SUCCESSFUL){
                pe_success_count++;
                Global_PEDone[Loc_u8CurrentAnchor-1] = 1;
                if (pe_success_count >= APP_MINUMUM_DISTANCE_READINGS && Loc_u8CurrentAnchor>CAN_ANCHOR_MAX) {
                    currentState = STATE_SECONDARY_TDM;
                    UART_SendMessage("Transition to STATE_SECONDARY_TDM");
                } else {
                    UART_SendMessage("Staying in STATE_SECONDARY_PE");
                }
            }
            else if(Copy_structEvent == EVENT_SECONDARY_PE_FAILED){
                if (pe_success_count >= APP_MINUMUM_DISTANCE_READINGS && Loc_u8CurrentAnchor>CAN_ANCHOR_MAX) {
                    currentState = STATE_SECONDARY_TDM;
                    UART_SendMessage("Transition to STATE_SECONDARY_TDM");
                } else if(pe_success_count < APP_MINUMUM_DISTANCE_READINGS && Loc_u8CurrentAnchor>CAN_ANCHOR_MAX){
                    Loc_u8CurrentAnchor = 1;
                    pe_success_count = 0;
                } else {
                    UART_SendMessage("Staying in STATE_SECONDARY_PE");
                }
            }
            else{

            }
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Loc_u8CurrentAnchor, 0);
            Loc_u8CurrentAnchor++;

            break;

        case STATE_SECONDARY_TDM:
            pe_success_count = 0;
            if (Copy_structEvent== EVENT_RECEIVE_DISTANCE) {
                UART_SendMessage("Transition to STATE_VEHICLE_LEVEL_DECISION_MAKING");

                while(!Global_PEDone[pe_success_count]){pe_success_count++;}
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, pe_success_count, 0);
                pe_success_count++;
            }
            currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
            break;

        case STATE_VEHICLE_LEVEL_DECISION_MAKING:
            if (Copy_structEvent== EVENT_RECEIVE_DISTANCE) {
                currentState = STATE_FUSION_ALGO;
                UART_SendMessage("Transition to STATE_FUSION_ALGO");
            } else if (Copy_structEvent== EVENT_DISTANCE_ABOVE_THRESHOLD) {
                currentState = STATE_WAKEUP_DECISION_MAKING;
                UART_SendMessage("Transition to STATE_WAKEUP_DECISION_MAKING");
            }
            break;

        case STATE_FUSION_ALGO:
            if (Copy_structEvent== EVENT_FINAL_DISTANCE) {
                currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
                UART_SendMessage("Transition back to STATE_VEHICLE_LEVEL_DECISION_MAKING");
            }
            break;

        default:
            UART_SendMessage("Unknown state");
            break;
    }
}


