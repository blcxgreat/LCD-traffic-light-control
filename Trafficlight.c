#include <tm4c123gh6pm.h>
#include <stdint.h>
#include "SSD2119.h"

void LED_Init(void) { volatile unsigned long delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;       // Activate clock for port A
  delay = SYSCTL_RCGC2_R;             // Allow time for clock to initialize
  GPIO_PORTE_PCTL_R &= ~0x0000FFF0;   // Regular GPIO
  GPIO_PORTE_AMSEL_R &= ~0x0E;        // Disable analog on PA2
  GPIO_PORTE_DIR_R |= 0x0E;          // Direction PA2 output
  GPIO_PORTE_AFSEL_R &= ~0x0E;        // regular port function
  GPIO_PORTE_DEN_R |= 0x0E;           // Enable digital port
}

int ps = 3; // 1:RED LED 2:GREEN LED 3:YELLOW LED 4:OFF STATE
int ns = 3;

void timer1A_delay(void)
{
  TIMER1_CTL_R |= 0x01;        // enable Timer A after initialization 
  
    int i;
    for(i = 0; i < 2; i++) {
      while ((TIMER1_RIS_R & 0x01) != 0x01){      // wait for TimerA timeout flag
      }
        TIMER1_ICR_R = 0x1;     // clear the TimerA timeout flag
    
    }
    TIMER1_CTL_R |= 0x00;
}

//// Switch check functions
//unsigned long Switch1(void) {
//
//  return (GPIO_PORTE_DATA_R&0x04);
//}
//unsigned long Switch2(void) {
//
//  return (GPIO_PORTE_DATA_R&0x08);
//}

// lcd check function
unsigned long Switch1(void) {
  int typex = Touch_ReadX();
 int typey = Touch_ReadY();
  unsigned long typez1 = Touch_ReadZ1();
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
  
  if(typez1 > (unsigned long)1000 && (typex <= 25 && typey >= 200)){
    return 1;
  } else {
    return 0;
  }
}

unsigned long Switch2(void) {
  int typex = Touch_ReadX();
  int typey = Touch_ReadY();
  unsigned long typez1 = Touch_ReadZ1();
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
  
  if(typez1 > (unsigned long)1000 && (typex <= 90 && typey <= 60)){
    return 1;
  } else {
    return 0;
  }
}
  



void timer0A_delay(void)
{
  TIMER0_CTL_R |= 0x01;        // enable Timer A after initialization 
  
    int i;
    for(i = 0; i < 5; i++) {
      while ((TIMER0_RIS_R & 0x01) != 0x01){      // wait for TimerA timeout flag
      }
     if((Switch2()) && (ps == 1)){
        timer1A_delay();
        if ((Switch2()) && (ps == 1)){
          ns = 2;
          break;
        }  
     }
      if((Switch1()) && (ps != 3)){
        timer1A_delay();
        if ((Switch1()) && (ps != 3)){
           ns = 3;
           break;
        }

      }
      TIMER0_ICR_R = 0x1;      // clear the TimerA timeout flag
    }
    TIMER0_CTL_R |= 0x00;
}

//void Switch_Init(void) { volatile unsigned long delay;
//  SYSCTL_RCGC2_R |= 0x00000001;       // Activate clock for port A
//  delay = SYSCTL_RCGC2_R;             // Allow time for clock to initialize
//  GPIO_PORTA_AMSEL_R &= ~0x0C;        // Disable analog on PA5
//  GPIO_PORTA_PCTL_R &= ~0x00F00000;   // PCTL GPIO on PA5
//  GPIO_PORTA_DIR_R &= ~0x0C;          // Direction PA5 input
//  GPIO_PORTA_AFSEL_R &= ~0x0C;        // PA5 regular port function
//  GPIO_PORTA_DEN_R |= 0x0C;           // Enable PA5 digital port
//
//}


