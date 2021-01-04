#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "constants.h"
#include "commands.h"
#include "packets.h"
#include "buffer_client.h"
#include "configuration.h"

struct Pins {
    uint8_t Num;
    char Name[MAX_LEN_PIN_NAME];
};

typedef struct {
    bool isConfigured;
    char deviceName[MAX_LEN_DEV_NAME];
    Pins pinNames[BLOC_PIN];
} Configuration;

Configuration config_dev;

void configInit() {
    config_dev.isConfigured = false;
    strcpy(config_dev.deviceName, "");
    uint8_t i;
    for (i = 0; i < NUM_PIN; i++){
        strcpy(config_dev.pinNames[i].Name, "");
        if (i <= 7) config_dev.pinNames[i].Num = i;
        else if (i >=  8 && i <=  9) config_dev.pinNames[i].Num = i -  6;
        else if (i >= 10 && i <= 13) config_dev.pinNames[i].Num = i -  5;
        else if (i >= 14 && i <= 15) config_dev.pinNames[i].Num = i -  3;
        else if (i >= 16 && i <= 23) config_dev.pinNames[i].Num = i + 30;
    }
}

void set_isConfigured() {
    config_dev.isConfigured = true;
} 

void setPinName(uint8_t pin, char* name) {
        strcpy(config_dev.pinNames[pin].Name, name);
}

char* getPinName(uint8_t pin, uint8_t k) {
    uint8_t i=0;
    if (config_dev.isConfigured) {
        while (config_dev.pinNames[k*BLOC_PIN+i].Num != pin && i <= BLOC_PIN) ++i;
        return config_dev.pinNames[k*BLOC_PIN+i].Name;
    }
    return "";
}

void setDeviceName(char *name){
        strcpy(config_dev.deviceName, name);
}

char* getDeviceName() {
    if (config_dev.isConfigured) {
        return config_dev.deviceName;
    }
    return "";
}

int getPinByName(char *name){
    uint8_t i = 0; 
    int numPin = -1;
    while (i < NUM_PIN && strcmp(name,config_dev.pinNames[i].Name)) ++i;
    if (i < NUM_PIN) numPin = config_dev.pinNames[i].Num;
    return numPin;
}

int getOldConfig(cbuf_handle_t tx_buf) {

    //ask for old configuration (if not present, it returns a nack)

    ControlPacket rc = {
        .command = readConfig
    };

    circular_buf_put(tx_buf, SOH);
    uint8_t *data = (uint8_t*) &rc;
    uint8_t checksum = 0;
    size_t size = sizeof(ControlPacket);
    while (size) 
    {
        circular_buf_put(tx_buf, *data);
        checksum ^= *data;
        --size;
        ++data;
    }
    circular_buf_put(tx_buf, checksum);
    circular_buf_put(tx_buf, EOT);
    return 0;
}

int resetConfig(cbuf_handle_t tx_buf) {

    ControlPacket rc = {
        .command = resetConf
    };

    circular_buf_put(tx_buf, SOH);
    uint8_t *data = (uint8_t*) &rc;
    uint8_t checksum = 0;
    size_t size = sizeof(ControlPacket);
    while (size) 
    {
        circular_buf_put(tx_buf, *data);
        checksum ^= *data;
        --size;
        ++data;
    }
    circular_buf_put(tx_buf, checksum);
    circular_buf_put(tx_buf, EOT);
    return 0;
}

