#ifndef __BOARD_H__
#define __BOARD_H__


#ifdef BOARD_SPARTAN_6_MINI
#define CLOCK_FREQ 12000000

#define LED0    GPIO_0
#define LED1    GPIO_1
#define LED2    GPIO_2
#define LED3    GPIO_3
#define LED4    GPIO_4
#define LED5    GPIO_5
#define LED6    GPIO_6
#define LED7    GPIO_7

#define BTN0    GPIO_8
#define BTN1    GPIO_9
#define BTN2    GPIO_10
#define BTN3    GPIO_11

#define BTN0    GPIO_8
#define BTN1    GPIO_9
#define BTN2    GPIO_10
#define BTN3    GPIO_11

#define SW0     GPIO_12
#define SW1     GPIO_13
#define SW2     GPIO_14
#define SW3     GPIO_15


#define LEDRGB_R    PWM_0
#define LEDRGB_G    PWM_1
#define LEDRGB_B    PWM_2




#endif // BOARD_SPARTAN_6_MINI


#endif // __BOARD_H__