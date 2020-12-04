#include "packet_handler_client.h"
#include "serial_port.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>

int run, printed, fd;
cbuf_handle_t tx_buf;
uint8_t *tx_buffer;

int send_packet() 
{
    uint8_t* data = malloc (sizeof(uint8_t));
    while (circular_buf_empty(tx_buf));
    do {
        circular_buf_get(tx_buf, data);
        write(fd, data, 1);
    } while (*data != EOT);
    return 0;
}

int receive_packet(OperationPacket* op)
{
    uint8_t* data = malloc (sizeof(uint8_t));
    do {
        read(fd, data, 1);
    } while (*data != SOH);

    int ret = -1;
    uint8_t checksum = 0;

    read(fd, data, 1);
    checksum ^= *data;
    if (*data == ack) ret = 0;
    else if (*data == nack) ret = -1;
    else 
    {
        op->command = *data;
        ret = 1;
    }
    
    read(fd, data, 1);
    checksum ^= *data;
    if (ret == 1) op->pin_num = *data;

    read(fd, data, 1);
    checksum ^= *data;
    if (ret == 1) op->intensity = *data;

    read(fd, data, 1);
    if (checksum != *data) ret = -1;

    read(fd, data, 1);
    if (*data != EOT) ret = -1;

    return ret;
}

void* packetHandlerFunction()
{
    OperationPacket* op;
    while (run)
    {
        save_current_head_pointer(tx_buf);
        send_packet();
        int retval = receive_packet(op);
        if (retval == -1) {
            update_pointer(tx_buf);
        }
        else if (retval == 1) {
            print_packet(op);
            printed = 1;
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
        if (retval >= 0 && retval < 8) pin = retval + 2;
        else if (retval >= 8 && retval < 16) pin = retval + 38;
        else pin = retval - 16;

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

    configInit();

    run = 1;
    printed = 0;

    bool configuration = true;

    while (configuration)
    {
        printf("Scegli il tipo di configurazione del device ([usage] vecchia/nuova)\n");
        char *input = readline(NULL);
        if (!strcmp(input,"nuova"))
        {
            configure();
            configuration = false;
        }
        else if (!strcmp(input,"vecchia"))
        {
            // TO DO
            configuration = false;
        }  
        else printf("Istruzione inesistete\n");            
    }

    pthread_t packetHandler, shell;

    pthread_create (&packetHandler, NULL, packetHandlerFunction, NULL);
    pthread_create (&shell, NULL, shellFunction, NULL);

    pthread_join(packetHandler, NULL);
    pthread_join(shell, NULL);
    
    free(tx_buffer);
    circular_buf_free(tx_buf);

    s_close(fd);

    return 0;
}