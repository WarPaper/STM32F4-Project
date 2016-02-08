
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

bool isSlave = 0;       //If master --> isSlave = 0

#if isSlave
uint8_t slave_no = 11;
#else
uint8_t slave_no = 10;
#endif

uint8_t task_total = 20;
uint8_t task_store[] = "";

volatile uint16_t ticker, downTicker = 0, downTickerH;

/*
* The USB data must be 4 byte aligned if DMA is enabled. This macro handles
* the alignment, if necessary (it's actually magic, but don't tell anyone).
*/
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

void ssssssss(char[],char[]);
bool f_ssssssss = 0;

void init(void);
void ColorfulRingOfDeath(void);

void SR_Protocol(char[],char[],char[],uint16_t,uint8_t);
char receive_(void);

void push_status(void);
uint8_t Message_block(char[],char[],char[]);
void Send_Message_block(char []);
void uniq_Random_task(uint8_t);

char *itoa(uint8_t);

void waitFor (unsigned int);
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

void main(void)
{
  /* Set up the system clocks */
  SystemInit();
  
  /* Initialize USB, IO, SysTick, and all those other things you do in the morning */
  init();
  
//  char *ssss = (char *)malloc(sizeof(char));
  
  while (1)
  {
    uint8_t theByte;
    
    if (VCP_get_char(&theByte))
    {
      SR_Protocol("m","all","0001",2000,3);
      SR_Protocol("m","cdc","0003",1000,3);
      //      push_status();
      //      Message_block();
      //      uniq_Random_task(task_total);
      
      //      VCP_send_str(aaaa("m","aa","bb"));
      //      free(aaaa("m","aa","bb"));
      
      // do some thing usefull
      
      //            ssss = itoa(128);
      //            free(itoa(128));
      //            VCP_send_str(ssss);
      //            free(ssss);
      //            
      //            VCP_put_char('\n');        
      //            
      //            ssss = itoa(sizeof(128));
      //            free(itoa(sizeof(128)));
      //            VCP_send_str(ssss);
      //            free(ssss);   
      //            
      //            VCP_put_char('\n');
      //            VCP_put_char(theByte);
      //            downTicker = 100;
    }
    //    bool p = false;
    //    if(downTicker == 1 && p == false){
    //      VCP_send_str("delayed");
    //      p = true;
    //    }else p = false;
  }
}

void ssssssss(char *a,char *b){
//  char *ssss; 
//  if(ssss != NULL){
//    ssss = (char *)malloc(sizeof(char)); 
//    VCP_send_str("in ssssNULL");
//  }else{
//    VCP_send_str("in ssssNONULL");
//    ssss = itoa(128);
//    free(itoa(128));
//    VCP_send_str(ssss);
//    free(ssss);
//    
//    VCP_put_char('\n');        
//    
//    ssss = itoa(sizeof(128));
//    free(itoa(sizeof(128)));
//    VCP_send_str(ssss);
//    free(ssss);   
//    
//    VCP_put_char('\n');
//    
//      uint8_t cn = 5;
//  
//  Message_block("m","all","mASTER");
//  downTicker = 10;
//  while(cn > 0){
//     
//    if(downTicker == 1){
//      cn--;
//      VCP_send_str("w8ing 4 re");
//      downTicker = 10;
//    } 
//  }
//  VCP_send_str("w8ing 4 redsasadasdsad");
//  VCP_send_str(a);
  
//    char pp[]="";
  
  char *aa =(char *)calloc(sizeof(char),sizeof(a));
  if(aa != NULL){
    strcat(aa,"|");
    strcat(aa,a);
    strcat(aa,b);
    strcat(aa,"@");
    VCP_send_str(aa);
    free(aa);
  } else {
    VCP_send_str("itoa out of memory");
  }
}

void SR_Protocol(char *w_ho, char *t_o, char *d_t,uint16_t deLay,uint8_t counter){                        //Broadcast Intro
  bool flag  = 0,received = 0;
  uint8_t cn = counter;                               //Receiving Counter
  
  while(cn > 0){                                //w8ing 4 re
    
    if(receive_() != NULL){
      received = 1;
      cn = 0;
    }
    
    if(downTicker == 0 && received != 1){                        //resend "all" = broadcast
      cn--;
      while(flag == 0){
        
        
        if(Message_block(w_ho,t_o,d_t) != 0)downTicker = deLay;
        
               
        flag = 1;
      }
      flag = 0;
    }
  }
  
  if(received == 0){
    VCP_put_char('\n');
    VCP_send_str("no receive");
  }else {
    VCP_put_char('\n');
    VCP_send_str("received");
  }
  VCP_put_char('\n');

}
char receive_(){
  uint8_t theByte = NULL;
  
  if(VCP_get_char(&theByte)){
    if(theByte == 'w'){
      VCP_send_str("get! | ");
      VCP_put_char(theByte);
      return theByte;
    }
  }else{
    return NULL;
  }
  
}
void push_status(void){
  VCP_send_str("in <Status>");
  VCP_put_char('\n');
  //  char Slave_details[] = "Slave_No: ";
  char *Slave_details = (char *)calloc(5,sizeof(char));
  Slave_details = itoa(slave_no);
  //  free(itoa(slave_no));
  strcpy(Slave_details, itoa(slave_no));
  //  free(itoa(slave_no));
  VCP_send_str(Slave_details);
  //  free(Slave_details);
  VCP_put_char('\n');
  VCP_send_str(itoa(slave_no)); 
  VCP_put_char('\n');
}

