#include "packets.h"
#include "buffer_client.h"

#include <stdint.h>

typedef struct 
{
	const char* name;
	Command command;
} CommandName;

extern CommandName commandNames[];

Command findCommand(char *command);

void print_packet(OperationPacket* op);


