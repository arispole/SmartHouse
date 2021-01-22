#include <avr/io.h>
#include <stdint.h>

#include "adc.h"

void ADC_init(void) {
    
    ADMUX = (0 << REFS1) | (1 << REFS0 | (1 << ADLAR));     // AVCC with external capacitor at AREF pin
    ADCSRA = (1<<ADEN);                                     // Enables the ADC 
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);   // ADC Prescaler - 128 Division Factor    
}

uint8_t ADC_read(uint8_t pin_num) {

    ADMUX &= 0xE0;                                          //Clear bits MUX0-4
    ADMUX |= pin_num;                                       //Defines the new ADC channel to be read by setting bits MUX0-4
    ADCSRA |= (1<<ADSC);                                    //Starts a new conversion
    while(ADCSRA & (1<<ADSC));                              //Wait until the conversion is done
    return ADCH;                                            //Returns the ADC value of the chosen channel
}