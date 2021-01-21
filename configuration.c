#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
// #include <readline/readline.h>
// #include <readline/history.h>

#include "constants.h"
#include "commands.h"
#include "packets.h"
#include "packet_handler_client.h"
#include "configuration.h"

extern int fd;
extern OperationPacket op; 
extern ConfigurationPacket cp;
extern ControlPacket rc;

Configuration config_dev;

void configInit() {
    config_dev.isConfigured = false;
    strcpy(config_dev.deviceName, "");
    uint8_t i;
    for (i = 0; i < NUM_PIN; i++) {
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

    uint8_t i = 0;
    
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

numPin getPinByName(char *name){
    
    numPin numpin;
    int i = 0;

    numpin.num = -1;
    numpin.ind = -1;
    while (i < NUM_PIN && strcmp(name,config_dev.pinNames[i].Name)) ++i;
    if (i < NUM_PIN) {
        numpin.ind = i;
        numpin.num = config_dev.pinNames[i].Num;
    }
    return numpin;
}

int getOldConfig() {    

    int ret;
    size_t size = sizeof(ControlPacket);
    uint8_t *data = (uint8_t*) &rc;

    rc.command = readConfig;
    do {
        if (send_packet(data, size) == -1) return -1;
        ret = receive_packet();
    } while ((ret != 3) && (ret != -1));                        //loops until it receives and ACK
    if (ret == -1) return -1;
    ret = receive_packet();
    if (ret == -1) return -1;                                      
    else if (ret == -3) return 0;                               //old configuration not present (NACK)
    while  (ret != -2) {
        if (cp.pin_num == 24) setDeviceName(cp.pin_name);
        else setPinName(cp.pin_num, cp.pin_name);
        ret = receive_packet();
        if (ret == -1) return -1;
    }
    set_isConfigured();
    return 1;
}

int resetConfig() {

    uint8_t ret;
    uint8_t *data = (uint8_t*) &rc;
    size_t size = sizeof(ControlPacket);

    rc.command = resetConf;
    do {
        if (send_packet(data, size) == -1) return -1;
        ret = receive_packet();
    } while ((ret != 3) && (ret != -1));
    if (ret == -1) return -1;
    return 0;
}

int configPin (uint8_t k) {

    char *line = NULL;
    size_t len = 0;
    char* token;
    uint8_t pin, i, stop;
    uint8_t* data = (uint8_t*) &cp;
    size_t size = sizeof(ConfigurationPacket);
    numPin numpin;
    int ret = 0;
    
    stop = 0;
    while (!stop) {
        printf("Inserisci il numero del pin e il nome, [quit] per terminare\n");
        getline(&line, &len, stdin);
        char *newline = strchr(line, '\n' ); 
        if ( newline ) *newline = 0;
        token = line;
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
            if (i == BLOC_PIN) {
                printf("Pin non disponibile\n");
                continue;
            }
            else strcpy(config_dev.pinNames[k*BLOC_PIN+i].Name, ""); 
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("[numero-pin nome-pin]\n");
                continue;
            }
            if (strlen(token) > MAX_LEN_NAME - 1) { 
                printf("nome Pin deve essere <= %d\n", MAX_LEN_NAME - 1);
                continue;
            }
            numpin = getPinByName(token);
            if (numpin.num != -1) {
                printf("nome già utilizzato\n");
                continue;
            }
            else {
                setPinName(k*BLOC_PIN+i, token);
                cp.command = conf;
                cp.pin_num = k*BLOC_PIN+i;
                memset(cp.pin_name, 0, MAX_LEN_NAME);
                memcpy(cp.pin_name, token, strlen(token));
                do {
                    if (send_packet(data, size) == -1) {ret = -1; stop = 0;}
                    else {
                        ret = receive_packet(); 
                        if (ret == -1) stop = 0;
                    }
                } while ((ret != 3) && (ret != -1));
            }
        }
    }
    free(line);
    return ret;
}

int configure() {
    
    char *line = NULL;
    size_t len = 0;
    char* name;
    uint8_t stop; 
    uint8_t* data = (uint8_t*) &cp;
    size_t size = sizeof(ConfigurationPacket);
    int ret;

    printf("\n\nINIZIO CONFIGURAZIONE\n\n");
    bool nomedev = true;
    if (config_dev.isConfigured) {
        printf("Cambiare nome al device? [s/n]\n");
        getline(&line, &len, stdin);
        char *newline = strchr(line,'\n'); 
        if ( newline ) *newline = 0;
        if (!strcmp(line,"n")) nomedev = false;
    }
    stop = 0;
    while (!stop && nomedev){
        printf("Inserisci il nome del device:\n");
        getline(&line, &len, stdin);
        char *newline = strchr(line,'\n'); 
        if (newline) *newline = 0;
        if (strlen(line) > MAX_LEN_NAME - 1)
                printf("nome device deve essere <= %d\n", MAX_LEN_NAME - 1);
        else {
            setDeviceName(line);
            cp.command = conf;
            cp.pin_num = 24;
            memset(cp.pin_name, 0, MAX_LEN_NAME);
            memcpy(cp.pin_name, line, strlen(line));
            do {
                if (send_packet(data, size) == -1) {ret = -1; stop = 0;}
                else {
                    ret = receive_packet(); 
                    if (ret == -1) stop = 0;
                }
            } while ((ret != 3) && (ret != -1));
            stop = 1;
        }
    }
    printf("\n---------------------------------------------------------------------\n\n");
    printf("CONFIGURAZIONE PIN\n\n");
    printf(" Pin disponibili:   PIN ANALOGICI\n");
    printf("                    0 - 1 - 2 - 3 - 4 - 5 - 6 - 7\n\n");
    if (configPin(0) == -1) return -1;
    printf(" Pin disponibili:   PIN DIGITALI/SWITCH/DIMMER\n");
    printf("                    2 - 3 - 5 - 6 - 7 - 8 - 11 - 12\n\n");
    if (configPin(1) == -1) return -1;
    printf(" Pin disponibili:   PIN DIGITALI INPUT\n");
    printf("                    46 - 47 - 48 - 49 - 50 - 51 - 52 - 53\n\n");
    if (configPin(2) == -1) return -1;
    set_isConfigured();
    printf("\n\nCONFIGURAZIONE TERMINATA\n\n");
    data = (uint8_t*) &rc;
    size = sizeof(ControlPacket);
    rc.command = stopConfig;
    do {
        if (send_packet(data, size) == -1) {ret = -1; stop = 0;}
        else {
            ret = receive_packet(); 
            if (ret == -1) stop = 0;
        }
    } while ((ret != 3) && (ret != -1));

    free(line);

    return ret;
}

void printConfiguration() {

    uint8_t i = 0;
    
    if (!config_dev.isConfigured) return;
    printf("\nNOME DEVICE: %s\n\n", config_dev.deviceName);

    printf("  PIN ANALOGICI %19s     PIN DIGITALI/SWITCH/DIMMER %7s    INPUT\n\n"," "," ");
    for (i=0; i < BLOC_PIN; i++) {
        printf("  PIN %3d --> %16s          PIN %3d --> %16s          PIN %3d --> %16s\n", config_dev.pinNames[i].Num, config_dev.pinNames[i].Name,
                                                                                           config_dev.pinNames[i+BLOC_PIN].Num, config_dev.pinNames[i+BLOC_PIN].Name,
                                                                                           config_dev.pinNames[i+2*BLOC_PIN].Num, config_dev.pinNames[i+2*BLOC_PIN].Name);
    }
    printf("\n\n");
}