#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "constants.h"
#include "commands.h"
#include "shell.h"
#include "packets.h"
#include "buffer_client.h"
#include "configuration.h"



extern cbuf_handle_t tx_buf;
extern int printed,run;

CommandName commandNames []= 
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
    },
    {
        .name = "leggiconfig",
        .command = readConfig,
    },
    {
        .name = "resettaconfig",
        .command = resetConf,
    },
    {
        .name = "config",
        .command = conf,
    }
};

void printCommand() {
    printf("LISTA COMANDI: \n\n");
    int i = 0;
    for (i=0; i < NUM_COMMAND_USERS; i++){
        printf(" %s\n", commandNames[i].name);
    }
    printf("\n\n");
}

Command findCommand(char *command) {
    int i;
    for (i=0; i < NUM_COMMAND_USERS; i++){
        if (!strcmp(command, commandNames[i].name)) {
            return commandNames[i].command;
        }
    }
    return 0;
}

int shellFunction() {

    int pin, intensity;
    Command command;
    char* token;
    OperationPacket op;
    uint8_t* data = (uint8_t*) &op;
    uint8_t checksum;
    size_t size = sizeof(OperationPacket);
    size_t s;

    while (run) 
    {
        printf("Inserisci un comando: ('quit' per uscire, 'help' per lista comandi)\n");
        token = readline(NULL);
        if (strlen(token) == 0 || !memcmp(token, " ", 1))
            continue;
        token = strtok(token, " ");
        if (!strcmp(token, "quit")) {
            run = 0;
            continue;
        }
        if (!strcmp(token, "help")) {
            printCommand();
            continue;
        }
        command = findCommand(token);
        if (command == 0) {
            printf("Comando inesistente\n");
            continue;
        }
        if (command == conf) {
            configure(tx_buf);
            continue;
        }
        if (command == readConfig) {
            getOldConfig(tx_buf);
            printConfiguration();
            continue;
        }
        if (command == resetConf) {
            resetConfig(tx_buf);
            configInit();
            continue;
        }
        if (command == dimmer) {   
            token = strtok(NULL, " ");
            if (token != NULL) {
                intensity = atoi(token);
                if (intensity < 0 || intensity > 100) {
                    printf("Intensità non corretta\n");
                    continue;
                }
            }
            else {
                printf("Comando incompleto\n");
                continue;
            }
        } 
        else intensity = 0;
        token = strtok(NULL, " ");
        if (token != NULL) {
            pin = getPinByName(token);
            if (pin < 0) {
                printf("Impossibile trovare il nome del pin\n");
                continue;
            }
            op.command = command;
            op.pin_num = pin;
            op.intensity = intensity;
            circular_buf_put(tx_buf, SOH);
            data = (uint8_t*) &op;
            checksum = 0;
            s = size;
            while (s) 
            {
                circular_buf_put(tx_buf, *data);
                checksum ^= *data;
                --s;
                ++data;
            }
            circular_buf_put(tx_buf, checksum);
            circular_buf_put(tx_buf, EOT);
            if (command == input || command == status) {
                while (!printed);
                printed = 0;
            } 
            usleep(10000);
        }
            else 
            {
                printf("Comando incompleto\n");
                continue;
            }
    }
    return 0;
}