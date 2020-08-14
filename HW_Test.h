/*******************************************************************************
* Title                 :   Hardware Test
* Filename              :   ATE_enter_HW_mode.h
* Author                :   Nguyen Dac Hoang
* Origin Date           :   10/08/2020
* Notes                 :   None
*******************************************************************************/

#ifndef ATE_ENTER_HW_MODE_H_
#define ATE_ENTER_HW_MODE_H_

/******************************************************************************
* INCLUDES
*******************************************************************************/
#include <arduino.h>
#include <string.h>
#include <stdint.h>
/******************************************************************************
* PREPROCESSOR CONSTANTS
*******************************************************************************/
#define ATE_TEST_BUF_LEN (40) //Byte
#define ATE_RESULT_BUF_LEN (40) //Byte
#define ATE_RX_MSG_BUF_LEN (50) //Byte
#define ATE_TX_MSG_BUF_LEN (50) //Byte


/******************************************************************************
* CONFIGURATION CONSTANTS
*******************************************************************************/
static uint8_t tx_buff [ ATE_TX_MSG_BUF_LEN ];
static uint8_t rx_buff [ ATE_RX_MSG_BUF_LEN ];
static uint8_t ATE_result_buff [ATE_RESULT_BUF_LEN];
static uint8_t ATE_test_buff [ATE_TEST_BUF_LEN];

/******************************************************************************
* MACROS
*******************************************************************************/


/******************************************************************************
* TYPEDEFS
*******************************************************************************/


/******************************************************************************
* VARIABLES
* 
*******************************************************************************/
 extern uint8_t ATE_message_ID;
 extern uint8_t ATE_component_ID;
 extern uint8_t ATE_command_ID;
 extern uint8_t ATE_test_length;
 extern uint8_t ATE_payload_length;
 extern bool    ATE_in_HT_mode_flag;
/******************************************************************************
* FUNCTION PROTOTYPES
*******************************************************************************/
bool ATE_enter_HT_mode (void);
void execute_cmd (uint8_t id);
void execute_res (uint8_t *rx_msg, uint8_t msgSize);
static void ATE_send_response (uint8_t msgID, uint8_t oriMsgId, uint8_t oriSeqNum, uint8_t error);
void ATE_send_ping_msg (void);
void ATE_send_request_msg (void);

#endif 
/*** END OF FILE **************************************************************/
