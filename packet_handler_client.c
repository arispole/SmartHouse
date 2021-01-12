#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h> 
#include <stdbool.h>

#include "constants.h"
#include "commands.h"
#include "buffer_client.h"
#include "configuration.h"
#include "packets.h"
#include "packet_handler_client.h"

extern int run, fd, printed, configuration_phase, configuration_found;
//extern cbuf_handle_t tx_buf;

void print_packet(OperationPacket* op)
{
    uint8_t temp;
    double R, tempK1, tempk2;

     if (op->command == input)
    {
        if (op->pin_num > 45 && op->pin_num < 54) 
        {
            printf("\nLettura %s\n", getPinName(op->pin_num, 2));
            printf("Valore: %d\n\n", (int) op->intensity);
        }
        else
        {
            printf("\nLettura %s\n", getPinName(op->pin_num, 0));
            printf("Valore: %4.2f\n\n", (float) op->intensity*5/255);
        }
    }
    else //status
    {
        printf("\nStato %s\n", getPinName(op->pin_num, 1));
        printf("Luce %s\n", (op->intensity!=0)? "accesa" : "spenta");
        printf("Intensità %d\n\n", (int) round((float) op->intensity*100/255));
    }
}

int send_packet(uint8_t* data, cbuf_handle_t tx_buf) 
{
    while (circular_buf_empty(tx_buf)) {
        if (!run) return -1;
    }
    do {
        circular_buf_get(tx_buf, data);
        write(fd, data, 1);
    } while (*data != EOT);
    return 0;
}

int receive_packet(uint8_t* data, OperationPacket* op, ConfigurationPacket* cp)
{
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
            cp->command = *data;
            ret = 2;
            read(fd, data, 1);
            checksum ^= *data;
            cp->pin_num = *data;
            for (i = 0; i < MAX_LEN_NAME; i++) {
                read(fd, data, 1);
                checksum ^= *data;
                cp->pin_name[i] = *data;
            }
            break;
    }
    read(fd, data, 1);
    if (checksum != *data) ret = -1;
    read(fd, data, 1);
    if (*data != EOT) ret = -1;
    return ret;
}

void* packetHandlerFunction(void* tb) {

    OperationPacket op_struct = {
        .command = 0,
        .pin_num = 0,
        .intensity = 0
    };

    OperationPacket *op = &op_struct;

    ConfigurationPacket cp_struct = {
        .command = 0,
        .pin_num = 0,
        .pin_name = 0
    };

    ConfigurationPacket *cp = &cp_struct;

    uint8_t* data = malloc (sizeof(uint8_t));

    cbuf_handle_t tx_buf = tb;

    while (run)
    {
        int retval;
        save_current_head_pointer(tx_buf);
        retval = send_packet(data, tx_buf);
        if (retval == -1) continue;
        retval = receive_packet(data, op, cp);
        if (retval == -1) {
            if (configuration_phase) {
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
                if (cp->pin_num == 24) setDeviceName(cp->pin_name);
                else setPinName(cp->pin_num, cp->pin_name);
                retval = receive_packet(data, op, cp);
            } while (retval != -2);
            configuration_phase = 0;
            configuration_found = 1;
        }
    }
    free(data);
    pthread_exit(NULL);
}