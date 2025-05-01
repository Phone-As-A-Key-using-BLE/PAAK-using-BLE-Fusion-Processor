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
char gArr_DebugMsg[600];
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
uint8_t Global_u8ExpectDistance = 1;
uint8_t Global_u8HandoverTo = 0 ;
uint8_t Global_u8isAnchorConnected[CAN_ANCHOR_MAX + 1]={0};
uint8_t Global_u8isWaitingForTDM[CAN_ANCHOR_MAX + 1]={0};
uint8_t Global_u8HandoverFailCount[CAN_ANCHOR_MAX + 1] = {0};
APP_tenuEvents Global_u8Event=0;
extern uint8_t isBondingDataReceived;
RSSIData_t targetData;
extern RSSIData_t gRSSIData[CAN_ANCHOR_MAX + 1];
//Fusion
Particle particles[NUM_PARTICLES];
uint8_t volatile firstTimeFlag = 1;
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
            Global_u8isWaitingForTDM[CAN_PRIMARY_ANCHOR] = 1;
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
            Global_u8isWaitingForTDM[CAN_PRIMARY_ANCHOR]=0;
            Global_u8isAnchorConnected[CAN_PRIMARY_ANCHOR] = 1;
            currentState = STATE_WAKEUP_DECISION_MAKING;
            UART_SendMessage("\n[INFO] Distance measurement completed. Evaluating wake-up decision...\n");

            // if (!Global_u8FirstTime)
            // {
            //     currentState = STATE_PRIMARY_TDM;
            //     // Trigger distance measurement when the device is out of range
            //     CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
            //     break;
            // }
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
            if(Global_u8isAnchorConnected[CAN_PRIMARY_ANCHOR] == 1){
                Global_u8isWaitingForTDM[CAN_PRIMARY_ANCHOR] = 1;
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
            }
            else
                CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Loc_u8DeviceId);
            break;
        }

        switch (Copy_structEvent)
        {
            case EVENT_RECEIVE_DISTANCE:
                        Global_u8isWaitingForTDM[Global_u8CurrentAnchor] = 0;
                        Global_u8SuccessPE++;
                        Global_f64Readings[Global_u8CurrentAnchor] = (double_t)(gRSSIData[Global_u8CurrentAnchor].distance / 100.0);
                        Global_PEDone[Global_u8CurrentAnchor] = 1;
                        if (Global_u8CurrentAnchor != CAN_PRIMARY_ANCHOR) {
                            // Store distance only from secondary anchors
                            snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
                                    "\n[SUCCESS] Distance received from anchor %d...\n", Global_u8CurrentAnchor);
                            UART_SendMessage(gArr_DebugMsg);
                        }


                        if (Global_u8CurrentAnchor != CAN_PRIMARY_ANCHOR) {
                            // After receiving distance from secondary, return to anchor 1
                            Global_u8HandoverTo = CAN_PRIMARY_ANCHOR;
                            //TimerDriver_Start(3000, HandoverTimeoutHandler);
                            Global_u8SendHandover = 1;
                        } else {
                            // In anchor 1, decide next unprocessed secondary anchor
                            uint8_t nextAnchor = 0;
                            uint8_t i = 0;
                            for (i = CAN_PRIMARY_ANCHOR + 1; i <= CAN_ANCHOR_MAX; i++) {
                                if (!Global_PEDone[i]) {
                                    nextAnchor = i;
                                    break;
                                }
                            }

                            if (nextAnchor) {
                                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
                                        "\n[INFO] Handover from anchor 1 to anchor %d...\n", nextAnchor);
                                UART_SendMessage(gArr_DebugMsg);
                                Global_u8HandoverTo = nextAnchor;
                                //TimerDriver_Start(3000, HandoverTimeoutHandler);
                                Global_u8SendHandover = 1;
                            } else if (Global_u8SuccessPE >= APP_MINUMUM_DISTANCE_READINGS) {
                                UART_SendMessage("\n[INFO] Minimum distance readings met. Proceeding to vehicle-level decision making...\n");
                                currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
                                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                                break;
                            } else {
                                UART_SendMessage("\n[INFO] Repeating distance measurements. Looping again...\n");
                                // Reset and retry
                                uint8_t i = 0;
                                for (i = 0; i <= CAN_ANCHOR_MAX; i++) {
                                    Global_PEDone[i] = 0;
                                }
                                Global_u8CurrentAnchor = CAN_PRIMARY_ANCHOR + 1;
                                Global_u8SendHandover = 1;
                            }
                        }
                        break;

                    case EVENT_HANDOVER_FAILED:
                                TimerDriver_Stop();
                                /* Increment failure count for this anchor */
                                Global_u8HandoverFailCount[Global_u8CurrentAnchor]++;
                                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
                                        "\n[ERROR] Handover failed on anchor %d (attempt %d)...\n",
                                        Global_u8CurrentAnchor,
                                        Global_u8HandoverFailCount[Global_u8CurrentAnchor]);
                                UART_SendMessage(gArr_DebugMsg);

                                if (Global_u8HandoverFailCount[Global_u8CurrentAnchor] >= MAX_HANDOVER_RETRIES)
                                {
                                    /* Exceeded max retries: reset counter and trigger PE to next anchor */
                                    UART_SendMessage("\n[INFO] Max handover retries reached. Sending PE to next anchor...\n");
                                    Global_u8HandoverFailCount[Global_u8CurrentAnchor] = 0;

                                    /* Determine next anchor to try PE on */
                                    uint8_t nextAnchor = 0;
                                    uint8_t i=0;
                                    for (i = CAN_PRIMARY_ANCHOR + 1; i <= CAN_ANCHOR_MAX; i++)
                                    {
                                        if (!Global_PEDone[i])
                                        {
                                            nextAnchor = i;
                                            break;
                                        }
                                    }
                                    if (nextAnchor == 0)
                                    {
                                        /* wrap or fallback to primary if all done */
                                        nextAnchor = CAN_PRIMARY_ANCHOR;
                                    }

                                    /* Prepare for passive entry on the selected anchor */
                                    Global_u8CurrentAnchor = nextAnchor;
                                    Global_u8SendPE = 1;
                                }
                                else
                                {
                                    /* Retry handover as before */
                                    UART_SendMessage("\n[INFO] Retrying handover...\n");
                                    Global_u8SendHandover = 1;
                                }
                                break;
                    case EVENT_HANDOVER_SUCCESS:
                        TimerDriver_Stop();
                        Global_u8HandoverFailCount[Global_u8CurrentAnchor]=0;
                        Global_u8isAnchorConnected[Global_u8CurrentAnchor] = 0;
                        Global_u8isAnchorConnected[Global_u8HandoverTo] = 1;
                        Global_u8CurrentAnchor = Global_u8HandoverTo;
                        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
                                "\n[INFO] Handover success: Now at anchor %d\n", Global_u8CurrentAnchor);
                        UART_SendMessage(gArr_DebugMsg);

                        // Now that handover succeeded, you can start requesting distance
                        // from the current anchor (if not primary)
                        if (Global_u8CurrentAnchor != CAN_PRIMARY_ANCHOR) {
                            if(Global_u8isAnchorConnected[Global_u8CurrentAnchor] == 1){
                                    Global_u8isWaitingForTDM[Global_u8CurrentAnchor] = 1;
                                    CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8CurrentAnchor, Loc_u8DeviceId);
                            }
                            else
                                    CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Loc_u8DeviceId);
                        }
                        else{
                            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                            break;
                        }
                        break;
                    case EVENT_SECONDARY_WAKEUP_RECEIVED:
                        TimerDriver_Stop();
                        Global_u8isAnchorConnected[Global_u8CurrentAnchor] = 0;
                        Global_u8isAnchorConnected[Global_u8HandoverTo] = 0;
                        //CAN_voidSendCommand(CAN_COMMAND_DISCONNECT_FROM_DEVICE, Global_u8CurrentAnchor-1, Loc_u8DeviceId);
                        Global_u8CurrentAnchor = Global_u8HandoverTo;
                        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
                                "\n[INFO] Handover failed on anchor %d and was reset .. Sending PE to this anchor\n", Global_u8CurrentAnchor);
                        UART_SendMessage(gArr_DebugMsg);
                        Global_u8SendPE = 1;
                    break;
                    case EVENT_SECONDARY_PE_FAILED:
                    case EVENT_PRIMARY_PE_FAILED:
                        TimerDriver_Stop();
                        Global_u8isAnchorConnected[Global_u8CurrentAnchor] = 0;
                        Global_u8isAnchorConnected[Global_u8HandoverTo] = 0;
                        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
                                "\n[INFO] Anchor %d Disconnected .. Sending PE to anchor %d\n", Global_u8CurrentAnchor,Global_u8HandoverTo);
                        UART_SendMessage(gArr_DebugMsg);
                        Global_u8CurrentAnchor = Global_u8HandoverTo;
                        Global_u8SendPE = 1;
                    
                    default:
                        break;
            }
        break;

    // **VEHICLE LEVEL DECISION MAKING**: Evaluate final position of the device
    case STATE_VEHICLE_LEVEL_DECISION_MAKING:
        UART_SendMessage("\n[INFO] Executing vehicle-level decision-making process...\n");
        if (Copy_structEvent == EVENT_RECEIVE_DISTANCE)
        {
            TimerDriver_Stop();
            Global_u8CurrentAnchor = CAN_ANCHOR_1;
            //CAN_voidSendCommand(CAN_COMMAND_DISCONNECT_FROM_DEVICE, CAN_ANCHOR_2, 0);
            uint8_t Loc_u8Distance = CAN_structGetDistanceData().distanceIntegerPart;
            if (Loc_u8Distance <= APP_DISTANCE_TRIGGER_THRESHOLD)
            {
                UART_SendMessage("\n[INFO] Device is in range, start fusion algo...\n");
                currentState = STATE_FUSION_ALGO;
                Global_u8Event = EVENT_DISTANCE_BELOW_THRESHOLD;
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
        estimate_final[0] = 0;
        estimate_final[1] = 0;
        measureA = Master_trilaterate_position(testObj);

        // snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Location from Trilateration: (x = %.2f , y = %.2f)\n", measureA.x,measureA.y);
        // UART_SendMessage(gArr_DebugMsg);

        //Particle filter
        // if (firstTimeFlag){
        //   firstTimeFlag = 0;
        //   Master_initialize_particles(particles,measureA.x,measureA.y,1.00);
        // }

        //Master_prediction(particles);
        //Master_update_particles(particles,measureA);
        //Master_resample(particles);
//        Master_estimate(particles,estimate_final);

        //sprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Location from Particle Filter: (x = %.2f , y = %.2f)\n", estimate_final[0],estimate_final[1]);
        //UART_SendMessage(gArr_DebugMsg);
        // my_sprintf(&buffer, "\n[INFO] Location from Particle Filter: (x = %.2f , y = %.2f)\n", estimate_final[0],estimate_final[1]);
        // UART_SendMessage(buffer);
        UART_SendMessage("\n[INFO] Fusion Algorithm is done, returning to vehicle decision-making...\n");
        currentState = STATE_PRIMARY_TDM;
        //Add fusion Algo
        if(Global_u8isAnchorConnected[Global_u8CurrentAnchor] == 1)
                Global_u8SendTDM=1;
        else
                Global_u8SendPE=1;
        // CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Loc_u8DeviceId);
        // APP_voidFSMHandler(EVENT_PRIMARY_WAKEUP_RECEIVED);
        break;

    default:
        UART_SendMessage("[ERROR] Unknown system state encountered.\n");
        break;
    }
}