#include "digital_input.h"

const uint8_t mask46 = (1<<3);
const uint8_t mask47 = (1<<2);
const uint8_t mask48 = 1;
const uint8_t mask49 = 0;
const uint8_t mask50 = (1<<3);
const uint8_t mask51 = (1<<2);
const uint8_t mask52 = 1;
const uint8_t mask53 = 0;


void digInput_Init(void){
    DDRL &= ~ (mask46 & mask47 & mask48 & mask49);      //configure pins as inputs
    PORTL |= (mask46 | mask47 | mask48 | mask49);       //enable pull-up resistors

    DDRB &= ~ (mask50 & mask51 & mask52 & mask53);      //configure pins as inputs
    PORTB |= (mask50 | mask51 | mask52 | mask53);       //enable pull-up resistors
}

uint8_t read_digInput(uint8_t pin) {

    // check if pin is out of boundaries
    if (pin < 46 || pin > 53) {
        return -1;
    }

    uint8_t value;

    switch (pin)
    {
    case 46:
        value = (PINL&mask46) == 0;
        break;

    case 47:
        value = (PINL&mask47) == 0;
        break;
    
    case 48:
        value = (PINL&mask48) == 0;
        break;

    case 49:
        value = (PINL&mask49) == 0;
        break;

    case 50:
        value = (PINB&mask50) == 0;
        break;
    
    case 51:
        value = (PINB&mask51) == 0;
        break;

    case 52:
        value = (PINB&mask52) == 0;
        break;

    case 53:
        value = (PINB&mask53) == 0;
        break;

    }

    return value;
}


