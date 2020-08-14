/*******************************************************************************
* Title                 :   HW Test
* Filename              :   HW_Test.cpp
* Author                :   Quoc Tran
* Created Date          :   Aug 07, 2020
* Notes                 :   
*******************************************************************************/

/////////////////////INCLUDE/////////////////////
#include <stdint.h>
#include <string.h>
#include <arduino.h>
#include "HW_Test_Config.h"
#include "ATE_Comm.h"
#include "HW_Test.h"

/////////////////////PARAMETERS////////////////////


/////////////////////VARIABLE DEFINITIONS////////////////////
uint8_t ATE_message_ID;
uint8_t ATE_component_ID;
uint8_t ATE_command_ID;
uint8_t ATE_test_length;
uint8_t ATE_payload_length;
bool    ATE_in_HT_mode_flag = false;
/////////////////////FUNCTION PROTOTYPES////////////////////


//////////////////////////FUNCTIONS/////////////////////////

/******************************************************************************
* Function       : bool ATE_enter_HT_mode (void)
* Description    : ATE checks to enter hardware test mode           
* Input          : None.
* Output         : None.
* Return         : None.
/ 11/08/2020     : Dac Hoang -  Initial version.
*******************************************************************************/
bool ATE_enter_HT_mode (void)
{
  bool enter_ht_mode = false ;
  //Config trig 1 and trig 2 as Output 
  pinMode(ATE_TRIG_1_PIN, OUTPUT);
  pinMode(ATE_TRIG_2_PIN, OUTPUT);

  //Pull low trig 1 and trig 2 
  digitalWrite(ATE_TRIG_1_PIN, LOW);
  digitalWrite(ATE_TRIG_2_PIN, LOW);

  //Config Trig 3 as Input 
  pinMode(ATE_TRIG_3_PIN, INPUT);
  
   // Check to enter hardware test mode
  if(!digitalRead(ATE_TRIG_3_PIN)) enter_ht_mode= true;
  if(enter_ht_mode = true)
  {
    delay(100); //delay to polling
    //change trig 3 to output in ATE
    //trig 3 has a external Pull up - Resistor
    if(digitalRead(ATE_TRIG_3_PIN))
    {
      pinMode(ATE_TRIG_3_PIN, OUTPUT);
      delay(5);
    }
    //Pull low trig 3 for 100ms to signal to DUT
    digitalWrite(ATE_TRIG_3_PIN, LOW);
    delay(100);
    digitalWrite(ATE_TRIG_3_PIN, HIGH);

    //Start the second sequence
    //Pull high Trig 1 and pull low trig 2 for 100ms to signal to DUT

    digitalWrite(ATE_TRIG_1_PIN, HIGH);
    digitalWrite(ATE_TRIG_2_PIN, LOW) ;
    delay(100);

    //Start the third sequence 
    //Pull low trig 1 and pull high trig 2 for 100ms to signal to DUT

    digitalWrite(ATE_TRIG_1_PIN, LOW);
    digitalWrite(ATE_TRIG_2_PIN, HIGH);
    delay(100);

    //Reconfig trig 1,2,3
    //Config trig 3 to input
    pinMode(ATE_TRIG_3_PIN, INPUT);
    ATE_in_HT_mode_flag = true;
  }

  }


