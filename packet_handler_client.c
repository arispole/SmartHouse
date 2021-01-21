#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h> 
#include <stdbool.h>
#include <sys/socket.h>

#include "constants.h"
#include "commands.h"
#include "packets.h"
#include "configuration.h"
#include "packet_handler_client.h"

extern int fd;
extern OperationPacket op; 
extern ConfigurationPacket cp;

void print_packet()
{
     if (op.command == input)
    {
        if (op.pin_num > 45 && op.pin_num < 54) 
        {
            printf("\nLettura %s\n", getPinName(op.pin_num, 2));
            printf("Valore: %d\n\n", (int) op.intensity);
        }
        else 
        {
            printf("\nLettura %s\n", getPinName(op.pin_num, 0));
            printf("Valore: %4.2f\n\n", (float) op.intensity*5/255);
        }
    }
    else //status
    {
        printf("\nStato %s\n", getPinName(op.pin_num, 1));
        printf("Luce %s\n", (op.intensity!=0)? "accesa" : "spenta");
        printf("Intensità %d\n\n", (int) round((float) op.intensity*100/255));
    }
}

int send_packet(uint8_t* data, size_t size) 
{
    uint8_t b = 0;
    uint8_t* data1 = &b;
    uint8_t checksum = 0;

    b = SOH;
    if (send(fd, data1, 1, 0) == -1) return -1;
    while (size) {
        if (send(fd, data, 1, 0) == -1) return -1;
        checksum ^= *data;
        --size;
        ++data;
    }
    b = checksum;
    if (send(fd, data1, 1, 0) == -1) return -1;
    b = EOT;
    if (send(fd, data1, 1, 0) == -1) return -1;
    return 1;
}

int receive_packet()
{
    uint8_t i, checksum;
    int ret;
    uint8_t b = 0;
    uint8_t* data = &b;

    do {
        if (recv(fd, data, 1, 0) == -1) return -1;
    } while (*data != SOH);
    ret = -1;
    checksum = 0;
    if (recv(fd, data, 1, 0) == -1) return -1;
    checksum ^= *data;
    switch (*data) {
        case (ack):
            ret = 3;
            break;
        case (nack):
            ret = -3;
            break;
        case (stopConfig):
            ret = -2;
            break;
        case (input):
        case (status):
            op.command = *data;
            ret = 1;
            if (recv(fd, data, 1, 0) == -1) return -1;
            checksum ^= *data;
            op.pin_num = *data;
            if (recv(fd, data, 1, 0) == -1) return -1;
            checksum ^= *data;
            op.intensity = *data;
            print_packet(op);
            break;
        case (conf):
            cp.command = *data;
            ret = 2;
            if (recv(fd, data, 1, 0) == -1) return -1;
            checksum ^= *data;
            cp.pin_num = *data;
            for (i = 0; i < MAX_LEN_NAME; i++) {
            if (recv(fd, data, 1, 0) == -1) return -1;
                checksum ^= *data;
                cp.pin_name[i] = *data;
            }
            break;
    }
    if (recv(fd, data, 1, 0) == -1) return -1;
    if (checksum != *data) ret = -3;
    if (recv(fd, data, 1, 0) == -1) return -1;
    if (*data != EOT) ret = -1;
    return ret;
}