
#define HSE_VALUE ((uint32_t)8000000) /* STM32 discovery uses a 8Mhz external crystal */

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"

#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdio.h"
#include "time.h"

#define isSlave 0       //If master --> slave = 0

#if isSlave
uint8_t slave_no = 1;
#else
uint8_t slave_no = 0;
#endif
uint8_t task_total = 20;

uint8_t task_store[] = "";

volatile uint32_t ticker, downTickerW, downTickerA, downTickerD, downTickerS;

/*
* The USB data must be 4 byte aligned if DMA is enabled. This macro handles
* the alignment, if necessary (it's actually magic, but don't tell anyone).
*/
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

void init(void);
void ColorfulRingOfDeath(void);

void Slave_status(void);
void Message_block(void);

void uniq_Random_task(uint8_t);
char* itoa(uint8_t);

/*
* Define prototypes for interrupt handlers here. The conditional "extern"
* ensures the weak declarations from startup_stm32f4xx.c are overridden.
*/
#ifdef __cplusplus
extern "C" {
#endif
  
  void SysTick_Handler(void);
  void NMI_Handler(void);
  void HardFault_Handler(void);
  void MemManage_Handler(void);
  void BusFault_Handler(void);
  void UsageFault_Handler(void);
  void SVC_Handler(void);
  void DebugMon_Handler(void);
  void PendSV_Handler(void);
  void OTG_FS_IRQHandler(void);
  void OTG_FS_WKUP_IRQHandler(void);
  
  
#ifdef __cplusplus
}
#endif


int main(void)
{
  /* Set up the system clocks */
  SystemInit();
  
  /* Initialize USB, IO, SysTick, and all those other things you do in the morning */
  init();
  Message_block();
  
  while (1)
  {
    uint8_t theByte;
    
    if (VCP_get_char(&theByte))
    {
      Slave_status();
      Message_block();
      uniq_Random_task(task_total);
      
      
    }
  }
  
}


void init()
{
  /* STM32F4 discovery LEDs */
  GPIO_InitTypeDef LED_Config;
  
  /* Always remember to turn on the peripheral clock...  If not, you may be up till 3am debugging... */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  LED_Config.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
  LED_Config.GPIO_Mode = GPIO_Mode_OUT;
  LED_Config.GPIO_OType = GPIO_OType_PP;
  LED_Config.GPIO_Speed = GPIO_Speed_25MHz;
  LED_Config.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &LED_Config);
  
  /* Setup SysTick or CROD! */
  if (SysTick_Config(SystemCoreClock / 1000))
  {
    ColorfulRingOfDeath();
  }
  /* Setup USB */
  USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);
  
  return;
}

void Slave_status(void){
  VCP_send_str("in <Status>");
  VCP_put_char('\n');
  char Slave_details[] = "Slave_No: ";
  itoa(slave_no);
  //  sprintf(sc_tmp,"%d", slave_no);       //Convert int to string
  strcat(Slave_details, itoa(slave_no));
  VCP_send_str(Slave_details);
  VCP_put_char('\n');
  
}
void Message_block(void){       //Message Block for Transfer Data
  char Message_frame[] = "";   //Message_frame = null
  VCP_send_str("in <MBlock>");
  VCP_put_char('\n');
  //1st_Type(1 byte) : M(Master) or S(Slave)
  if(isSlave == 1)strcat(Message_frame, "s");
  else strcat(Message_frame, "m");
  
  //2nd_No(1 byte) : M = null or S = 0,1,2,..  
  strcat(Message_frame, itoa(slave_no));
  
  //3rd_howmanyTask(3 byte) : 001,002,003,...
  strcat(Message_frame, itoa(task_total));
  
  VCP_send_str(Message_frame);
  VCP_put_char('\n');
}

void uniq_Random_task(uint8_t task_max){ 
  char ch_tmp[30];
  
  uint8_t task_tmp;
  
  VCP_send_str("in <uRtask>");
  VCP_put_char('\n');
  
  srand ( time(NULL) );
  task_store[0] = rand() % task_max;
  
  for(uint8_t i=1;i<task_max;i++){
    task_tmp = rand() % task_max;
    for(uint8_t l=0;l<i;l++){
      if(task_tmp != task_store[l]){
        task_store[i] = task_tmp;
        VCP_put_char('\0');           //need to write something for delay !?@!?!?
      }else{
        i--;
        l=i;
      }
    }
  }
  
  
  VCP_send_str("split Task: ");
  for(uint8_t k=0;k<task_max;k++){
    VCP_send_str(itoa(task_store[k]));
    VCP_put_char(',');
  }
}

char* itoa(uint8_t x){          //Convert Int to String
  char char_tmp[30];
  sprintf(char_tmp,"%d", x);
  return char_tmp;
}


/*
* Call this to indicate a failure.  Blinks the STM32F4 discovery LEDs
* in sequence.  At 168Mhz, the blinking will be very fast - about 5 Hz.
* Keep that in mind when debugging, knowing the clock speed might help
* with debugging.
*/
void ColorfulRingOfDeath(void)
{
  uint16_t ring = 1;
  while (1)
  {
    uint32_t count = 0;
    while (count++ < 500000);
    
    GPIOD->BSRRH = (ring << 12);
    ring = ring << 1;
    if (ring >= 1<<4)
    {
      ring = 1;
    }
    GPIOD->BSRRL = (ring << 12);
  }
}

/*
* Interrupt Handlers
*/

void SysTick_Handler(void)
{
  ticker++;
  if (downTickerW > 0)
  {
    downTickerW--;
  }
  if (downTickerA > 0)
  {
    downTickerA--;
  }
  if (downTickerD > 0)
  {
    downTickerD--;
  }
  if (downTickerS > 0)
  {
    downTickerS--;
  }
}

void NMI_Handler(void)       {}
void HardFault_Handler(void) { ColorfulRingOfDeath(); }
void MemManage_Handler(void) { ColorfulRingOfDeath(); }
void BusFault_Handler(void)  { ColorfulRingOfDeath(); }
void UsageFault_Handler(void){ ColorfulRingOfDeath(); }
void SVC_Handler(void)       {}
void DebugMon_Handler(void)  {}
void PendSV_Handler(void)    {}

void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}
