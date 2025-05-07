/*
 * APP_FSM.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Mohamed Abdel Hamid
 */

#include <string.h>
#include <stdio.h>
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/CAN_MasterReceive.h"
#include "DeviceRangingTypeManager.h"
#include "APP/CAN_App.h"
#include "APP/APP_FSM.h"
#include "APP/Timer.h"
#include "APP/ErrorHandling.h"
//Fusion
#include "APP/Fusion/Trilateration.h"
#include "APP/Fusion/Particle.h"

void delay_ms(uint32_t ms) {
    // 1 ms delay = (SysClk / 3) / 1000 loops
    SysCtlDelay((SysCtlClockGet() / 3) / 1000 * ms);
}

/* Debug print buffer */
char gArr_DebugMsg[1024];
APP_tenuStates currentState = STATE_IDLE;

uint8_t Global_PEDone[CAN_ANCHOR_MAX+1] = {0};
double_t Global_f64Readings[CAN_ANCHOR_MAX+1] = {0};
uint8_t Global_u8CurrentAnchor = CAN_PRIMARY_ANCHOR;
uint8_t Global_u8SuccessPE = 0; // Success Passive Entry
uint8_t Global_u8FirstTime = 1; 
uint8_t Global_u8PERetryCount = 0;

// Sync Flags
uint8_t Global_u8SendPE = 0;
uint8_t Global_u8SendTDM = 0;
uint8_t Global_u8SendHandover= 0;
uint8_t Global_u8CurrentDeviceId=0;

