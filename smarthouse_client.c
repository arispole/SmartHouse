#include "packet_handler_client.h"
#include "serial_port.h"
#include "configuration.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>

int run, printed, fd, configuration_phase, configuration_found;
cbuf_handle_t tx_buf;
uint8_t *tx_buffer;

OperationPacket *op;

ConfigurationPacket cp_struct = {
    .command = NULL,
    .pin_num = NULL,
    .pin_name = NULL
};

ConfigurationPacket *cp = &cp_struct;

int send_packet() 
{
    uint8_t* data = malloc (sizeof(uint8_t));
    while (circular_buf_empty(tx_buf));
    do {
        circular_buf_get(tx_buf, data);
        printf("%02x  ", *data);
        write(fd, data, 1);
    } while (*data != EOT);
    return 0;
}

int receive_packet()
{
    uint8_t* data = malloc (sizeof(uint8_t));
    do {
        read(fd, data, 1);
    } while (*data != SOH);

    int ret = -1;
    uint8_t checksum = 0;
    uint8_t i;

    read(fd, data, 1);
    checksum ^= *data;
    switch (*data) {
        case (ack):
            ret = 0;
            break;
        case (nack):
            ret = -1;
            break;
        case (stopConfig):
            ret = -2;
            break;
        case (input):
        case (status):
            op->command = *data;
            ret = 1;

            read(fd, data, 1);
            checksum ^= *data;
            op->pin_num = *data;

            read(fd, data, 1);
            checksum ^= *data;
            op->intensity = *data;
            break;
        case (conf):
            printf("pacchetto\n");
            cp->command = *data;
            ret = 2;

            read(fd, data, 1);
            checksum ^= *data;
            cp->pin_num = *data;

            for (i = 0; i < MAX_LEN_PIN_NAME; i++) {
                read(fd, data, 1);
                checksum ^= *data;
                printf(":: %02x\n", *data);
                cp->pin_name[i] = *data;
            }

            printf("fine lettura pacchetto\n");
            
            break;
    }
    
    read(fd, data, 1);
    if (checksum != *data) ret = -1;

    read(fd, data, 1);
    if (*data != EOT) ret = -1;

    return ret;
}

void* packetHandlerFunction()
{

    while (run)
    {
        save_current_head_pointer(tx_buf);
        send_packet();
        int retval = receive_packet();
        if (retval == -1) {
            if (configuration_phase) {
                printf("ppp\n");
                configuration_phase = 0;
                configuration_found = 0;
            }
            else update_pointer(tx_buf);
        }
        else if (retval == 1) {
            print_packet(op);
            printed = 1;
        }
        else if (retval == 2) {
            do {
                setPinName(cp->pin_num, cp->pin_name);
                printf("settato nome\n");
                retval = receive_packet();
            } while (retval != -2);
            configuration_phase = 0;
            configuration_found = 1;
        }
        usleep(1000000);
    }

    pthread_exit(NULL);
}

void* shellFunction()
{
    int retval, pin, intensity;
    Command command;
    
    while (run)
    {
        printf("Inserisci un comando:\n");
        char *token = strtok(readline(NULL), " ");
        if (!strcmp(token, "quit")) 
        {
            run = 0;
            continue;
        }
        if (!strcmp(token, "configurazione")) {
            configure(tx_buf);
            continue;
        }
        
        command = findCommand(token);

        if (command == 0)
        {
            printf("Comando inesistente\n");
            continue;
        }
        
        if (command == dimmer) 
        {   
            token = strtok(NULL, " ");
            intensity = atoi(token);
            if (intensity < 0 || intensity > 100) 
            {
                printf("Intensità non corretta\n");
                continue;
            }
        } 
        else intensity = NULL;

        token = strtok(NULL, " ");
        retval = getPinByName(token);
        if (retval < 0)
        {
            printf("Impossibile trovare il nome del pin\n");
            continue;
        }
        if (retval >= 8 && retval < 16) {
            switch (retval) {
                case (8):
                case (9):
                    pin = retval - 6;
                    break;
                case (10):
                case (11):
                case (12):
                case (13):
                    pin = retval - 5;
                    break;
                case (14):
                case (15):
                    pin = retval - 3;
                    break; 
            }
        }
        else if (retval >= 16 && retval < 24) pin = retval + 30;

        OperationPacket op = 
        {
            .command = command,
            .pin_num = pin,
            .intensity = intensity,
        };
    
        circular_buf_put(tx_buf, SOH);
        uint8_t *data = (uint8_t*) &op;
        uint8_t checksum = 0;
        size_t size = sizeof(OperationPacket);
        while (size) 
        {
            circular_buf_put(tx_buf, *data);
            checksum ^= *data;
            --size;
            ++data;
        }
        circular_buf_put(tx_buf, checksum);
        circular_buf_put(tx_buf, EOT);

        if (command == input || command == status) {
            while (!printed);
            printed = 0;
        } 

        usleep(2000000);
    }

    pthread_exit(NULL);
}


int main()
{

    fd = s_open("/dev/ttyACM0");
    s_set_attributes(fd);

    tx_buffer  = malloc(BUFFER_MAX_SIZE * sizeof(uint8_t));
    tx_buf = circular_buf_init(tx_buffer, BUFFER_MAX_SIZE);

    pthread_t packetHandler, shell;

    run = 1;
    printed = 0;

    configInit();

    configuration_phase = 1;
    configuration_found = 0;

    pthread_create (&packetHandler, NULL, packetHandlerFunction, NULL);

    printf("INSERISCI QUALCOSA: \n");
    char *res = readline(NULL);

    getOldConfig(tx_buf);

    while (configuration_phase);

    if (!configuration_found) configure(tx_buf);

    pthread_create (&shell, NULL, shellFunction, NULL);

    pthread_join(packetHandler, NULL);
    pthread_join(shell, NULL);
    
    free(tx_buffer);
    circular_buf_free(tx_buf);

    s_close(fd);

    return 0;
}