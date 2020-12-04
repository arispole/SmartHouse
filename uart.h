#include <stdint.h>

struct UART;

struct UART* UART_init();

void UART_putChar(struct UART* uart, uint8_t c);

uint8_t UART_getChar(struct UART* uart);

int UART_rxBufferSize(struct UART* uart);

int UART_txBufferSize(struct UART* uart);

int UART_rxBufferFull(struct UART* uart);

int UART_rxBufferFree(struct UART* uart);

int UART_txBufferFull(struct UART* uart);