extern CAN_tstrDistance s_distanceData;  
extern uint8_t isBondingDataReceived;
RSSIData_t targetData;
extern RSSIData_t gRSSIData[CAN_ANCHOR_MAX + 1];
//Fusion
Particle particles[NUM_PARTICLES];
uint8_t volatile firstTimeFlag = 1;
uint8_t Global_u8ResetAndPE = 0;
void APP_voidFSMHandler(APP_tenuEvents Copy_structEvent)
{
    uint8_t Loc_u8DeviceId = 0;
    object_list testObj[CAN_ANCHOR_MAX];
    double_t estimate_final[2];
    Measurement_Type measureA;
    switch (currentState)
    {
    // **IDLE STATE**: System is waiting for user input
    case STATE_IDLE:
        if (Copy_structEvent == EVENT_OWNER_PAIRING_BUTTON_PRESSED)
        {
            Global_u8CurrentAnchor = CAN_PRIMARY_ANCHOR;
            Global_u8SuccessPE = 0;
            Global_u8FirstTime = 1;
            Global_u8PERetryCount = 0;
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
            isBondingDataReceived=0;
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
            CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_RESET_ALL, 0);
        }
        else if (Copy_structEvent == EVENT_PRIMARY_PE_FAILED)
        {
            currentState = STATE_PRIMARY_PE;
            UART_SendMessage("\n[ERROR] Trigger owner pairing failed. Trigger OP again...\n");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_OWNER_PAIRING, CAN_PRIMARY_ANCHOR, 0);
        }
        break;
    // **WAITING FOR PRIMARY WAKEUP**: System waits for a wake-up event
    case STATE_WAITING_FOR_PRIMARY_WAKEUP:
        if (Copy_structEvent == EVENT_PRIMARY_WAKEUP_RECEIVED)
        {
            currentState = STATE_PRIMARY_PE;
            UART_SendMessage("\n[INFO] Wake-up signal received. Triggering Passive Entry (PE) on primary anchor...\n");
            DeviceStateManager_Update(Loc_u8DeviceId, APP_CS);
            //CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
            Global_u8CurrentDeviceId = Loc_u8DeviceId;
            Global_u8SendPE=1;

        }
        break;

    // **PRIMARY PASSIVE ENTRY (PE)**: Handle success or failure of PE
    case STATE_PRIMARY_PE:
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
        {
            currentState = STATE_PRIMARY_TDM;
            UART_SendMessage("\n[SUCCESS] Primary Passive Entry successful. Proceeding to Trigger Distance Measurement (TDM)...\n");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
        }
        else if (Copy_structEvent == EVENT_PRIMARY_PE_FAILED)
        {
            currentState = STATE_PRIMARY_PE;
            UART_SendMessage("\n[ERROR] Primary Passive Entry failed. Trigger PE again...\n");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
        }
        else if (Copy_structEvent == EVENT_PRIMARY_WAKEUP_RECEIVED)
        {
            currentState = STATE_PRIMARY_TDM;
            DeviceStateManager_Update(Loc_u8DeviceId, APP_RSSI);
            UART_SendMessage("\n[INFO] CS is not supported in mobile. Turning to RSSI...\n");
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
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
                currentState = STATE_PRIMARY_TDM;
                // Trigger distance measurement when the device is out of range
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
                break;
            }
            Global_u8FirstTime = 0;

            uint8_t Loc_u8Distance = CAN_structGetDistanceData().distanceIntegerPart;
            //  uint8_t Loc_u8Distance = 4;
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
        if (Copy_structEvent == EVENT_DISTANCE_BELOW_THRESHOLD)
        {
            currentState = STATE_SECONDARY_PE;
            UART_SendMessage("\n[INFO] Device is in range. Initiating Secondary Passive Entry (PE)...\n");
            Global_u8FirstTime = 1;
            APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE);
        }
        else if (Copy_structEvent == EVENT_DISTANCE_ABOVE_THRESHOLD)
        {
            currentState = STATE_PRIMARY_TDM;
            UART_SendMessage("\n[INFO] Distance above threshold. Retrying distance measurement...\n");
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
        }
        break;

   case STATE_SECONDARY_PE:
        if (Copy_structEvent == EVENT_DEVICE_IN_RANGE && Global_u8FirstTime)
        {
            // Initialize PE process
            Global_u8CurrentAnchor = CAN_PRIMARY_ANCHOR;
            Global_u8SuccessPE = 0;
            Global_u8FirstTime = 0;
            Global_u8PERetryCount = 0;
             uint8_t i;
            for (i = CAN_PRIMARY_ANCHOR; i <= CAN_ANCHOR_MAX; i++)
                Global_PEDone[i]=0;
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
            break;
        }

        switch (Copy_structEvent)
        {
            case EVENT_RECEIVE_DISTANCE:
                Global_u8SuccessPE++;
                Global_PEDone[Global_u8CurrentAnchor] = 1;
                Global_f64Readings[Global_u8CurrentAnchor] = (double_t)(gRSSIData[Global_u8CurrentAnchor].distance/100.0);
                    /* Log the parsed data */
                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[SUCCESS] Distance received from anchor %d (Passive Entry assumed success)...\n", Global_u8CurrentAnchor);
                UART_SendMessage(gArr_DebugMsg);
                double Loc_f64Distance = s_distanceData.distanceIntegerPart + (s_distanceData.distanceDecimalPart/100.0);
                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),"Anchor %d Distance %s: %.2f meter\r\n", Global_u8CurrentAnchor, "CS", Loc_f64Distance);
                UART_SendMessage (gArr_DebugMsg);
                Global_u8PERetryCount = 0;
                //CAN_voidSendCommand(CAN_COMMAND_DISCONNECT_FROM_DEVICE, Global_u8CurrentAnchor, Loc_u8DeviceId);
                // Move to the next available anchor
                do {
                    Global_u8CurrentAnchor++;
                } while (Global_u8CurrentAnchor <= CAN_ANCHOR_MAX && Global_PEDone[Global_u8CurrentAnchor]);

                
                break;

            case EVENT_SECONDARY_PE_FAILED:
            case EVENT_PRIMARY_PE_FAILED:
                // if (Global_u8PERetryCount < MAX_PE_RETRIES)
                // {
                //     Global_u8PERetryCount++;
                //     snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[WARNING] Passive Entry failed on anchor %d. Retrying attempt %d/%d...\n",
                //             Global_u8CurrentAnchor, Global_u8PERetryCount, MAX_PE_RETRIES);
                //     UART_SendMessage(gArr_DebugMsg);
                //     Global_u8CurrentDeviceId = Loc_u8DeviceId;
                //     Global_u8SendPE = 1;
                // }
                // else
                // {
                //     snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[ERROR] Passive Entry failed on anchor %d after maximum retries. Moving to next anchor...\n", Global_u8CurrentAnchor+1);
                //     UART_SendMessage(gArr_DebugMsg);
                //     Global_u8PERetryCount = 0;
                //     do {
                //         Global_u8CurrentAnchor++;
                //     } while (Global_u8CurrentAnchor <= CAN_ANCHOR_MAX && Global_PEDone[Global_u8CurrentAnchor]);
                //     Global_u8CurrentDeviceId = Loc_u8DeviceId;
                //     Global_u8SendPE = 1;
                // }
                Global_u8CurrentDeviceId = Loc_u8DeviceId;
                // Global_u8ResetAndPE = 2;
                Global_u8SendPE=1;
                break;

            case EVENT_HANDOVER_SUCCESS:
                TimerDriver_Stop();
                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Sending trigger distance measurement command to anchor %d...\n", Global_u8CurrentAnchor);
                UART_SendMessage(gArr_DebugMsg);
                Global_u8CurrentDeviceId = Loc_u8DeviceId;
                Global_u8SendTDM = 1;
            break;
            case EVENT_HANDOVER_FAILED:
                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Handover failed try again...\n");
                UART_SendMessage(gArr_DebugMsg);
                Global_u8CurrentDeviceId = Loc_u8DeviceId;
                Global_u8SendHandover = 1;
            break;
            case EVENT_PRIMARY_WAKEUP_RECEIVED:
            case EVENT_SECONDARY_WAKEUP_RECEIVED:
                Global_u8CurrentDeviceId = Loc_u8DeviceId;
                Global_u8SendPE = 1;
            break;

        }

        if(Copy_structEvent == EVENT_RECEIVE_DISTANCE || Copy_structEvent == EVENT_HANDOVER_FAILED ){
            if (Global_u8CurrentAnchor <= CAN_ANCHOR_MAX)
            {
                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Sending handover command from anchor %d to anchor %d...\n", Global_u8CurrentAnchor-1 , Global_u8CurrentAnchor);
                UART_SendMessage(gArr_DebugMsg);
                Global_u8CurrentDeviceId = Loc_u8DeviceId;
                //TimerDriver_Start(3000, HandoverTimeoutHandler);
                Global_u8SendHandover = 1;
                break;
            }
            else if (Global_u8CurrentAnchor >= CAN_ANCHOR_MAX && Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS)
            {
                uint8_t i;
                for (i = CAN_PRIMARY_ANCHOR; i <= CAN_ANCHOR_MAX; i++)
                    Global_PEDone[i]=0;
                // Initialize PE process
                //Global_u8CurrentAnchor = CAN_PRIMARY_ANCHOR;
                Global_u8SuccessPE = 0;
                Global_u8FirstTime = 1;
                Global_u8PERetryCount = 0;
                Global_u8CurrentDeviceId = Loc_u8DeviceId;

                UART_SendMessage("\n[INFO] Minimum distance readings met. Proceeding to vehicle-level decision making...\n");
                currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;

                Global_u8SendHandover=1;
                //APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                break;
            }
            else if (Global_u8CurrentAnchor > CAN_ANCHOR_MAX && Global_u8SuccessPE < APP_MINUMUM_DISTANCE_READINGS)
            {
                uint8_t i=0;
                for (i = 0; i < CAN_ANCHOR_MAX ;i++)
                    Global_PEDone[i]=0;
                
                Global_u8FirstTime = 1;
                
                CAN_voidSendCommand(CAN_COMMAND_DISCONNECT_FROM_DEVICE, Global_u8CurrentAnchor-1, Loc_u8DeviceId);
                
                UART_SendMessage("\n[INFO] Loop again through anchors...\n");
                currentState = STATE_SECONDARY_PE;
                APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE);
                break;
            }
        }
        break;

    // **VEHICLE LEVEL DECISION MAKING**: Evaluate final position of the device
    case STATE_VEHICLE_LEVEL_DECISION_MAKING:
        UART_SendMessage("\n[INFO] Executing vehicle-level decision-making process...\n");
        if (Copy_structEvent == EVENT_HANDOVER_SUCCESS)
        {
            Global_u8CurrentAnchor = CAN_ANCHOR_1;
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
        else if(Copy_structEvent == EVENT_SECONDARY_PE_FAILED || Copy_structEvent == EVENT_PRIMARY_PE_FAILED){
                Global_u8CurrentAnchor = CAN_ANCHOR_1;
                Global_u8CurrentDeviceId = Loc_u8DeviceId;
                // Global_u8ResetAndPE = 2;
                Global_u8SendPE=1;
        }
        else if(Copy_structEvent == EVENT_HANDOVER_FAILED){
            Global_u8SendHandover=1;
        }
        else if(Copy_structEvent == EVENT_PRIMARY_WAKEUP_RECEIVED || Copy_structEvent == EVENT_SECONDARY_WAKEUP_RECEIVED){
                 Global_u8CurrentAnchor = CAN_ANCHOR_1;
                Global_u8CurrentDeviceId = Loc_u8DeviceId;
                Global_u8SendPE = 1;
        }
        break;

    // **FUSION ALGORITHM STATE**: Execute sensor fusion
    case STATE_FUSION_ALGO:
        

        /******* Add functions for distance 3 calculations  *******/
        //Global_f64Readings[2] = ???
        // For testObj array:
      testObj[0] = (object_list){Global_f64Readings[1], 1};
      testObj[1] = (object_list){Global_f64Readings[2], 2};
      testObj[2] = (object_list){Global_f64Readings[3], 3};
    // testObj[0] = (object_list){0.8,1};
    // testObj[1] = (object_list){1.5,2};
    // testObj[2] = (object_list){0,3};
        // For estimate_final array:
        // estimate_final[0] = 0;
        // estimate_final[1] = 0;
        // measureA = Master_trilaterate_position(testObj);

        // snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Location from Trilateration: (x = %.2f , y = %.2f)\n", measureA.x,measureA.y);
        // UART_SendMessage(gArr_DebugMsg);

        //Particle filter
        // if (firstTimeFlag){
        //   firstTimeFlag = 0;
        //   Master_initialize_particles(particles,measureA.x,measureA.y,1.00);
        // }

        // Master_prediction(particles);
        // Master_update_particles(particles,measureA);
        // Master_resample(particles);
        // Master_estimate(particles,estimate_final);

        //sprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Location from Particle Filter: (x = %.2f , y = %.2f)\n", estimate_final[0],estimate_final[1]);
        //UART_SendMessage(gArr_DebugMsg);
        // my_sprintf(&buffer, "\n[INFO] Location from Particle Filter: (x = %.2f , y = %.2f)\n", estimate_final[0],estimate_final[1]);
        // UART_SendMessage(buffer);
        UART_SendMessage("\n[INFO] Fusion Algorithm is done, returning to vehicle decision-making...\n");
        currentState = STATE_PRIMARY_TDM;
        //Add fusion Algo
        Global_u8SendTDM=1;
        // CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Loc_u8DeviceId);
        // APP_voidFSMHandler(EVENT_PRIMARY_WAKEUP_RECEIVED);
        break;

    default:
        UART_SendMessage("[ERROR] Unknown system state encountered.\n");
        break;
    }
}
