typedef struct {
    int num;
    int ind;
} numPin;

typedef struct {
    uint8_t Num;
    char Name[MAX_LEN_NAME];
} Pins;

typedef struct {
    bool isConfigured;
    char deviceName[MAX_LEN_NAME];
    Pins pinNames[NUM_PIN];
} Configuration;

void configInit();

void set_isConfigured();

void setPinName(uint8_t pin, char* name);

char* getPinName(uint8_t pin, uint8_t k);

void setDeviceName(char* name);

char* getDeviceName();

numPin getPinByName(char* name);

int getOldConfig();

int resetConfig();

int configure();

void printConfiguration();