/******************************************************************************
* Function       : void execute_cmd (void)
* Description    : This function is used to execute hardware test mode           
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void execute_cmd (uint8_t id)
{
  uint16_t msgSize    = 0;
  uint16_t msg_status = 0;

  while(1)
  {
  memset(rx_buff, 0, sizeof(rx_buff));
  memset(tx_buff, 0, sizeof(tx_buff));

  msgSize = ATE_comm_uart_rx_receive_msg(rx_buff, true);

    if (msgSize > 0)
    {
      msg_status = ATE_comm_validate_msg_pc(rx_buff, msgSize);
      if (msg_status == 0)
      {

        if (ATE_COMM_MSG_ID_ACK == rx_buff[0])
        {
          //Send acknowledgement to DUT 
          ATE_send_response(ATE_COMM_MSG_ID_NACK, rx_buff[0], rx_buff[msgSize-4], 0);
        }
        else if (ATE_COMM_MSG_ID_NACK == rx_buff[0])
        {
          //Send Non-acknowledge to DUT
          ATE_send_response(ATE_COMM_MSG_ID_NACK, rx_buff[0], rx_buff[msgSize-4], 0);
        }


        switch (id)
        {
          case ATE_COMM_MSG_ID_PING:
            ATE_send_ping_msg;
            break;
          case ATE_COMM_MSG_ID_REQ: 
            ATE_send_request_msg;
          default:
            break;
        }
      }
    }
  }

  ATE_comm_wait_for_dut_response(tx_buff, msgSize);
}

/******************************************************************************
* Function       : void ATE_send_ping_msg (void)
* Description    : This function is used to send ping message           
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void ATE_send_ping_msg(void)
{
  uint16_t msgSize = 5;

  tx_buff[0] = ATE_COMM_MSG_ID_PING;
  tx_buff[1] = ATE_comm_increase_seq_num();
  tx_buff[2] = ATE_comm_cal_crc8(tx_buff, msgSize);
  tx_buff[3] = ATE_COMM_SYMBOL_SYNC1;
  tx_buff[4] = ATE_COMM_SYMBOL_SYNC2;

  //Send the ping message
  ATE_comm_uart_tx_send_string((uint8_t *) tx_buff, msgSize);
}

/******************************************************************************
* Function       : void ATE_send_request_msg (void)
* Description    : This function is used to send ping message           
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void ATE_send_request_msg(void)
{
  uint16_t msgSize = ATE_payload_length + 5;

  tx_buff[0] = ATE_COMM_MSG_ID_REQ;
  tx_buff[1] = lowByte(ATE_payload_length);
  tx_buff[2] = highByte(ATE_payload_length);
  tx_buff[3] = ATE_component_ID; 
  tx_buff[4] = ATE_command_ID;
  memcpy (tx_buff + 5, ATE_test_buff, ATE_payload_length - 3);
  tx_buff[msgSize-3] = ATE_comm_cal_crc8(tx_buff, msgSize);
  tx_buff[msgSize-2] = ATE_COMM_SYMBOL_SYNC1;
  tx_buff[msgSize-1] = ATE_COMM_SYMBOL_SYNC2;

  //Send the message
  ATE_comm_uart_tx_send_string((uint8_t *) tx_buff, msgSize);
}

/******************************************************************************
* Function       : static void ATE_send_response (uint8_t msgID, 
*                                                 uint8_t oriMsgId, 
*                                                 uint8_t oriSeqNum, 
*                                                 uint8_t error)
* Description    : This function is used to build response and           
*                  send this through UART to DUT
* Input          : msgID     - message ID
*                  origMsgId - original message ID
*                  error     - error condition
* Output         : None.
* Return         : None.
*******************************************************************************/
static void ATE_send_response (uint8_t msgID, uint8_t oriMsgId, uint8_t oriSeqNum, uint8_t error)
{
  uint16_t msgSize = 0;

  tx_buff[0] = msgID;
  tx_buff[1] = oriMsgId;
  tx_buff[2] = oriSeqNum;

  if (msgID == ATE_COMM_MSG_ID_ACK)
  {
    tx_buff[3] = ATE_comm_increase_seq_num();
    tx_buff[4] = 0xff;
    tx_buff[5] = ATE_COMM_SYMBOL_SYNC1;
    tx_buff[6] = ATE_COMM_SYMBOL_SYNC2;

    msgSize = 7;
    tx_buff[4] = ATE_comm_cal_crc8(tx_buff, msgSize);
  }
  else 
  {
    tx_buff[3] = error;
    tx_buff[4] = ATE_comm_increase_seq_num();
    tx_buff[5] = 0xff;
    tx_buff[6] = ATE_COMM_SYMBOL_SYNC1;
    tx_buff[7] = ATE_COMM_SYMBOL_SYNC2;

    msgSize = 8; 
    tx_buff[5] = ATE_comm_cal_crc8(tx_buff, msgSize);
  }

  ATE_comm_uart_tx_send_string((uint8_t *) tx_buff, msgSize);
}

/******************************************************************************
* Function       : static void execute_response (void)
* Description    : This function is used to execute           
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void execute_res (uint8_t *rx_msg, uint8_t msgSize)
{
  uint16_t Res_msgSize = msgSize +3 ;

  ATE_result_buff[0] = 'A';
  ATE_result_buff[1] = 'T';
  ATE_result_buff[2] = 'E';
  ATE_result_buff[3] = 'R';
  ATE_result_buff[4] = rx_msg[1];
  ATE_result_buff[5] = rx_msg[2];
  ATE_result_buff[6] = rx_msg[3];
  ATE_result_buff[7] = rx_msg[4];
  for( int z=8; z < Res_msgSize - 4; z++)
  {
    ATE_result_buff[z]= rx_msg[z-3];
  }
  ATE_result_buff[Res_msgSize-4] = '$';
  ATE_result_buff[Res_msgSize-3] = '$';
  ATE_result_buff[Res_msgSize-2] = '$';
  ATE_result_buff[Res_msgSize-1] = '$';

  Serial.write( ATE_result_buff, Res_msgSize);
}
