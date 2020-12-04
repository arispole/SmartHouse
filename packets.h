#include <stdint.h>

#define MAX_LEN_PIN_NAME 30

#define SOH 0xAA
#define EOT 0xBB

typedef enum {
    ledOn = 1,
    ledOff = 2,
    dimmer = 3,
    input = 4,
    status = 5,
    conf = 6,
    readConfig = 7,
    stopConfig = 8,
    ack = 9,
    nack = 10
} Command;

typedef struct {
    uint8_t command;
    uint8_t pin_num;
    uint8_t intensity;
} OperationPacket;

typedef struct {
    uint8_t command;
    uint8_t pin_num;
    uint8_t pin_name[MAX_LEN_PIN_NAME];
} ConfigurationPacket;

typedef struct {
    uint8_t command;
} ControlPacket;


