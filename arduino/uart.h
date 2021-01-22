#define UART_BUFFER_SIZE 100

typedef struct UARTI UART;

UART* UART_init(void);

void UART_putChar(UART* uart, uint8_t c);

uint8_t UART_getChar(UART* uart);

int UART_rxBufferSize(UART* uart);

int UART_txBufferSize(UART* uart);

int UART_rxBufferFull(UART* uart);

int UART_rxBufferFree(UART* uart);

int UART_txBufferFull(UART* uart);