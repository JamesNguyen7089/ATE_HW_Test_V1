/*******************************************************************************
* Title                 :   HW Test 
* Filename              :   ATE_Comm.cpp
* Author                :   Quoc Tran
* Created Date          :   Aug 07, 2020
* Notes                 :   
*******************************************************************************/

/////////////////////INCLUDE/////////////////////
#include <stdint.h>
#include <string.h>
#include "ATE_Comm.h"
#include <arduino.h>
#include "HW_Test_Config.h"
#include "HW_Test.h"
/////////////////////PARAMETERS////////////////////
#define ATE_UART_TX_SEND_TIMEOUT          (10000)
#define ATE_UART_RX_RECEIVE_TIMEOUT       (1)

/////////////////////VARIABLE DEFINITIONS////////////////////
uint8_t ATE_comm_seq_num             = 0;
bool    ATE_footer_flag = false;
long    ATE_param;
String  InFrame , header, footer;
/////////////////////FUNCTION PROTOTYPES////////////////////


//////////////////////////FUNCTIONS/////////////////////////
/******************************************************************************
* Function       : void ATE_comm_uart_tx_send_string (uint8_t* str, uint16_t msgSize)
* Description    : This function is used to send the input string through UART          
* Input          : str     - pointer to sent string 
*                  msgSize - size of input string 
* Output         : None.
* Return         : None.
*******************************************************************************/
void ATE_comm_uart_tx_send_string (uint8_t* str, uint16_t msgSize)
{
  Serial1.write(str, msgSize);
  Serial1.write("\n\r");
}

/******************************************************************************
* Function       : uint8_t ATE_comm_uart_rx_receive_char (bool wait)
* Description    : This function is used to receive a character from UART RX          
* Input          : wait - true  : wait until we get a character, 
*                         false : if not having any character, exit from function 
* Output         : None.
* Return         : Received character. 
*******************************************************************************/
uint8_t ATE_comm_uart_rx_receive_char (bool wait)
{
  uint8_t data = 0;
  uint16_t timeout = ATE_UART_RX_RECEIVE_TIMEOUT;
  if(wait == true)
  {
    timeout = 10000;
  }
  //Serial.setTimeout(timeout);
   Serial1.setTimeout(timeout);
  //Serial.readBytes(&data, 1);
   Serial1.readBytes(&data, 1);
  return data;
}

/******************************************************************************
* Function       : uint16_t ATE_comm_uart_rx_receive_msg (uint8_t* msg, bool wait)
* Description    : This function is used to receive message from UART         
* Input          : msg - Pointer to array to store received message
*                  wait - true  : wait until we get a character, 
*                         false : if not having any character, exit from function 
* Output         : None.
* Return         : Total length of received message.
*******************************************************************************/
uint16_t ATE_comm_uart_rx_receive_msg (uint8_t* msg, bool wait)
{
  uint16_t index          = 0;
  uint16_t counter        = 0;
  uint16_t payload_len    = 0;
  uint16_t sync_check_len = 0xFFFF;
  uint8_t  rx_char        = NULL;

  // Keep reading until end of message sync bytes are detected
  while(1)
  {
    rx_char = ATE_comm_uart_rx_receive_char(wait);
    msg[index] = rx_char;

    if ((payload_len == 0) && (index == 2))
    {
      if (msg[0] == ATE_COMM_MSG_ID_REQ)
      {
        payload_len = msg[1] + ((msg[2] << 8) & 0xff00);

        sync_check_len = payload_len + 2;
      }
      else
      {
        sync_check_len = 3;
      }
    }

    if (index > sync_check_len)
    {
      if ((msg[index-1] == ATE_COMM_SYMBOL_SYNC1) && (msg[index] == ATE_COMM_SYMBOL_SYNC2))
      {
        return index+1;
      }
    }

    index++;

    if (wait == false)
    {
      counter++;

      if (counter > 17000)
      {
        break;
      }
    }
  }

  return 0;
}

/******************************************************************************
* Function       : uint8_t ht_comm_increase_seq_num(void)
* Description    : This function is used to increase sequence number for message          
* Input          : None. 
* Output         : None.
* Return         : Sequence number.
*******************************************************************************/
uint8_t ATE_comm_increase_seq_num (void)
{
  if (ATE_comm_seq_num == 0xFF)
  {
     ATE_comm_seq_num = 1;
  }
  else
  {
     ++ATE_comm_seq_num;
  }

  return ATE_comm_seq_num;
}

