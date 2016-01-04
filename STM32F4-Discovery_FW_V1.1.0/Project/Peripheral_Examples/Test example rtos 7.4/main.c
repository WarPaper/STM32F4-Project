#include <stm32f4xx.h>
#include <misc.h>                         // I recommend you have a look at these in the ST firmware folder
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "stm32f4_discovery.h"

#define MAX_STRLEN 24 // this is the maximum string length of our string in characters

volatile char received_string[MAX_STRLEN+1]; // this will hold the recieved string
char buffer[MAX_STRLEN+1];
void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}
/* Use this function to enable "printf" */
//#ifdef __GNUC__
///* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
//set to 'Yes') calls __io_putchar() */
//#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
//#else
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
//#endif /* __GNUC__ */
/* (END) Use this function to enable "printf" */

/* Use this function to use "printf" for send USARTx */
//PUTCHAR_PROTOTYPE
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the USART */
//  USART_SendData(USART3, (uint8_t) ch);
//  /* Loop until transmit data register is empty */
//  while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
//  {}
//  return ch;
//}
/* (END) Use this function to use "printf" for send USARTx */

void init_usersw_led(){
  
  GPIO_InitTypeDef GPIO_InitStruct;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;		  // we want to configure PA0
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN; 	  // we want it to be an input
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//this sets the GPIO modules clock speed
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   // this sets the pin type to push / pull (as opposed to open drain)
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;   // this enables the pulldown resistor --> we want to detect a high level
  GPIO_Init(GPIOA, &GPIO_InitStruct);	
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void init_USART1(uint32_t baudrate){
  
  GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
  USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
  NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins 6 (TX) and 7 (RX) are used
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;                         // the pins are configured as alternate function so the USART peripheral has access to them
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;  //50            // this defines the IO speed and has nothing to do with the baudrate!
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                        // this defines the output type as push pull mode (as opposed to open drain)
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;                        // this activates the pullup resistors on the IO pins
  GPIO_Init(GPIOB, &GPIO_InitStruct);                                        // now all the values are passed to the GPIO_Init() function which sets the GPIO registers
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
  
  USART_OverSampling8Cmd(USART1, ENABLE);
  
  /* USART_InitStruct is used to define properties of USART1 */
  
  USART_InitStruct.USART_BaudRate = baudrate;                                // the baudrate is set to the value we passed into this init function
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
  USART_InitStruct.USART_StopBits = USART_StopBits_1;                // we want 1 stop bit (standard)
  USART_InitStruct.USART_Parity = USART_Parity_No;                // we don't want a parity bit (standard)
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
  USART_Init(USART1, &USART_InitStruct);                                        // again all the properties are passed to the USART_Init function which takes care of all the bit setting
  
  /* Here the USART1 receive interrupt is enabled
  * and the interrupt controller jump to the USART1_IRQHandler() function
  */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt 
  
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;                 // we want to configure the USART1 interrupts
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                 // this sets the subpriority inside the group
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         // the USART1 interrupts are globally enabled
  NVIC_Init(&NVIC_InitStructure);                                                         // the properties are passed to the NVIC_Init function which takes care of the low level stuff        
  
  USART_Cmd(USART1, ENABLE);
}

void init_USART3(uint32_t baudrate){
  
  GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
  USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
  NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8; // (TX) are used
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;                         // the pins are configured as alternate function so the USART peripheral has access to them
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;  //50            // this defines the IO speed and has nothing to do with the baudrate!
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                        // this defines the output type as push pull mode (as opposed to open drain)
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;                        // this activates the pullup resistors on the IO pins
  GPIO_Init(GPIOD, &GPIO_InitStruct);                                        // now all the values are passed to the GPIO_Init() function which sets the GPIO registers
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11; // (RX) are used
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;                         // the pins are configured as alternate function so the USART peripheral has access to them
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;  //50            // this defines the IO speed and has nothing to do with the baudrate!
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                        // this defines the output type as push pull mode (as opposed to open drain)
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;                        // this activates the pullup resistors on the IO pins
  GPIO_Init(GPIOB, &GPIO_InitStruct);           
  
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3); 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);
  
  USART_OverSampling8Cmd(USART3, ENABLE);
  
  /* USART_InitStruct is used to define properties of USART3 */
  
  USART_InitStruct.USART_BaudRate = baudrate;                                // the baudrate is set to the value we passed into this init function
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
  USART_InitStruct.USART_StopBits = USART_StopBits_1;                // we want 1 stop bit (standard)
  USART_InitStruct.USART_Parity = USART_Parity_No;                // we don't want a parity bit (standard)
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
  USART_Init(USART3, &USART_InitStruct);                                        // again all the properties are passed to the USART_Init function which takes care of all the bit setting
  
  /* Here the USART3 receive interrupt is enabled
  * and the interrupt controller jump to the USART3_IRQHandler() function
  */
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt 
  
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;                 // we want to configure the USART1 interrupts
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                 // this sets the subpriority inside the group
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         // the USART1 interrupts are globally enabled
  NVIC_Init(&NVIC_InitStructure);                                                         // the properties are passed to the NVIC_Init function which takes care of the low level stuff        
  
  USART_Cmd(USART3, ENABLE);
}

