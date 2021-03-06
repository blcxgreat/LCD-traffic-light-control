#include <tm4c123gh6pm.h>
#include <stdint.h>

#define INITIATE (*((unsigned long*)0x400FE608))
#define GPIOF_DIR (*((unsigned long*)0x40025400))
#define GPIOF_DEN (*((unsigned long*)0x4002551C))
#define GPIOF_DATA (*((unsigned long*)0x400253FC))

int count = 0;

void Timer_Init(void) {
SYSCTL_RCGCTIMER_R |= 0x01;   // Enable the appropriate timer bit
TIMER0_CTL_R = 0x00;
TIMER0_CFG_R = 0x00000000;
TIMER0_TAMR_R = 0x02;
TIMER0_TAILR_R = 0x00F42400;
TIMER0_IMR_R = 0x01;
NVIC_EN0_R |= (1<<19);
}

void Timer_Handler(void) {
    TIMER0_ICR_R = 0x01;
    ++count;
    if(count == 3){
      count = 0;
    }
}

int main()
{
  INITIATE = 0x20;  //enable Port F GPIO
  GPIOF_DIR = 0x0E;  //Set portF as output
  GPIOF_DEN = 0x0E;  //Enable digital port F
  Timer_Init();
  TIMER0_CTL_R = 0x01;
  
  while (1) {
    if(count == 0){
      GPIOF_DATA = 0x02;
    } else if (count == 1){
      GPIOF_DATA = 0x04;
    } else {
      GPIOF_DATA = 0x08;
    }
  }
  return 0;
}  