uint8_t Message_block(char *w_ho, char *t_o, char *d_t){       //Message Block for Transfer Data (who,to,data)
  char *pp = (char *)calloc(sizeof(char),sizeof(char)*15);
  if(pp!=NULL){
    strcat(pp,"|");
    //1st_Send From(1 byte) : m(Master) or s(Slave)
    strcat(pp,w_ho);
    strcat(pp,"|");
    //2nd_Slave Destination(3 byte) : all,s01,s02,s03,...
    strcat(pp,t_o);
    strcat(pp,"|");
    //3rd_Data(3 byte) : 001,002,003,...
    strcat(pp,d_t);
    VCP_send_str(pp);
    free(pp);
    VCP_put_char('\n');
    return atoi(d_t);
  }else{
    VCP_send_str("itoa out of memory");
    return 0000;
  }

//  VCP_send_str("heer1");
//  if(sssM_B!=NULL){
//    VCP_send_str("heer2");
//    sssM_B = (char *)malloc(sizeof(w_ho)+sizeof(t_o)+sizeof(d_t));
//  }else{
//    VCP_send_str("heer3");
//  char *ssss = (char *)malloc(sizeof(char));
//  char *sss = (char *)malloc(sizeof(char)*5);
//  if(ssss||sss!=NULL){
//    if(ssss!=NULL){
//      ssss = (char *)malloc(sizeof(char));
//    }else{
//      sss = (char *)malloc(sizeof(char)*5);
//    }
//  }else
    //  char Message_frame[] = "";    //Message_frame = null
 
  
  //1st_Type(1 byte) : M(Master) or S(Slave)
//  ssss = w_ho;
//  if(isSlave == 1)strcat(sssM_B, "s");
//  else strcat(sssM_B, w_ho);
//  free(ssss);
//  VCP_send_str(sssM_B);
//  VCP_put_char('\n');
  //  strcat(sss, "|");
  //  //2nd_No(1 byte) : M = null or S = 0,1,2,..
  //  ssss = itoa(slave_no);
//  memmove(sssM_B,t_o,strlen(t_o));
//    strcat(sss, t_o);
  //  
  //  strcat(sss, "|");
  //  //3rd_howmanyTask(3 byte) : 001,002,003,...
  //  ssss = itoa(task_total);
  //  strcat(sss, ssss);
  //  free(ssss);
  
//  VCP_send_str(sssM_B);
//  free(sssM_B);
//  VCP_put_char('\n');
 
}
void Send_Message_block(char a[]){
  VCP_send_str(a);
}

void uniq_Random_task(uint8_t task_max){ 
  uint8_t task_tmp;
  
  VCP_send_str("in <uRtask>");
  VCP_put_char('\n');
  
  srand ( time(NULL) );
  task_store[0] = rand() % task_max;
  
  for(uint8_t i=1;i<task_max;i++){      //Unique Loop
    task_tmp = rand() % task_max;
    for(uint8_t l=0;l<i;l++){
      if(task_tmp != task_store[l]){
        task_store[i] = task_tmp;
        VCP_put_char('\0');             //need to write something for delay !?@!?!?
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

char* itoa(uint8_t x){                  //Convert Int to String
  char *strrr = (char *)malloc(sizeof(x));
  if(strrr != NULL){
    sprintf(strrr,"%d", x);
    return strrr;
  } else {
    return "itoa out of memory";
    
    //return integer x to string & interger x
  }
  /* 
  Don't forget!!! 
  free(itoa(x));
  free(ssss);  *ssss is the char * that hold return string;
  */
}

void waitFor(unsigned int milliseconds){
  unsigned int pause;
  clock_t now,then;
  
  pause = milliseconds*(CLOCKS_PER_SEC/1000);
  now = then = clock();
  while( (now-then) < pause )
    now = clock();
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
  ticker++;                   //ticker time 100 ~= 1 sec.
  if (downTicker > 0){
    downTicker--;
  }
  if (downTickerH > 0){
    downTickerH--;
  }
}

void NMI_Handler(void)       {}
void HardFault_Handler(void) { 
  printf("HardFault_Handler!\n");
  ColorfulRingOfDeath(); 
}
void MemManage_Handler(void) { 
  printf("MemManage_Handler!\n");
  ColorfulRingOfDeath(); 
}
void BusFault_Handler(void)  { 
  printf("BusFault_Handler!\n");
  ColorfulRingOfDeath(); 
}
void UsageFault_Handler(void){ 
  printf("UsageFault_Handler!\n");
  ColorfulRingOfDeath(); 
}
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
