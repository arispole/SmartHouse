#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <math.h> 

#include "constants.h"
#include "commands.h"
#include "buffer_client.h"
#include "shell.h"
#include "packets.h"
#include "configuration.h"



// extern cbuf_handle_t tx_buf;
extern int printed,run;

CommandName commandNames []= 
{
    {
        .name = "accendi",
        .command = ledOn,
        .description = "[accendi] [nome pin]"
    },
    {
        .name = "spegni",
        .command = ledOff,
        .description = "[spegni] [nome pin]"
    },
    {
        .name = "intensità",
        .command = dimmer,
        .description = " [intensità] [valore: 0-100] [nome pin] (regola l'intensità su una scala da 0 a 100)"
    },
    {
        .name = "leggi",
        .command = input,
        .description = "[leggi] [nome pin]                     (legge lo stato 0/1 del pin digitale input o la tensione del pin analogico da 0 a 5.00 v)"
    },
    {
        .name = "stato",
        .command = status,
        .description = "[stato] [nome pin]                     (legge lo stato acceso/spento del pin digitale e la sua intensità)"
    },
    {
        .name = "leggiconfig",
        .command = readConfig,
        .description = "[leggiconfig]                          (stampa l'elenco dei pin con i rispettivi nomi)"
    },
    {
        .name = "resettaconfig",
        .command = resetConf,
        .description = "[resettaconfig]                        (annulla nome device e nomi pin)"
    },
    {
        .name = "config",
        .command = conf,
        .description = "[config]                               (entra in modifica configurazione permettendo il cambiamento/aggiunta dei nomi)"
    }
};

void printCommand() {
    printf("LISTA COMANDI: \n\n");
    int i = 0;
    for (i=0; i < NUM_COMMAND_USERS; i++){
        printf("  %-15s %s\n", commandNames[i].name, commandNames[i].description);
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

int shellFunction(cbuf_handle_t tx_buf) {

    numPin numpin;
    int intensity;
    Command command;
    char* token;
    OperationPacket op;
    uint8_t* data = (uint8_t*) &op;
    uint8_t checksum;
    size_t size = sizeof(OperationPacket);
    size_t s;

    while (run) 
    {
        printf("Inserisci un comando: ([quit] per uscire, [help] per lista comandi)\n");
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
                intensity = (int) round((float) atoi(token) * 255 / 100);
                if (intensity < 0 || intensity > 255) {
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
            numpin = getPinByName(token);
            if (numpin.num < 0) {
                printf("Impossibile trovare il nome del pin\n");
                continue;
            }
            else if ((numpin.ind < 8 || numpin.ind > 15) && (command == dimmer || command == status || command == ledOn || command == ledOff)) {
                printf("Il pin non è un pin digitale/switch/dimmer\n");
                continue;
            }
            else if (numpin.ind >= 8 && numpin.ind <= 15 && command == input) {
                printf("Il pin non è un pin analogico o un pin digitale input\n");
                continue;
            }
            op.command = command;
            op.pin_num = numpin.num;
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
        }
        else {
            printf("Comando incompleto\n");
            continue;
        }
    }
    return 0;
}