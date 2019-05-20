// Lab partB program (Extra cradit version updated)


#include <tm4c123gh6pm.h>
#include <stdint.h>
#include "SSD2119.h"

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F

#define UART_FR_TXFF            0x00000020  /* UART Transmit FIFO Full */
#define UART_FR_RXFE            0x00000010  /* UART Receive FIFO Empty */
#define UART_LCRH_WLEN_8        0x00000060  /* 8 bit word length */
#define UART_LCRH_FEN           0x00000010  /* UART Enable FIFOs */
#define UART_CTL_UARTEN         0x00000001  /* UART Enable */
#define SYSCTL_RCGC1_UART0      0x00000001  /* UART0 Clock Gating Control */
#define SYSCTL_RCGC2_GPIOA      0x00000001  /* port A Clock Gating Control */

double temp;
int pressed;
int ps = 0;
int ns = 0;
char receive;
char* msg = "\n\rEmbedded Systems Lab\n\r";

void Timer0_Init(unsigned long count) {
//SYSCTL_RCGCTIMER_R |= 0x01;   // Enable the appropriate timer bit
SYSCTL_RCGCWTIMER_R |= 1;    /* enable clock to WTimer Block 0 */
WTIMER0_CTL_R = 0;           /* disable WTimer before initialization */
WTIMER0_CFG_R = 0x04;        /* 32-bit option */
WTIMER0_TAMR_R = 0x02;       /* periodic mode and down-counter */
WTIMER0_TAILR_R = count;  /* WTimer A interval load value reg (1 s) */
WTIMER0_CTL_R |= 0x20;       /* timer triggers ADC */
WTIMER0_CTL_R |= 0x01;       /* enable WTimer A after initialization */
}

void UART_Init(void)
{
SYSCTL_RCGCUART_R |= 0x02;            /* activate UART1 */
SYSCTL_RCGCGPIO_R |= 0x02;            /* activate port B */

//while((SYSCTL_PRGPIO_R&0x0002) == 0){};/* ready? */
UART1_CTL_R &= ~UART_CTL_UARTEN;      /* disable UART */
UART1_IBRD_R = 43;/*8; */       /* IBRD = int(16,000,000 / (16 * 115,200)) = int(8.680) */
UART1_FBRD_R = 26;/*44; */      /* FBRD = round(0.680 * 64 ) = 44 (43.552)*/
                         /* 8 bit word length (no parity bits, one stop bit, FIFOs) */
UART1_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
UART1_CC_R = 0x00000000; // Use System Clock for Uar
UART1_CTL_R |= 0x301;       /* enable UART */
GPIO_PORTB_AFSEL_R |= 0x03;           /* enable alt funct on PB1-0 */
GPIO_PORTB_DEN_R |= 0x03;             /* enable digital I/O on PB1-0 */
GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00)+0x00000011; /* configure PB1-0 as UART */
GPIO_PORTB_AMSEL_R &= ~0x03;          /* disable analog functionality on PB */
}

/* UART_InChar
* Wait for new serial port input
* Input: none
* Output: ASCII code for key typed
*
*/
char UART_InChar(void)
{
      while( (UART1_FR_R & UART_FR_RXFE) != 0)
          ;
      return((char)(UART1_DR_R /*& 0xFF*/));
}
 
/* UART_OutChar
* Output 8-bit to serial port
* Input: letter is an 8-bit ASCII character to be transferred
* Output: none
*/
void UART_OutChar(double data)
{
      while((UART1_FR_R & UART_FR_TXFF) != 0)
          ;
      UART1_DR_R = (int)data;
}

void PortF_Init(void) {
SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;  //enable Port F GPIO
while((SYSCTL_PRGPIO_R&0x0020) == 0){};/* ready? */
GPIO_PORTF_DIR_R = 0x0E;  //Set portF as output
GPIO_PORTF_DEN_R = 0x1F;  //Enable digital port F
GPIO_PORTF_DATA_R = 0;  //Clear all port F
GPIO_PORTF_LOCK_R = 0x4C4F434B;   //Unlock the corresponding resister
GPIO_PORTF_CR_R = 0xFF;   //Un-committing the register
GPIO_PORTF_PUR_R = 0x11;   //Control one register under GPIOF_CR_R
}

