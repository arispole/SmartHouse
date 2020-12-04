#include "configuration.h"

void configInit() {
    config.isConfigured = false;
    memset(config.deviceName, '\0', MAX_LEN_DEV_NAME);
    int i;
    for (i = 0; i < NUM_PIN; i++){
        memset(config.pinNames[i], '\0', MAX_LEN_PIN_NAME);
    }
}

bool isConfigured() {
    return config.isConfigured;
} 

int setPinName(int pin, char *name){
    if (sizeof(name) <= MAX_LEN_PIN_NAME) {
        int i;
        for (i=0; i < sizeof(name); i++) {
            config.pinNames[pin][i] = *name;
            ++name;
        } 
        return 0;       
    }
    return -1;
}

char* getPinName(int pin){
    if (config.isConfigured) {
        return config.pinNames[pin];
    }
}

int setDeviceName(char *name){
    if (sizeof(name) <= MAX_LEN_DEV_NAME) {
        int i;
        for (i=0; i < MAX_LEN_DEV_NAME; i++) {
            config.deviceName[i] = *name;
            ++name;
        }
        return 0;
    }
    return -1;
}

char* getDeviceName() {
    if (config.isConfigured) {
        return config.deviceName;
    }
}

int getPinByName(char *name){
    int i, j, numPin = -1;
    int found = 0;
    for (i = 0; i < NUM_PIN; i++) {
        for (j = 0; j < MAX_LEN_PIN_NAME; j++) {
            if ((j > sizeof(name) && config.pinNames[i][j] != '\0') || name[j] != config.pinNames[i][j]) {
                found = 0;
                break;
            }
            else if (config.pinNames[i][j] == '\0') break;
            else found = 1;
        }
        if (found == 1) {
            numPin = i;
            break;
        }
    }
    return numPin;
}

int configure() {
    while (1){
        printf("Inserisci il nome del device:\n");
        char *name = readline(NULL);
        int retval = setDeviceName(name);
        if (retval == 0) break;
        else {
            printf("Il nome del device è troppo lungo!\n");
        }
    }
    char stop = 0;
    char *line;
    while (!stop) {
        printf("Inserisci il numero del pin e il nome (quit per terminare)\n");
        line = readline(NULL);
        if (!strcmp(line,"quit")) {
            printf("Configurazione terminata\n");
            stop = 1;
        }
        else {
            char *token = strtok(line, " ");
            if (token == NULL) printf("[usage] numero-pin nome-pin\n");
            else {
                int pin = atoi(token);
                token = strtok(NULL, " ");
                setPinName(pin, token);
            }
        }
    }
    config.isConfigured = true;
    return 0;
}