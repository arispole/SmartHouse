#include "pwm.h"

#include <avr/interrupt.h>

// pwm enabled array


// pin masks
const uint8_t mask2 = (1 << 4);
const uint8_t mask3 = (1 << 5);
const uint8_t mask5 = (1 << 3);
const uint8_t mask6 = (1 << 3);
const uint8_t mask7 = (1 << 4);
const uint8_t mask8 = (1 << 5);
const uint8_t mask11 = (1 << 5);
const uint8_t mask12 = (1 << 6);

// PWM timers
// WGMn0 - WGMn2 -> Fast PWM, 8 bit
// COMnA1 - COMnA0 -> inverted mode
// COMnB1 - COMnB0 -> inverted mode
// COMnC1 - COMnC0 -> inverted mode
// CSn0 -> no prescaling

// Timer 1 Mask (pin 11 - 12)
#define TCCR1B_MASK ((1 << WGM12) | (1 << CS10))
#define TCCR1A_MASK (1 << WGM10)
#define TCCR1A_COM1A_ENABLE_MASK ((1 << COM1A1) | (1 << COM1A0))
#define TCCR1A_COM1B_ENABLE_MASK ((1 << COM1B1) | (1 << COM1B0))

// Timer 3 Mask (pin 2 - 3 - 5)
#define TCCR3B_MASK ((1 << WGM32) | (1 << CS30))
#define TCCR3A_MASK (1 << WGM30) 
#define TCCR3A_COM3A_ENABLE_MASK ((1 << COM3A1) | (1 << COM3A0))
#define TCCR3A_COM3B_ENABLE_MASK ((1 << COM3B1) | (1 << COM3B0))
#define TCCR3A_COM3C_ENABLE_MASK ((1 << COM3C1) | (1 << COM3C0))

// Timer 4 Mask (pin 6 - 7 - 8)
#define TCCR4B_MASK ((1 << WGM42) | (1 << CS40))
#define TCCR4A_MASK (1 << WGM40) 
#define TCCR4A_COM4A_ENABLE_MASK ((1 << COM4A1) | (1 << COM4A0))
#define TCCR4A_COM4B_ENABLE_MASK ((1 << COM4B1) | (1 << COM4B0))
#define TCCR4A_COM4C_ENABLE_MASK ((1 << COM4C1) | (1 << COM4C0))

PWM_Enabled pwm_enabled_array[] = 
{   
    // pin 2
    {
        .pin_num = 2,
        .is_enabled = false,
    },
    // pin 3
    {
        .pin_num = 3,
        .is_enabled = false,
    },
    // pin 5
    {
        .pin_num = 5,
        .is_enabled = false,
    },
    // pin 6
    {
        .pin_num = 6,
        .is_enabled = false,
    },
    // pin 7
    {
        .pin_num = 7,
        .is_enabled = false,
    },
    // pin 8
    {
        .pin_num = 8,
        .is_enabled = false,
    },
    // pin 11
    {
        .pin_num = 11,
        .is_enabled = false,
    },
    // pin 12
    {
        .pin_num = 12,
        .is_enabled = false,
    }
};

void PWM_init(void) {

  // disable interrupts
  cli();

  // configure timer 1 - 3 -4
  TCCR1A = TCCR1A_MASK;
  TCCR1B = TCCR1B_MASK;
  TCCR3A = TCCR3A_MASK;
  TCCR3B = TCCR3B_MASK;
  TCCR4A = TCCR4A_MASK;
  TCCR4B = TCCR4B_MASK;

  // clear all higher bits of output compare for timer 1
  OCR1AH = 0;
  OCR1BH = 0;

  // clear all higher bits of output compare for timer 3
  OCR3AH = 0;
  OCR3BH = 0;
  OCR3CH = 0;

  // clear all higher bits of output compare for timer 4
  OCR4AH = 0;
  OCR4BH = 0;
  OCR4CH = 0;

  // configure pins as output
  DDRE |= (mask2 | mask3| mask5);
  DDRH |= (mask6 | mask7 | mask8);
  DDRB |= (mask11 | mask12);

  // enable interrupts
  sei();

}

int led_ON(uint8_t pin_num) {

    pwm_enabled_array[pin_num].is_enabled = false;

    // TO DO -> check pin boundaries 

    switch (pin_num) {
      case 2:
        TCCR3A = TCCR3A_MASK;           //disable OCRn
        PORTE |= mask2;
        break;
      case 3:
        TCCR3A = TCCR3A_MASK;           
        PORTE |= mask3;
        break;
      case 5:
        TCCR3A = TCCR3A_MASK;
        PORTE |= mask5;
        break;
      case 6:
        TCCR4A = TCCR4A_MASK;
        PORTH |= mask6;
        break;
      case 7:
        TCCR4A = TCCR4A_MASK;
        PORTH |= mask7;
        break;
      case 8:
        TCCR4A = TCCR4A_MASK;
        PORTH |= mask8;
        break;
      case 11:
        TCCR1A = TCCR1A_MASK;
        PORTB |= mask11;
        break;
      case 12:
        TCCR1A = TCCR1A_MASK;
        PORTB |= mask12;
        break;
    }

    return 0;
}

