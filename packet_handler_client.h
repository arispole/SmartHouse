void print_packet(OperationPacket* op);

int send_packet(uint8_t* data, cbuf_handle_t tx_buf);

int receive_packet(uint8_t* data, OperationPacket* op, ConfigurationPacket* cp);

void* packetHandlerFunction(void* tx_buf);