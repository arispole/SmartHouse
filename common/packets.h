typedef struct {
    uint8_t command;
    uint8_t pin_num;
    uint8_t intensity;
} OperationPacket;

typedef struct {
    uint8_t command;
    uint8_t pin_num;
    uint8_t pin_name[MAX_LEN_NAME];
} ConfigurationPacket;

typedef struct {
    uint8_t command;
} ControlPacket;