/******************************************************************************
* Function       : void ATE_comm_wait_for_dut_response (uint8_t* msg, uint16_t msgSize)
* Description    : This function is used to wait for response from DUT message.         
* Input          : msg      - Pointer to array stored message ATE would like to response to DUT
*                  msgSize - Message size in bytes
* Output         : None.
* Return         : None. 
*******************************************************************************/
void ATE_comm_wait_for_dut_response (uint8_t* msg, uint16_t msgSize) 
{
  //Waiting for the ACK/NACK response. 
  uint8_t   rx_msg[10]   = {0}; 
  uint16_t  timeout      = 0;
  uint16_t  resend_count = 0;
  bool      rx_msg_size  = 0;
  bool      tx_again     = false;

  //Looking for the ACK/NACK response
  while (resend_count < 2)
  {
    timeout = 0;

    //Resend last message if no response or NACK received 
    tx_again = true;

    while (timeout < 3)
    {
      rx_msg_size = ATE_comm_uart_rx_receive_msg(rx_msg, true);

      if (rx_msg_size > 0)
      {
        //Check ACK received
        if (rx_msg[0] == ATE_COMM_MSG_ID_ACK)
        {
          tx_again = false;
          resend_count = 3;
          break;
        }
      }
      timeout++;
    }

    if (tx_again == true)
    {
      //If no response or NACK is received, transmit the message again
      msg[msgSize-4] = ATE_comm_increase_seq_num();
      msg[msgSize-3] = ATE_comm_cal_crc8(msg, msgSize);
      ATE_comm_uart_tx_send_string(msg, msgSize);
      resend_count++;
    }
  }
  RestoPC(msg);
}