void USART_puts(USART_TypeDef* USARTx, volatile char *s){
  
  while(*s){
    // wait until data register is empty
    while( !(USARTx->SR & 0x00000040) ); 
    USART_SendData(USARTx, *s);
    *s++;
  }
}

static uint8_t lastButtonStatus = RESET;

int main(void) {
  init_usersw_led();
  init_USART1(115200);
  init_USART3(115200);
  //printf("Init USART   Press User_Button to Send data\n");
  USART_puts(USART1,"Init USART1 Complete!\n");
  USART_puts(USART3,"Init USART3 Complete!\n");
  GPIO_SetBits(GPIOD,GPIO_Pin_13);
  GPIO_ResetBits(GPIOD,GPIO_Pin_14);
  GPIO_ResetBits(GPIOD,GPIO_Pin_15);
  
  while(1){
    uint8_t currentButtonStatus = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);
    
    if (lastButtonStatus != currentButtonStatus && currentButtonStatus != RESET){
    GPIO_ToggleBits(GPIOD,GPIO_Pin_14);
    GPIO_ToggleBits(GPIOD,GPIO_Pin_15);
      
      USART_puts(USART1,"Puts USART1!\n");
      USART_puts(USART3,"Puts USART3!\n");
//      printf("Init complete! Sum_Million Test!\n");
    }
    lastButtonStatus = currentButtonStatus;
    GPIO_ResetBits(GPIOD,GPIO_Pin_14);
    GPIO_ResetBits(GPIOD,GPIO_Pin_15);
  }
}

void USART1_IRQHandler(void){
  if( USART_GetITStatus(USART1, USART_IT_RXNE) ){
    GPIO_ToggleBits(GPIOD,GPIO_Pin_14);
    static uint8_t cnt1 = 0; // this counter is used to determine the string length
    char t1 = USART1->DR; // the character from the USART1 data register is saved in t
    if( (t1 != '\n') && (cnt1 < MAX_STRLEN) ){ 
      received_string[cnt1] = t1;
      cnt1++;
    }
    else{ // otherwise reset the character counter and print the received string
      cnt1 = 0;
      //printf("%s\n",received_string);
    }
  }
}
void USART3_IRQHandler(void){
  if( USART_GetITStatus(USART3, USART_IT_RXNE) ){
    GPIO_ToggleBits(GPIOD,GPIO_Pin_15);
    static uint8_t cnt3 = 0; // this counter is used to determine the string length
    char t3 = USART3->DR; // the character from the USART1 data register is saved in t
    if( (t3 != '\n') && (cnt3 < MAX_STRLEN) ){ 
      received_string[cnt3] = t3;
      cnt3++;
    }
    else{ // otherwise reset the character counter and print the received string
      cnt3 = 0;
      //printf("%s\n",received_string);
    }
  }
}