void configPin (uint8_t k, cbuf_handle_t tx_buf) {

    char* token;
    uint8_t pin, i, stop;
    ConfigurationPacket cp;
    uint8_t* data = (uint8_t*) &cp;
    uint8_t checksum;
    const size_t size = sizeof(ConfigurationPacket);
    size_t s;

    stop = 0;
    while (!stop) {
        printf("Inserisci il numero del pin e il nome [quit per terminare]\n");
        token = readline(NULL);
        if (strlen(token) == 0 || !memcmp(token, " ", 1))
            continue;
        if (!strcmp(token,"quit")) {
            printf("\n°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°\n\n");
            stop = 1;
        }
        else {
            token = strtok(token, " ");
            pin = atoi(token);
            i = 0;
            while (config_dev.pinNames[k*BLOC_PIN+i].Num != pin && i <= (BLOC_PIN-1)) ++i;
            if (i = BLOC_PIN) {
                printf("Pin non disponibile\n");
                continue;
            }
            else strcpy(config_dev.pinNames[k*BLOC_PIN+i].Name, ""); 
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("[numero-pin nome-pin]\n");
                continue;
            }
            if (strlen(token) > MAX_LEN_PIN_NAME) { 
                printf("nome Pin deve essere <= %d\n", MAX_LEN_PIN_NAME);
                continue;
            }
            if (getPinByName(token)!= -1) {
                printf("nome già utilizzato\n");
                continue;
            }
            else {
                setPinName(i, token);
                cp.command = conf;
                cp.pin_num = i;
                memset(cp.pin_name, 0, MAX_LEN_PIN_NAME);
                memcpy(cp.pin_name, token, sizeof(token)-1);
                circular_buf_put(tx_buf, SOH);
                data = (uint8_t*) &cp;
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
            }
        }
    }
}

int configure(cbuf_handle_t tx_buf) {
    
    char* res;
    char* name;
    uint8_t stop;
    ConfigurationPacket cp;
    uint8_t* data = (uint8_t*) &cp;
    uint8_t checksum;
    const size_t size = sizeof(ConfigurationPacket);
    size_t s;

    printf("\n\nINIZIO CONFIGURAZIONE\n\n");

    bool nomedev = true;
    if (config_dev.isConfigured) {
        printf("Cambiare nome al device? [s/n]\n");
        res = readline(NULL);
        if (!strcmp(res,"n")) nomedev = false;
    }
    stop = 0;
    while (!stop && nomedev){
        printf("Inserisci il nome del device:\n");
        name = readline(NULL);
        if (strlen(name) > MAX_LEN_DEV_NAME)
                printf("nome device deve essere <= %d\n", MAX_LEN_DEV_NAME);
        else {
            setDeviceName(name);
            cp.command = conf;
            cp.pin_num = 24;
            memset(cp.pin_name, 0, MAX_LEN_PIN_NAME);
            memcpy(cp.pin_name, name, sizeof(name)-1);
            circular_buf_put(tx_buf, SOH);
            data = (uint8_t*) &cp;
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
            stop = 1;
        }
    }
    printf("\n---------------------------------------------------------------------\n\n");
    printf("CONFIGURAZIONE PIN\n\n");
    printf(" Pin disponibili:   PIN ANALOGICI\n");
    printf("                    0 - 1 - 2 - 3 - 4 - 5 - 6 - 7\n\n");
    configPin(0, tx_buf);
    printf(" Pin disponibili:   PIN DIGITALI/SWITCH/DIMMER\n");
    printf("                    2 - 3 - 5 - 6 - 7 - 8 - 11 - 12\n\n");
    configPin(1, tx_buf);
    printf(" Pin disponibili:   INPUT\n");
    printf("                    46 - 47 - 48 - 49 - 50 - 51 - 52 - 53\n\n");
    configPin(2, tx_buf);
    config_dev.isConfigured = true;
    printf("\n\nCONFIGURAZIONE TERMINATA\n\n");
    return 0;
}

void printConfiguration() {

    uint8_t i = 0;
    
    if (!config_dev.isConfigured) return;
    printf("\nNOME DEVICE: %s\n\n", config_dev.deviceName);

    printf("  PIN ANALOGICI %19s     PIN DIGITALI/SWITCH/DIMMER %7s    INPUT\n\n"," "," ");
    for (i=0; i < BLOC_PIN; i++){
        printf("  PIN %3d --> %16s          PIN %3d --> %16s          PIN %3d --> %16s\n", config_dev.pinNames[i].Num, config_dev.pinNames[i].Name,
                                                                                           config_dev.pinNames[i+8].Num, config_dev.pinNames[i+8].Name,
                                                                                           config_dev.pinNames[i+16].Num, config_dev.pinNames[i+16].Name);
    }
    printf("\n\n");
}