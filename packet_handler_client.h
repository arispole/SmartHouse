#include <stdint.h>
#include "packets.h"

typedef struct CommandName
{
	const char* name;
	Command command;
} CommandName;


extern CommandName commandNames[];

Command findCommand(char *command);

void print_packet(OperationPacket* op);


