/*******************************************************************************
* Title                 :   HW Test 
* Filename              :   ATE_Comm.h
* Author                :   Quoc Tran
* Created Date          :   Aug 07, 2020
* Notes                 :   
*******************************************************************************/

#ifndef ATE_COMM_H_
#define ATE_COMM_H_

/////////////////////INCLUDE/////////////////////
#include <arduino.h>
#include <string.h>
#include <stdint.h>
/////////////////////PARAMETERS////////////////////
#define ATE_COMM_MSG_ID_ACK         (0xC1)
#define ATE_COMM_MSG_ID_NACK        (0xC2)
#define ATE_COMM_MSG_ID_REQ         (0x21)
#define ATE_COMM_MSG_ID_PING        (0x28)
#define ATE_COMM_MSG_ID_REQ_STATUS  (0x81)
#define ATE_COMM_SYMBOL_SYNC1       (0xBD)
#define ATE_COMM_SYMBOL_SYNC2       (0xDB)

/////////////////////VARIABLE DEFINITIONS////////////////////
extern uint8_t ATE_comm_seq_num;
extern bool ATE_footer_flag ;
extern long  ATE_param ;
/////////////////////FUNCTION PROTOTYPES////////////////////
void ATE_comm_uart_tx_send_string (uint8_t* str, uint16_t msgSize); 
uint8_t ATE_comm_uart_rx_receive_char (bool wait);
uint16_t ATE_comm_uart_rx_receive_msg (uint8_t* msg, bool wait); 
uint8_t ATE_comm_increase_seq_num (void);
void ATE_comm_wait_for_dut_response (uint8_t* msg, uint16_t msgSize);
uint8_t ATE_comm_cal_crc8 (uint8_t* msg, uint16_t msgSize); 
bool ATE_comm_validate_msg_id (uint8_t id); 
uint16_t ATE_comm_validate_msg(uint8_t* msg, uint16_t msgSize);
uint16_t ATE_comm_validate_msg_pc(uint8_t* msg, uint16_t msgSize);
static bool ATE_comm_check_crc8(uint8_t* msg, uint16_t msgSize); 
void RestoPC(uint8_t *msg);
void receive_cmd(void);
void receive_res(void);
#endif // ATE_COMM_H_
