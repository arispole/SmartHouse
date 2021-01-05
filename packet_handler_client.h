void print_packet(OperationPacket* op);

int send_packet(uint8_t* data);

int receive_packet(uint8_t* data, OperationPacket* op, ConfigurationPacket* cp);

void* packetHandlerFunction();