void PLL_Init(uint32_t freq) {
/* 1) configure the system to use RCC2 for advanced features
    such as 400 MHz PLL and non-integer System Clock Divisor */
SYSCTL_RCC2_R |= 0x80000000;
/* 2) bypass PLL while initializing */
SYSCTL_RCC2_R |= (1<<11);
/* 3) select the crystal value and oscillator source */
SYSCTL_RCC_R = (SYSCTL_RCC_R&~0x7C0)+0x540;       /*selecting crystal value and oscillator source*/
SYSCTL_RCC2_R &= ~0x70;			/* configure for main oscillator*/
/* 4) activate PLL by clearing PWRDN */
SYSCTL_RCC2_R &= ~(1<<13);
/* 5) set the desired system divider and the system divider least significant bit */
SYSCTL_RCC2_R |= (1<<30);  	/* use 400 MHz PLL */
SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000) /* clear system clock divider field */
                + (freq<<22)/*(0x63<<22)    ((0x04<<22) == (0x2<<23))*/;      	    /* configure for 4MHz or 80 MHz clock */ 
/* 6) wait for the PLL to lock by polling PLLLRIS */
while((SYSCTL_RIS_R&0x40)==0){};
/* 7) enable use of PLL by clearing BYPASS */
SYSCTL_RCC2_R &= ~(1<<11);
}

void ADC0_Init(void) { //volatile unsigned long delay;
SYSCTL_RCGCADC_R |= 1;       /* enable clock to ADC0 */
//delay = SYSCTL_RCGCADC_R;             /* Allow time for clock to initialize*/
//delay(1000000);
ADC0_ACTSS_R &= ~8;          /* disable SS3 during configuration */
ADC0_EMUX_R &= ~0xF000;
ADC0_EMUX_R |= 0x5000;       /* timer trigger conversion seq 0 */
ADC0_SSMUX3_R = 0;           /* get input from channel 0 */
ADC0_SSCTL3_R |= 0x0E;       /* take chip temperature, set flag at 1st sample */
ADC0_IM_R = (1<<3);         /* mask the interrupt */
NVIC_PRI4_R |= 0;          /* set the interrupt priority */
NVIC_EN0_R |= (1<<17);
ADC0_ISC_R = (1<<3);       /* enable the interrupt */
ADC0_ACTSS_R |= 8;           /* enable ADC0 sequencer 3 */

}

void ADC0_Handler(void) {
  //ADC1_PSSI_R |= 0x8;              /* start a conversion sequence 3 */
  //while((ADC1_RIS_R & 0x8) == 0);  /* wait for conversion complete */
  //result = ADC0_SSFIFO3_R;         /* read conversion result */
  temp = (147.5 - (247.5 * ADC0_SSFIFO3_R) / 4096);
  ADC0_ISC_R = 0x8;                /* clear completion flag */
}

char readChar(void)  
{
    char c;
    while((UART1_FR_R & (1<<4)) != 0); 
    c = UART1_DR_R;                  
    return c;                    
}

void printChar(char c)  
{
    while((UART1_FR_R & (1<<5)) != 0);
    UART1_DR_R = c;           
}

void printString(char * string)
{
  while(*string)
  {
    printChar(*(string++));
  }
}

//static void displayTouchCoord() {
//  unsigned long x = Touch_ReadX();
//  unsigned long y = Touch_ReadY();
//  unsigned long z1 = Touch_ReadZ1();
//  unsigned long z2 = Touch_ReadZ2();
//  long coord = Touch_GetCoords();
//  uint16_t x1 = 320 - (coord >> 16);
//  uint16_t y1 = coord;
//
//  //LCD_DrawFilledRect(0, 210, 50, 30, 0x0000);
//  LCD_SetCursor(0, 210);
//  LCD_PrintString("x:");
//  LCD_PrintInteger((long)x1);
//  LCD_PrintString("\ny:");
//  LCD_PrintInteger((long)y1);
//}

