typedef struct Pins Pins;

typedef struct {
    int num;
    int ind;
} numPin;

void configInit();

void set_isConfigured();

void setPinName(uint8_t pin, char* name);

char* getPinName(uint8_t pin, uint8_t k);

void setDeviceName(char* name);

char* getDeviceName();

numPin getPinByName(char* name);

int getOldConfig(cbuf_handle_t tx_buf);

int resetConfig(cbuf_handle_t tx_buf);

int configure(cbuf_handle_t tx_buf);

void printConfiguration();
