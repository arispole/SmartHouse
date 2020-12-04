#include "packet_handler_client.h"
#include "configuration.h"

#include <stdint.h>
#include <stddef.h>
#include <math.h>  


CommandName commandNames[] = 
{
    {
        .name = "accendi",
        .command = ledOn,
    },
    {
        .name = "spegni",
        .command = ledOff,
    },
    {
        .name = "intensità",
        .command = dimmer,
    },
    {
        .name = "leggi",
        .command = input,
    },
    {
        .name = "stato",
        .command = status,
    }
};

int numCommands = sizeof(commandNames)/sizeof(CommandName);

Command findCommand(char *command)
{
    int i;
    for (i=0; i < numCommands; i++){
        if (!strcmp(command, commandNames[i].name)) {
            return commandNames[i].command;
        }
    }
    return 0;
}

void print_packet(OperationPacket* op) 
{
    if (op->command == input)
    {
        if (op->pin_num > 45 && op->pin_num < 54) 
        {
            printf("\nLettura %s\n", getPinName(op->pin_num - 38));
        }
        else 
        {
            printf("\nLettura %s\n", getPinName(op->pin_num + 16));
            uint8_t temp = op->intensity;
            double tempK = log(10000.0 * ((1024.0 / temp -1)));
            tempK= 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );
            float tempC = tempK - 273.15;
            printf("Temperatura: %2.2f\n\n", tempC);
        }
        printf("Valore: %d\n\n", (int) op->intensity);
    }

    if (op->command == status)
    {
        printf("\nStato %s\n", getPinName(op->pin_num));
        printf("Luce %s\n", (op->intensity!=0)? "accesa" : "spenta");
        printf("Intensità %d%%\n\n", (int) op->intensity);
    }

}





