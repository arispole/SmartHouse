#include <avr/interrupt.h>

#define sbi(port,bit)  __asm__ __volatile__ ( "sbi %0, %1" :: "I" (_SFR_IO_ADDR(port)),"I" (bit))
#define cbi(port,bit)  __asm__ __volatile__ ( "cbi %0, %1" :: "I" (_SFR_IO_ADDR(port)),"I" (bit))

void EEPROM_write(unsigned int uiAddress, unsigned char ucData) {
    
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));
    
    /* Set up address and Data Registers */
    EEAR = uiAddress;
    EEDR = ucData;
    
    /* Disable interrupst */
    cli();

    /* Write logical one to EEMPE */
    // EECR |= (1<<EEMPE);
    sbi(EECR,EEMPE);
    
    /* Start eeprom write by setting EEPE */
    // EECR |= (1<<EEPE);
    sbi(EECR,EEPE);

    /* Enable interrupts */
    sei();

}

unsigned char EEPROM_read(unsigned int uiAddress) {
    
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));
    
    /* Set up address register */
    EEAR = uiAddress;
    
    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);
    
    /* Return data from Data Register */
    return EEDR;
    
}