// LED On/Off functions
void LEDG_On(void) {
  GPIO_PORTE_DATA_R |= 0x08;
}
void LEDG_Off(void) {
  GPIO_PORTE_DATA_R &= ~0x08;
}
void LEDY_On(void) {
  GPIO_PORTE_DATA_R |= 0x04;
}
void LEDY_Off(void) {
  GPIO_PORTE_DATA_R &= ~0x04;
}
void LEDR_On(void) {
  GPIO_PORTE_DATA_R |= 0x02;
}
void LEDR_Off(void) {
  GPIO_PORTE_DATA_R &= ~0x02;
}

void delay (void){
  for (int i = 0; i<30000000; i++){
  }
}
// Set state functions
void All_Off(void) {
  LEDG_Off();
  LEDY_Off();
  LEDR_Off();
}
void REDled (void) {
  LEDG_Off();
  LEDY_Off();
  LEDR_On();
}
void GREENled (void) {
  LEDG_On();
  LEDY_Off();
  LEDR_Off();
}
void YELLOWled (void) {
  LEDG_Off();
  LEDY_On();
  LEDR_Off();
}

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

int main() {
  //Switch_Init();
  LED_Init();
  All_Off();
  LCD_Init();
  Touch_Init();
  SYSCTL_RCGCTIMER_R |= (1<<1);     // enable clock to Timer Block 1
  TIMER1_CTL_R = 0;            //disable Timer before initialization
  TIMER1_CFG_R = 0x00;         //16-bit option 
  TIMER1_TAMR_R = 0x02;        // periodic mode and down-counter
  TIMER1_TAILR_R = 16000000;  // Timer A interval load value register
  TIMER1_ICR_R = 0x1;          // clear the TimerA timeout flag
  //TIMER1_CTL_R |= 0x01;        // enable Timer A after initialization 
  
  SYSCTL_RCGCTIMER_R |= (1<<0);     // enable clock to Timer Block 0
  TIMER0_CTL_R = 0;            //disable Timer before initialization
  TIMER0_CFG_R = 0x00;         //16-bit option 
  TIMER0_TAMR_R = 0x02;        // periodic mode and down-counter
  TIMER0_TAILR_R = 16000000;  // Timer A interval load value register
  TIMER0_ICR_R = 0x1;          // clear the TimerA timeout flag
  //TIMER0_CTL_R |= 0x01;        // enable Timer A after initialization 
  
    LCD_DrawFilledRect(265, 0, 55, 55, PINK); // under face (rect)
    LCD_DrawFilledRect(265, 185, 55, 55, ORANGE); // under face (rect)
  
  drawCube(BLUE);
  while(1) {
    
    switch(ps) {
      case(0): 
        REDled();
        if (Switch1()) {
          timer1A_delay();
          if (Switch1()){
             ns = 3;
             break;
          } else {
             ns = 1;
             timer0A_delay();
             break;
          }

        } else { // Go Green
          ns = 1;
          timer0A_delay();
          break;
        }
      break;
      
      case(1):
        GREENled();
        if (Switch1()) { // Stop button
          timer1A_delay();
          if (Switch1()){
            ns = 3;
            break;
          } else {
            ns = 0;
            timer0A_delay();
            break; 
          }
          
        } else {
            if (Switch2()) { // Passenger, go yellow
              timer1A_delay();
              if (Switch2()){
                ns = 2;
                break;
              } else {
                ns = 0;
                timer0A_delay();
                break;
              }
              
              
          } else {
            ns = 0;
            timer0A_delay();
            break; 
          }
        }
      break;
      
      case(2):  
        YELLOWled();
        if (Switch1()) {
          timer1A_delay();
          if(Switch1()){
            ns = 3;
            break;
          } else {
            ns = 0;   // Always go red
            timer0A_delay();
          }
          
        } else {
          ns = 0;   // Always go red
          timer0A_delay();
        }
      break;
      
      case(3):  
        All_Off();
        if (Switch1()) {
          timer1A_delay();
          if(Switch1()){
            ns = 0;
          } else {
            ns = 3;
          }
          
        } else {
          ns = 3;
        }
      break;
    }
      ps = ns;
  }
}     