/******************************************************************************
* Function       : uint8_t ATE_comm_cal_crc8(uint8_t* msg, uint16_t msgSize)
* Description    : This function is used to calculate CRC8 for a message
* Input          : msg      - Pointer to message would like to calculate CRC8
*                  msgSize - Size of that message in bytes
* Output         : None.
* Return         : None.
*******************************************************************************/
static const uint8_t crc8_table[256] =
{
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

uint8_t ATE_comm_cal_crc8 (uint8_t* msg, uint16_t msgSize)
{
  uint8_t crc = 0;

  // The initial value is usually zero, but non-zero values are better.
  // You can pick any byte you want, just be sure it's the same every time.
  for (uint16_t i=0; i < msgSize-3; ++i)
  {
     crc = crc8_table[crc ^ msg[i]];
  }

  // Some CRC implementations xor the final value with some other value.
  crc ^= 0xa5;
  return crc;
}

/******************************************************************************
* Function       : bool ATE_comm_validate_msg_id(uint8_t id)
* Description    : This function is used to validate if input ID is a valid ID
* Input          : id - ID need to be validated
* Output         : None.
* Return         : None.
*******************************************************************************/
bool ATE_comm_validate_msg_id (uint8_t id)
{
   // list of valid receive message IDs
   uint8_t msg_ids[] = { ATE_COMM_MSG_ID_PING,
                         ATE_COMM_MSG_ID_REQ,
                         ATE_COMM_MSG_ID_REQ_STATUS,
                         ATE_COMM_MSG_ID_ACK,
                         ATE_COMM_MSG_ID_NACK };

   for (uint16_t i=0; i < sizeof(msg_ids); ++i)
   {
     if (msg_ids[i] == id)
     {
       // Valid message ID
       return true;
     }
   }

   // Invalid message ID
   return false;
}

/******************************************************************************
* Function       : uint16_t ATE_comm_validate_msg(uint8_t* msg, uint16_t msgSize)
* Description    : this function is used to validate input message format
* Input          : msg      - Pointer to message that needed to be validated
*                  msgSize  - Total message size
* Output         : None.
* Return         : Message status: Not recognize, Invalid size or wrong sync bytes, Bad CRC8.
*******************************************************************************/
uint16_t ATE_comm_validate_msg(uint8_t* msg, uint16_t msgSize)
{
 uint16_t msg_status = 0;

 if (ATE_comm_validate_msg_id(msg[0]) == false)
 {
   //Non-recognized Message ID
   msg_status |= 0x01;
 }

 if ((msg[msgSize-2] != ATE_COMM_SYMBOL_SYNC1) || (msg[msgSize-1] != ATE_COMM_SYMBOL_SYNC2))
 {
   //Invalid message size or sync bytes not recognized
   msg_status |= 0x02;
 }

 if (ATE_comm_check_crc8(msg, msgSize) == false)
 {
   //Bad message crc8 checksum
   msg_status |= 0x04;
 }

 //Minimum message length = 5: msgID, seqNum, crc8, syncByte1, syncByte2
 if (msgSize < 5)
 {
   //Invalid message size or sync bytes not recognized
   msg_status |= 0x08;
 }

 return msg_status;  
}

uint16_t ATE_comm_validate_msg_pc(uint8_t* msg, uint16_t msgSize)
{
  uint16_t msg_status = 0;

  if (ATE_comm_validate_msg_id(msg[0]) == false)
  {
    //Non-recognized Message ID
    msg_status |= 0x01;
  }

  if (ATE_footer_flag == false)
  {
    //Invalid footer message
    msg_status |= 0x02;
  }
  
  //Minimum message length = 8: Header, Footer 
  if(msgSize < 4)
  {
    //Invalid message size or footer not recognized
    msg_status |= 0x04;
  }

  return msg_status;
}
/******************************************************************************
* Function       : static bool ATE_comm_check_crc8(uint8_t* msg, uint16_t msgSize)
* Description    : This function is used to check if CRC8 of input message is the same with the CRC8 inside that message
* Input          : msg      - Pointer to message would like to check
*                  msgSize - Actual size of the above message
* Output         : None.
* Return         : None.
*******************************************************************************/
static bool ATE_comm_check_crc8(uint8_t* msg, uint16_t msgSize)
{
  uint8_t crc = ATE_comm_cal_crc8(msg, msgSize);
  if (crc == msg[msgSize-3])
  {
    return true;
  }
  else {
    return false;
  }
}

/******************************************************************************
* Function       : void receive_cmd(void)
* Description    : This function is used to receive message command
* Input          : None.
* Output         : None.
* Return         : None.
* 
*  11/08/2020    : Implement function to receive msg
*******************************************************************************/
uint8_t HexValue[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

void receive_cmd(void)
{
  Serial.println("Enter");
  while(!Serial.available());
  InFrame = Serial.readString();
 
  char inArray[InFrame.length()];
  InFrame.toCharArray(inArray, InFrame.length());

  header = InFrame.substring(0,4);

  uint8_t a = InFrame.length();
  footer = InFrame.substring(a-5, a-1); 


   if( header == "ATEC")
   { 
     ATE_message_ID = 0x21;
   }

   if( header == "ATEP")
   { 
     ATE_message_ID = 0x28;
   }

   if( header == "ACK$" )
   {
      ATE_message_ID = 0xC1;
   }

   if( header == "NACK")
   {
      ATE_message_ID = 0xC2;
   }
   
   if( footer == "$$$$")
   { 
     ATE_footer_flag = true ;  
   }

  ATE_payload_length = (inArray[4] << 8 ) | inArray[5];
  ATE_component_ID   = inArray[6];
  ATE_command_ID     = inArray[7];
  
  if( a > 12)
  { ATE_param= inArray[8]; 
    for( int i =9 ; i < (a-5); i++)
    ATE_param = (ATE_param << 8) | inArray[i];
  }
  else 
  { 
    ATE_param = 0x00;
  }

  ATE_test_buff[0] = ATE_message_ID ;
  ATE_test_buff[1] = lowByte(ATE_payload_length);
  ATE_test_buff[2] = highByte(ATE_payload_length);
  ATE_test_buff[3] = ATE_component_ID;
  ATE_test_buff[4] = ATE_command_ID;
  if(a>12)
  {
    memcpy(ATE_test_buff + 5 , inArray + 8 , a - 12 );
  }
  
  //ATE_comm_validate_msg_id (ATE_message_ID); 
}

/******************************************************************************
* Function       : void receive_res(void)
* Description    : This function is used to receive message response
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void receive_res(void) 
{
  uint16_t msgSize    = 0;
  uint16_t msg_status = 0;

  while (1)
  {
    memset(rx_buff, 0, sizeof(rx_buff));
    memset(tx_buff, 0, sizeof(tx_buff));

    msgSize = ATE_comm_uart_rx_receive_msg(rx_buff, true);
    if (msgSize > 0)
    {
       msg_status = ATE_comm_validate_msg(rx_buff, msgSize);
       if (msg_status == 0)
       {
          if (ATE_COMM_MSG_ID_ACK == rx_buff[0])
          {
            continue;
          }
          else if (ATE_COMM_MSG_ID_NACK == rx_buff[0])
          {
            continue;
          }        
          //memset(ATE_result_buff, 0, sizeof(ATE_result_buff));
          //Process ATE message to send to PC
          execute_res(rx_buff, msgSize);
        }
    }
  } 
}
/******************************************************************************
* Function       : void RestoPC(uint8_t *msg)
* Description    : This function is used to response to PC ACK/NACK.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RestoPC(uint8_t *msg)
{
   if(msg[0] == ATE_COMM_MSG_ID_ACK)
   {
     ATE_result_buff[0] = 'A';
     ATE_result_buff[1] = 'C';
     ATE_result_buff[2] = 'K';
     ATE_result_buff[3] = '$';
   }
   if(msg[0]= ATE_COMM_MSG_ID_NACK)
   {
     ATE_result_buff[0] = 'N';
     ATE_result_buff[1] = 'A';
     ATE_result_buff[2] = 'C';
     ATE_result_buff[3] = 'K';
   }
  ATE_result_buff[4] = '$';
  ATE_result_buff[5] = '$';
  ATE_result_buff[6] = '$';
  ATE_result_buff[7] = '$';
   Serial.write( ATE_result_buff , 8);
}