void drawCube(pixelColor){
    // draw a cube in the center section 4
    
  //first fill the color
  LCD_DrawFilledRect(132, 92, 55, 55, pixelColor); // under face (rect)
  LCD_DrawFilledRect(109, 115, 55, 55, pixelColor); // upper face (rect)
  int temp1 = 132;
  for(int i=109; i<=164; i++){ // draw upper paranomial
    LCD_DrawLine(i, 115, temp1, 92, pixelColor); // sloped line
    temp1 = temp1 + 1;
  }
  int temp2 = 132;
  for(int j=109; j<=164; j++){ // draw lower paranomial
    LCD_DrawLine(j, 170, temp2, 147, pixelColor); // sloped line
    temp2 = temp2 + 1;
  }
    
  //then draw the shape lines
  LCD_DrawLine(132, 92, 187, 92, WHITE); // top row(rect below)
  LCD_DrawLine(132, 147, 187, 147, WHITE); // bottom row(rect below)
  LCD_DrawLine(132, 92, 132, 147, WHITE); // left column(rect below)
  LCD_DrawLine(187, 92, 187, 147, WHITE); // right column(rect below)
  
  LCD_DrawLine(109, 115, 164, 115, WHITE); // top row(rect upper) 
  LCD_DrawLine(109, 170, 164, 170, WHITE); // bottom row(rect upper)
  LCD_DrawLine(109, 115, 109, 170, WHITE); // left column(rect upper) 
  LCD_DrawLine(164, 115, 164, 170, WHITE); // right column(rect upper)
  
  LCD_DrawLine(109, 115, 132, 92, WHITE); // left up slope
  LCD_DrawLine(164, 115, 187, 92, WHITE); // right up slope
  LCD_DrawLine(109, 170, 132, 147, WHITE); // left down slope
  LCD_DrawLine(164, 170, 187, 147, WHITE); // right down slope
}



int touched = 0;

int main()
{
  //PLL_Init(4);
  LCD_Init();
  Touch_Init();
  unsigned short pixelColor = LIGHTBLUE;
  int count = 0;
  drawCube(pixelColor);
  
  while (1) {
  int typex = Touch_ReadX();
 int typey = Touch_ReadY();
  unsigned long typez1 = Touch_ReadZ1();
  if(typez1 > (unsigned long)1000){
    touched = 1;
  } else {
    touched = 0;
  }
  
  if (touched){
    long coordinateCode = Touch_GetCoords();
    uint16_t xcor = (coordinateCode & 0xFFFF0000) >> 16;
    uint16_t ycor = coordinateCode;
    LCD_SetCursor(80,80);
    LCD_SetTextColor(255,255,240);

    typex = typex/3 - 450;
    typey = typey/5 - 250;
    if(typex < 0){
      typex = 0;
    } else if(typex > 320){
      typex = 320;
    }
    
    if(typey < 0){
      typey = 0;
    } else if(typey > 240){
      typey = 240;
    }
    
    printf("%d, %d, %d, %d , %d", 320-typex, 240-typey, typez1, xcor, ycor);
  }
    //LCD_PrintInteger(xcor);
  
//  unsigned long x = Touch_ReadX();
//  unsigned long y = Touch_ReadY();
//  unsigned long z1 = Touch_ReadZ1();
//  unsigned long z2 = Touch_ReadZ2();
//  long coord = Touch_GetCoords();
//  uint16_t x1 = 320 - (coord >> 16);
//  uint16_t y1 = coord;
//
//  //LCD_DrawFilledRect(0, 210, 50, 30, 0x0000);
//  LCD_SetCursor(0, 210);
//  LCD_PrintString("x:");
//  LCD_PrintInteger((long)x1);
//  LCD_PrintString("\ny:");
//  LCD_PrintInteger((long)y1);  
    
   
    LCD_DrawFilledRect(265, 0, 55, 55, PINK); // under face (rect)
    LCD_DrawFilledRect(265, 185, 55, 55, ORANGE); // under face (rect)
    
    switch(ps) {
      case(0): // not pressed 
        if (touched) {
          ns = 1;
          pressed = 0;
        } else {
          ns = 0;
          pressed = 0;
        }
      break;
      
      case(1): // pressed but held
        if (touched) {
            ns = 1;
            pressed = 0;
        } else {
            ns = 0;
            pressed = 1; 
        }
      break;
    }
      ps = ns;
    
    if(pressed == 1){
      if(count == 0){
        pixelColor = RED;
        drawCube(pixelColor);
        count++;
      } else if (count == 1){
        pixelColor = GREEN;
        drawCube(pixelColor);
        count++;
      } else {
        pixelColor = BLUE;
        drawCube(pixelColor);
        count = 0;
      }
    }
  
  
  }
  return 0;
}  


