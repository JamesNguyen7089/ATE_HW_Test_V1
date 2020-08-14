/*******************************************************************************
* Title                 :   
* Filename              :   ATE_HW_TEST.ino
* Author                :   Quoc Tran
* Created Date          :   Aug 07, 2020
* Notes                 :   
*******************************************************************************/

///////////INCLUDE///////////
#include <stdint.h>
#include <string.h>
#include "HW_Test_Config.h"
#include "ATE_Comm.h"
#include "HW_Test.h"

void setup() {
  // put your setup code here, to run once:
  
  

  //UART Communication
  Serial.begin(9600);
  Serial1.begin(9600);//Initialize serial with a baud rate of 9600 bps and wait for port to open
  while (!Serial) 
  {
    ; // wait for serial port to connect.
  }

  
}

void loop() {
  // put your main code here, to run repeatedly:
  ATE_enter_HT_mode();
  while(ATE_in_HT_mode_flag)
  {
      
      receive_cmd(); 
        
          //TODO
          execute_cmd(ATE_message_ID); 
        
      
      receive_res();        
     ATE_in_HT_mode_flag = false; 
  }
}