int led_OFF(uint8_t pin_num) {

    pwm_enabled_array[pin_num].is_enabled = false;

    // TO DO -> check pin boundaries 

    switch (pin_num) {
      case 2:
        TCCR3A = TCCR3A_MASK;             //disable OCRn
        PORTE &= ~ mask2;
        break;
      case 3:
        TCCR3A = TCCR3A_MASK;
        PORTE &= ~ mask3;
        break;
      case 5:
        TCCR3A = TCCR3A_MASK;
        PORTE &= ~ mask5;
        break;
      case 6:
        TCCR3A = TCCR3A_MASK;
        PORTH &= ~ mask6;
        break;
      case 7:
        TCCR3A = TCCR3A_MASK;
        PORTH &= ~ mask7;
        break;
      case 8:
        TCCR3A = TCCR3A_MASK;
        PORTH &= ~ mask8;
        break;
      case 11:
        TCCR3A = TCCR3A_MASK;
        PORTB &= ~ mask11;
        break;
      case 12:
        TCCR3A = TCCR3A_MASK;
        PORTB &= ~ mask12;
        break; 
    }

    return 0;
}

int led_DIMMER(uint8_t pin_num, uint8_t intensity) {

    pwm_enabled_array[pin_num].is_enabled = true;

    // TO DO -> check pin and intensity boundaries 

    switch (pin_num) {
      case 2:
        TCCR3A |= TCCR3A_COM3B_ENABLE_MASK;         //enable OCRn
        OCR3BL = 255 - ((int) intensity * 255)/100;
        break;
      case 3:
        TCCR3A |= TCCR3A_COM3C_ENABLE_MASK;
        OCR3CL = 255 - (intensity * 255)/100;
        break;
      case 5:
        TCCR3A |= TCCR3A_COM3A_ENABLE_MASK;
        OCR3AL = 255 - (intensity * 255)/100;
        break;
      case 6:
        TCCR4A |= TCCR4A_COM4A_ENABLE_MASK;
        OCR4AL = 255 - (intensity * 255)/100;
        break;
      case 7:
        TCCR4A |= TCCR4A_COM4B_ENABLE_MASK;
        OCR4BL = 255 - (intensity * 255)/100;
        break;
      case 8:
        TCCR4A |= TCCR4A_COM4C_ENABLE_MASK;
        OCR4CL = 255 - (intensity * 255)/100;
        break;
      case 11:
        TCCR1A |= TCCR1A_COM1A_ENABLE_MASK;
        OCR1AL = 255 - (intensity * 255)/100;
        break;
      case 12:
        TCCR1A |= TCCR1A_COM1B_ENABLE_MASK;
        OCR1BL = 255 - (intensity * 255)/100;
        break;
    }

    return 0;
}

uint8_t get_intensity(uint8_t pin_num) {

  uint8_t intensity;

  bool isEnabled;
  int i;
  for (i = 0; i < sizeof(pwm_enabled_array); i++) {
    if (pwm_enabled_array[i].pin_num == pin_num) isEnabled = pwm_enabled_array[i].is_enabled;
  }
  
  switch (pin_num) {
      case 2:
        if (isEnabled) intensity = 20; // intensity = (255 - OCR3BL) * 100/255;     
        else intensity = (PORTE&mask2) != 0 ? 100 : 0;
        break;
      case 3:
        if (isEnabled) intensity = (255 - OCR3CL) * 100/255;     
        else intensity = ((PORTE&mask3) == 0) * 100;
        break;
      case 5:
        if (isEnabled) intensity = (255 - OCR3AL) * 100/255;     
        else intensity = ((PORTE&mask5) == 0) * 100;
        break;
      case 6:
        if (isEnabled) intensity = (255 - OCR4AL) * 100/255;     
        else intensity = ((PORTH&mask6) == 0) * 100;
        break;
      case 7:
        if (isEnabled) intensity = (255 - OCR4BL) * 100/255;     
        else intensity = ((PORTH&mask7) == 0) * 100;
        break;
      case 8:
        if (isEnabled) intensity = (255 - OCR4CL) * 100/255;     
        else intensity = ((PORTH&mask8) == 0) * 100;
        break;
      case 11:
        if (isEnabled) intensity = (255 - OCR1AL) * 100/255;     
        else intensity = ((PORTB&mask11) == 0) * 100;
        break;
      case 12:
        if (isEnabled) intensity = (255 - OCR1BL) * 100/255;     
        else intensity = ((PORTB&mask12) == 0) * 100;
        break;
    }

  return intensity;

}


