#define MAX_LEN_PIN_NAME 30
#define MAX_LEN_DEV_NAME 15
#define NUM_PIN 23

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct {
    bool isConfigured;
    char deviceName[MAX_LEN_DEV_NAME];
    char pinNames[NUM_PIN][MAX_LEN_PIN_NAME];
} Configuration;

Configuration config;

void configInit();

bool isConfigured();

int setPinName(int pin, char *name);

char* getPinName(int pin);

int setDeviceName(char *name);

char* getDeviceName();

int getPinByName(char *name);

int configure();

