#include "uart.h"
#include "pwm.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/atomic.h>
#include <stdio.h>

#define UART_BUFFER_SIZE 100

#define BAUD 19200
#define MYUBRR (F_CPU/16/BAUD-1)

typedef struct UART {

    int tx_buffer[UART_BUFFER_SIZE];
    volatile uint8_t tx_start;
    volatile uint8_t tx_end;
    volatile uint8_t tx_size;

    int rx_buffer[UART_BUFFER_SIZE];
    volatile uint8_t rx_start;
    volatile uint8_t rx_end;
    volatile uint8_t rx_size;

} UART;

static UART uart_0;

struct UART* UART_init() {
    
    UART* uart = &uart_0;

    uart->tx_start=0;
    uart->tx_end=0;
    uart->tx_size=0;
    uart->rx_start=0;
    uart->rx_end=0;
    uart->rx_size=0;


    // Set baud rate
    UBRR0H = (uint8_t)(MYUBRR>>8);
    UBRR0L = (uint8_t)MYUBRR;

    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* 8-bit data */ 
    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);   /* Enable RX and TX */  

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

void UART_putChar(struct UART* uart, uint8_t c) {
    while (uart->tx_size >= UART_BUFFER_SIZE);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        uart->tx_buffer[uart->tx_end] = c;
        ++uart->tx_end;
        if (uart->tx_end >= UART_BUFFER_SIZE) {
            uart->tx_end = 0;
        }
        ++uart->tx_size;
    }
    UCSR0B |= (1<<UDRIE0);
}

uint8_t UART_getChar(struct UART* uart) {
    while (uart->rx_size == 0);
    uint8_t c;
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

ISR(USART0_RX_vect) {
    uint8_t c = UDR0;
    if (uart_0.rx_size < UART_BUFFER_SIZE) {
        uart_0.rx_buffer[uart_0.rx_end] = c;
        ++uart_0.rx_end;
        if (uart_0.rx_end >= UART_BUFFER_SIZE) {
            uart_0.rx_end = 0;
        }
        ++uart_0.rx_size;
    }
}

ISR(USART0_UDRE_vect) {
    if (!uart_0.tx_size) {
        UCSR0B &= ~(1<<UDRIE0);
    }
    else {
        UDR0 = uart_0.tx_buffer[uart_0.tx_start];
        ++uart_0.tx_start;
        if (uart_0.tx_start >= UART_BUFFER_SIZE) {
            uart_0.tx_start = 0;
        }
        --uart_0.tx_size;
    }
} 



       
