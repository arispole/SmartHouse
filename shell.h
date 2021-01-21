#define NUM_COMMAND_USERS 8

typedef struct {
	const char* name;
	Command command;
	const char* description;
} CommandName;

void printCommand();

Command findCommand(char *command);

int shellFunction();