#include "uart.h"
#include "packets.h"
#include "pwm.h"
#include "digital_input.h"
#include "adc.h"

#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SOH 0xAA
#define EOT 0xBB

static struct UART* uart;
OperationPacket op;

OperationPacket ACK = {
  .command = ack,
  .pin_num = NULL,
  .intensity = NULL,
};

OperationPacket NACK = {
  .command = nack,
  .pin_num = NULL,
  .intensity = NULL,
};

OperationPacket send;

int execute() {

  int ret;

  if (op.command == ledOn) ret = led_ON(op.pin_num);

  else if (op.command == ledOff) ret = led_OFF(op.pin_num);

  else if (op.command == dimmer) ret = led_DIMMER(op.pin_num, op.intensity);

  else if (op.command == input) {
    send.command = input;
    send.pin_num = op.pin_num;
    if ((op.pin_num >= 0) && (op.pin_num < 8)) send.intensity = ADC_read(op.pin_num);
    else send.intensity = read_digInput(op.pin_num);
    ret = 1; 
  }

  else if (op.command == status) {
    send.command = status;
    send.pin_num = op.pin_num;
    if (send.pin_num > 1 || send.pin_num < 13) send.intensity = get_intensity(op.pin_num);
    else send.intensity = 200;
    ret = 1;
  }

  else ret = -1;

  return ret;

}

int readPacket() {

  uint8_t c;

  while (!UART_rxBufferFull(uart));

  while (UART_rxBufferFull(uart)) {
      c = UART_getChar(uart);
      if (c == SOH) {
        break;
      }
  }
  
  int ret = 0;
  uint8_t checksum = 0;

  c = UART_getChar(uart);
  checksum ^= c;
  op.command = c;
  
  c = UART_getChar(uart);
  checksum ^= c;
  op.pin_num = c;

  c = UART_getChar(uart);
  checksum ^= c;
  op.intensity = c;
  
  c = UART_getChar(uart);
  if (checksum != c) ret = -1;

  c = UART_getChar(uart);
  if (c == EOT && ret != -1) ret = execute();

  
  return ret;
}

void sendPacket(int status) {
  uint8_t *data;
  if (status == 0) data = (uint8_t*) &ACK;
  else if (status == -1) data = (uint8_t*) &NACK;
  else data = (uint8_t*) &send;
  UART_putChar(uart, SOH);
  uint8_t checksum = 0;
  size_t size = sizeof(OperationPacket);
  while (size) 
  {
    UART_putChar(uart, *data);
    checksum ^= *data;
    --size;
    ++data;
  }
  UART_putChar(uart, checksum);
  UART_putChar(uart, EOT);
}

int main(void){

  uart = UART_init();

  PWM_init();

  digInput_Init();

  ADC_init();

  while(1) {
    int ret = readPacket();
    sendPacket(ret);
  }

}
