#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/atomic.h>
#include <stdio.h>
#include <stdint.h>

#include "uart.h"

#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)

struct UARTI {

    int tx_buffer[UART_BUFFER_SIZE];
    volatile uint8_t tx_start;
    volatile uint8_t tx_end;
    volatile uint8_t tx_size;

    int rx_buffer[UART_BUFFER_SIZE];
    volatile uint8_t rx_start;
    volatile uint8_t rx_end;
    volatile uint8_t rx_size;

};

static UART uart_0;

UART* UART_init() {
    
    UART* uart = &uart_0;

    uart->tx_start=0;
    uart->tx_end=0;
    uart->tx_size=0;
    uart->rx_start=0;
    uart->rx_end=0;
    uart->rx_size=0;

    // Set baud rate

    UBRR1H = (uint8_t)(MYUBRR>>8);
    UBRR1L = (uint8_t)MYUBRR;
    UCSR1C = (1<<UCSZ11) | (1<<UCSZ10); /* 8-bit data */ 
    UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1);   /* Enable RX and TX */  
    sei();
    return &uart_0;
}

int UART_rxBufferSize(UART* uart) {
    return UART_BUFFER_SIZE;
}

int UART_txBufferSize(UART* uart) {
    return UART_BUFFER_SIZE;
}

int UART_rxBufferFull(UART* uart) {
    return uart->rx_size;
}

int UART_txBufferFull(UART* uart) {
    return uart->tx_size;
}

int UART_txBufferFree(UART* uart) {
    return UART_BUFFER_SIZE - uart->tx_size;
}

void UART_putChar(UART* uart, uint8_t c) {
    while (uart->tx_size >= UART_BUFFER_SIZE);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        uart->tx_buffer[uart->tx_end] = c;
        ++uart->tx_end;
        if (uart->tx_end >= UART_BUFFER_SIZE) {
            uart->tx_end = 0;
        }
        ++uart->tx_size;
    }
    UCSR1B |= (1<<UDRIE1);
}

uint8_t UART_getChar(UART* uart) {

    uint8_t c;

    while (uart->rx_size == 0);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        c = uart->rx_buffer[uart->rx_start];
        ++uart->rx_start;
        if (uart->rx_start >= UART_BUFFER_SIZE) {
            uart->rx_start = 0;
        }
        --uart->rx_size;
    }
    return c;
}

ISR(USART1_RX_vect) {

    uint8_t c = UDR1;

    if (uart_0.rx_size < UART_BUFFER_SIZE) {
        uart_0.rx_buffer[uart_0.rx_end] = c;
        ++uart_0.rx_end;
        if (uart_0.rx_end >= UART_BUFFER_SIZE) {
            uart_0.rx_end = 0;
        }
        ++uart_0.rx_size;
    }
}

ISR(USART1_UDRE_vect) {

    if (!uart_0.tx_size) {
        UCSR1B &= ~(1<<UDRIE1);
    }
    else {
        UDR1 = uart_0.tx_buffer[uart_0.tx_start];
        ++uart_0.tx_start;
        if (uart_0.tx_start >= UART_BUFFER_SIZE) {
            uart_0.tx_start = 0;
        }
        --uart_0.tx_size;
    }
}