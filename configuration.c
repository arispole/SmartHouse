#include "packets.h"
#include "configuration.h"

Configuration config_dev;

void configInit() {
    config_dev.isConfigured = false;
    memset(config_dev.deviceName, '\0', MAX_LEN_DEV_NAME);
    int i;
    for (i = 0; i < NUM_PIN; i++){
        memset(config_dev.pinNames[i], '\0', MAX_LEN_PIN_NAME);
    }
}

bool isConfigured() {
    return config_dev.isConfigured;
} 

int setPinName(int pin, char *name){
    if (sizeof(name) <= MAX_LEN_PIN_NAME) {
        int i;
        for (i=0; i < sizeof(name); i++) {
            config_dev.pinNames[pin][i] = *name;
            ++name;
        } 
        return 0;       
    }
    return -1;
}

char* getPinName(int pin){
    if (config_dev.isConfigured) {
        return config_dev.pinNames[pin];
    }
}

int setDeviceName(char *name){
    if (sizeof(name) <= MAX_LEN_DEV_NAME) {
        int i;
        for (i=0; i < MAX_LEN_DEV_NAME; i++) {
            config_dev.deviceName[i] = *name;
            ++name;
        }
        return 0;
    }
    return -1;
}

char* getDeviceName() {
    if (config_dev.isConfigured) {
        return config_dev.deviceName;
    }
}

int getPinByName(char *name){
    int i, j, numPin = -1;
    int found = 0;
    for (i = 0; i < NUM_PIN; i++) {
        for (j = 0; j < MAX_LEN_PIN_NAME; j++) {
            if ((j > sizeof(name) && config_dev.pinNames[i][j] != '\0') || name[j] != config_dev.pinNames[i][j]) {
                found = 0;
                break;
            }
            else if (config_dev.pinNames[i][j] == '\0') break;
            else found = 1;
        }
        if (found == 1) {
            numPin = i;
            break;
        }
    }
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

    printf("ASKING FOR CONFIGURATION\n");

    return 0;

}

int configure(cbuf_handle_t tx_buf) {

    printf("CONFIGURAZIONE\n");

    bool dev = true;
    if (isConfigured()) {
        printf("Cambiare nome al device? [usage:Si/No]\n");
        char *res = readline(NULL);
        if (!strcmp(res,"No")) dev = false;
    }
    while (1 && dev){
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

    printf("---------------------------------------------------------------------\n");
    printf("CONFIGURAZIONE PIN ANALOGICI\n");
    printf("Pin disponibili: 0 - 1 - 2 - 3 - 4 - 5 - 6 - 7\n");
    while (!stop) {
        printf("Inserisci il numero del pin e il nome [quit per terminare]\n");
        line = readline(NULL);
        if (!strcmp(line,"quit")) {
            printf("---------------------------------------------------------------------\n");
            stop = 1;
        }
        else {
            char *token = strtok(line, " ");
            if (token == NULL) printf("[usage: numero-pin nome-pin]\n");
            else {
                int pin = atoi(token);
                if (pin < 0 || pin > 7) {
                    printf("Pin non disponibile\n");
                    continue;
                }
                token = strtok(NULL, " ");
                setPinName(pin, token);
                ConfigurationPacket cp = {
                    .command = conf,
                    .pin_num = pin,
                    .pin_name = NULL
                };

                memset(cp.pin_name, 0, MAX_LEN_PIN_NAME);
                memcpy(cp.pin_name, token, sizeof(token)-1);

                circular_buf_put(tx_buf, SOH);
                uint8_t *data = (uint8_t*) &cp;
                uint8_t checksum = 0;
                size_t size = sizeof(ConfigurationPacket);
                while (size) 
                {
                    circular_buf_put(tx_buf, *data);
                    checksum ^= *data;
                    --size;
                    ++data;
                }
                circular_buf_put(tx_buf, checksum);
                circular_buf_put(tx_buf, EOT);
            }
        }
    }

    stop = 0;
    printf("CONFIGURAZIONE PIN DIGITALI\n\n");
    printf("SWITCH/DIMMER\n");
    printf("Pin disponibili: 2 - 3 - 5 - 6 - 7 - 8 - 11 - 12\n");
    while (!stop) {
        printf("Inserisci il numero del pin e il nome [quit per terminare]\n");
        line = readline(NULL);
        if (!strcmp(line,"quit")) {
            printf("°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°\n");
            stop = 1;
        }
        else {
            char *token = strtok(line, " ");
            if (token == NULL) printf("[usage: numero-pin nome-pin]\n");
            else {
                int pin = atoi(token);
                switch (pin) {
                    case (2):
                    case (3):
                        pin = 8 + (pin - 2);
                        break;
                    case (5):
                    case (6):
                    case (7):
                    case(8):
                        pin = 8 + (pin - 3);
                        break;
                    case (11):
                    case (12):
                        pin = 8 + (pin - 5);
                        break;
                    default:
                        printf("Pin non disponibile\n");
                        continue;
                }
                token = strtok(NULL, " ");
                setPinName(pin, token);

                ConfigurationPacket cp = {
                    .command = conf,
                    .pin_num = pin,
                    .pin_name = NULL
                };

                memset(cp.pin_name, 0, MAX_LEN_PIN_NAME);
                memcpy(cp.pin_name, token, sizeof(token)-1);

                circular_buf_put(tx_buf, SOH);
                uint8_t *data = (uint8_t*) &cp;
                uint8_t checksum = 0;
                size_t size = sizeof(ConfigurationPacket);
                while (size) 
                {
                    circular_buf_put(tx_buf, *data);
                    checksum ^= *data;
                    --size;
                    ++data;
                }
                circular_buf_put(tx_buf, checksum);
                circular_buf_put(tx_buf, EOT);
            }
        }
    }
    stop = 0;
    printf("INPUT\n");
    printf("Pin disponibili: 46 - 47 - 48 - 49 - 50 - 51 - 52 - 53\n");
    while (!stop) {
        printf("Inserisci il numero del pin e il nome [quit per terminare]\n");
        line = readline(NULL);
        if (!strcmp(line,"quit")) {
            printf("---------------------------------------------------------------------\n");
            stop = 1;
        }
        else {
            char *token = strtok(line, " ");
            if (token == NULL) printf("[usage: numero-pin nome-pin]\n");
            else {
                int pin = atoi(token);
                if (pin < 46 || pin > 53) {
                    printf("Pin non disponibile\n");
                    continue;
                }
                pin = pin - 30;
                token = strtok(NULL, " ");
                setPinName(pin, token);

                ConfigurationPacket cp = {
                    .command = conf,
                    .pin_num = pin,
                    .pin_name = NULL
                };

                memset(cp.pin_name, 0, MAX_LEN_PIN_NAME);
                memcpy(cp.pin_name, token, sizeof(token)-1);

                circular_buf_put(tx_buf, SOH);
                uint8_t *data = (uint8_t*) &cp;
                uint8_t checksum = 0;
                size_t size = sizeof(ConfigurationPacket);
                while (size) 
                {
                    circular_buf_put(tx_buf, *data);
                    checksum ^= *data;
                    --size;
                    ++data;
                }
                circular_buf_put(tx_buf, checksum);
                circular_buf_put(tx_buf, EOT);
            }
        }
    }
    config_dev.isConfigured = true;
    printf("\n\nCONFIGURAZIONE TERMINATA\n\n");

    return 0;
}