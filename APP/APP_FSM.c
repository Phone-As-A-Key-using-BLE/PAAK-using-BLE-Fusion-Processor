/*
 * APP_FSM.c
 *
 *  Created on: Feb 27, 2025
 *      Author: 
 */
#include <cmath>
#include <cstdint>
#include <string.h>
#include <stdio.h>
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/CAN_MasterReceive.h"
#include "APP/CAN_App.h"
#include "APP/APP_FSM.h"

//Fusion
#include "APP/Fusion/Trilateration.h"
#include "APP/Fusion/Particle.h"


char buffer[1024]; // for debugging

APP_tenuStates currentState = STATE_IDLE;

uint8_t Global_PEDone[CAN_ANCHOR_MAX]={0,0,0};
double_t Global_f64Readings[CAN_ANCHOR_MAX] = {0,0,0};
uint8_t Global_u8CurrentAnchor=0;
uint8_t Global_u8SuccessPE=0; // Success Passive Entry
uint8_t Global_u8FirstTime=1; 
uint8_t Global_u8PERetryCount=0;
uint8_t flag=1;

//Fusion
Particle particles[NUM_PARTICLES];
uint8_t volatile firstTimeFlag = 1;

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
            uint32_t delay=10000;
            while(delay--);
            CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_PRIMARY_ANCHOR, 0);
        }
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
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, 0);
        }
        else if (Copy_structEvent == EVENT_PRIMARY_PE_FAILED)
        {
            currentState = STATE_WAITING_FOR_PRIMARY_WAKEUP;
            UART_SendMessage("\n[ERROR] Primary Passive Entry failed. Reverting to bonding data state to reset and trigger PE again...\n");
            APP_voidFSMHandler(EVENT_PRIMARY_WAKEUP_RECEIVED);
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
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, 0);
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

   case STATE_SECONDARY_PE:
        if (Copy_structEvent == EVENT_DEVICE_IN_RANGE && Global_u8FirstTime)
        {
            // Initialize PE process
            Global_u8CurrentAnchor = 1;
            Global_u8SuccessPE = 0;
            Global_u8FirstTime = 0;
            Global_u8PERetryCount = 0;
        }

        switch (Copy_structEvent)
        {
            case EVENT_RECEIVE_DISTANCE:
                Global_u8SuccessPE++;
                Global_PEDone[Global_u8CurrentAnchor] = 1;
                Global_f64Readings[Global_u8CurrentAnchor] = (double_t)(CAN_structGetDistanceData().distanceIntegerPart + CAN_structGetDistanceData().distanceDecimalPart/100.0);

                snprintf(buffer, sizeof(buffer), "\n[SUCCESS] Distance received from anchor %d (Passive Entry assumed success)...\n", Global_u8CurrentAnchor);
                UART_SendMessage(buffer);

                Global_u8PERetryCount = 0;
                
                // Move to the next available anchor
                do {
                    Global_u8CurrentAnchor++;
                } while (Global_u8CurrentAnchor < CAN_ANCHOR_MAX && Global_PEDone[Global_u8CurrentAnchor]);

                if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS)
                {
                    uint8_t i=0;
                    for (i = 0; i < CAR_ANCHOR_MAX ;i++)
                        Global_PEDone[i]=0;
                    
                    UART_SendMessage("\n[INFO] Minimum distance readings met. Proceeding to vehicle-level decision making...\n");
                    currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
                    APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                    break;
                }
                break;

            case EVENT_SECONDARY_PE_FAILED:
                if (Global_u8PERetryCount < MAX_PE_RETRIES)
                {
                    Global_u8PERetryCount++;
                    snprintf(buffer, sizeof(buffer), "\n[WARNING] Passive Entry failed on anchor %d. Retrying attempt %d/%d...\n",
                            Global_u8CurrentAnchor, Global_u8PERetryCount, MAX_PE_RETRIES);
                    UART_SendMessage(buffer);
                }
                else
                {
                    snprintf(buffer, sizeof(buffer), "\n[ERROR] Passive Entry failed on anchor %d after maximum retries. Moving to next anchor...\n", Global_u8CurrentAnchor);
                    UART_SendMessage(buffer);
                    Global_u8PERetryCount = 0;
                    do {
                        Global_u8CurrentAnchor++;
                    } while (Global_u8CurrentAnchor < CAN_ANCHOR_MAX && Global_PEDone[Global_u8CurrentAnchor]);
                }
                break;
        }

        if (Global_u8CurrentAnchor < CAN_ANCHOR_MAX)
        {
            snprintf(buffer, sizeof(buffer), "\n[INFO] Sending Passive Entry command to anchor %d...\n", Global_u8CurrentAnchor);
            UART_SendMessage(buffer);
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, 0);
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
        //Add fusion Algo
        

        /******* Add functions for distance 3 calculations  *******/
        //Global_f64Readings[2] = ???
        object_list testObj [NUM_OF_ANCHORS] = {
        {Global_f64Readings[0],1},{Global_f64Readings[1],2},{Global_f64Readings[2],3}
          };
        double_t estimate_final[2]={0,0};
        Measurement_Type measureA = Master_trilaterate_position(testObj);

        //Particle filter
        if (firstTimeFlag){
          firstTimeFlag = 0;
          Master_initialize_particles(particles,measureA.x,measureA.y,1.00);
        }

        Master_prediction(particles);
        Master_update_particles(particles,measureA);
        Master_resample(particles);
        Master_estimate(particles,estimate_final);


        APP_voidFSMHandler(EVENT_FINAL_DISTANCE);
        break;

    default:
        UART_SendMessage("[ERROR] Unknown system state encountered.\n");
        break;
    }
}