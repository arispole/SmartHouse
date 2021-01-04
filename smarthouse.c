#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "constants.h"
#include "commands.h"
#include "packets.h"
#include "uart.h"
#include "pwm.h"
#include "digital_input.h"
#include "adc.h"
#include "eeprom.h"

/*
#define SOH 0xAA
#define EOT 0xBB
*/

static struct UART* uart;

OperationPacket op;
OperationPacket send;

ConfigurationPacket cp;
ConfigurationPacket sendC;

ControlPacket ACK = {
  .command = ack,
};

ControlPacket NACK = {
  .command = nack,
};

ControlPacket STOPCONFIG = {
  .command = stopConfig,
};

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

int readOP(uint8_t checksum) {

  uint8_t c;
  int ret = 0;

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

int saveConfig() {

  unsigned int address = cp.pin_num * MAX_LEN_PIN_NAME + 1;

  uint8_t i;
  for (i = 0; i < MAX_LEN_PIN_NAME; i++) {
    EEPROM_write(address, cp.pin_name[i]);
    address++;
  }

  return 0;
}

void readConfiguration() {
  
  unsigned int address = 1, j = 0;
  uint8_t c;
  sendC.command = 6;

  for (address = 1; address < (25*MAX_LEN_PIN_NAME); address += MAX_LEN_PIN_NAME) {
    sendC.pin_num = j++;
    for (uint8_t i = 0; i < MAX_LEN_PIN_NAME; i++){
      sendC.pin_name[i] = EEPROM_read(address+i);
    }
    sendPacket(2);
  }

}

void resetConfig() {

  unsigned int address;

  for (address = 0; address < (25*MAX_LEN_PIN_NAME); address++) {
    EEPROM_write(address, 0);
  }

}

int readCP(uint8_t checksum) {

  uint8_t c;
  int ret = 0;

  c = UART_getChar(uart);
  checksum ^= c;
  cp.pin_num = c;

  uint8_t i;
  for (i = 0; i < MAX_LEN_PIN_NAME; i++) {
    c = UART_getChar(uart);
    checksum ^= c;
    cp.pin_name[i] = c;
  }
  
  c = UART_getChar(uart);
  if (c != checksum) ret = -1;
  
  c = UART_getChar(uart);
  if (c == EOT && ret != -1) {
    ret = saveConfig();
  }

  return ret;
}

int readPacket() {

  uint8_t c;
  int found = 0;

  while(!found) {
    while (!UART_rxBufferFull(uart));
    do {
      c = UART_getChar(uart);
      if (c == SOH) {
        found = 1;
        break;
      }
    } while (UART_rxBufferFull(uart));
  }
  
  int ret = 0;
  uint8_t checksum = 0;

  c = UART_getChar(uart);
  checksum ^= c;
  if (c >= ledOn && c <= status) {
    op.command = c;
    ret = readOP(checksum);
  }

  else if (c == conf) {
    cp.command = c;
    ret = readCP(checksum);
  }

  else if (c == resetConf) {
    c = UART_getChar(uart);
    if (c != checksum) ret = -1;
    if (UART_getChar(uart) == EOT && ret != -1) {
          resetConfig();
          ret = 0;
    }
  }

  else if (c == readConfig) {
    c = UART_getChar(uart);
    if (c != checksum) ret = -1;

    if (UART_getChar(uart) == EOT && ret != -1) {

      if (EEPROM_read(0) != 3) {
        resetConfig();
        EEPROM_write(0,3);
        ret = -1;
      }
      else {
        readConfiguration();
        ret = 3;
      } 
    }
  }
  else ret = -1;
  
  return ret;
}

void sendPacket(int status) {
  uint8_t *data;
  size_t size;
  switch (status) {
    case 0:
      data = (uint8_t*) &ACK;
      size = sizeof(ControlPacket);
      break;
    case -1:
      data = (uint8_t*) &NACK;
      size = sizeof(ControlPacket);
      break;
    case 1:
      data = (uint8_t*) &send;
      size = sizeof(OperationPacket);
      break;
    case 2:
      data = (uint8_t*) &sendC;
      size = sizeof(ConfigurationPacket);
      break;
    case 3:
      data = (uint8_t*) &STOPCONFIG;
      size = sizeof(ControlPacket);
      break;
    }

  UART_putChar(uart, SOH);
  uint8_t checksum = 0;
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
