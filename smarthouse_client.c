#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h> 
#include <stdbool.h>

#include "constants.h"
#include "commands.h"
#include "buffer_client.h"
#include "serial_port.h"
#include "configuration.h"
#include "shell.h"
#include "packets.h"
#include "packet_handler_client.h"

int run, fd, printed, configuration_phase, configuration_found;
cbuf_handle_t tx_buf;
uint8_t *tx_buffer;

int main()
{
    fd = s_open("/dev/ttyACM0");
    if (fd < 0) return 0;
    s_set_attributes(fd);

    tx_buffer  = malloc(BUFFER_MAX_SIZE * sizeof(uint8_t));
    tx_buf = circular_buf_init(tx_buffer, BUFFER_MAX_SIZE);
   
    pthread_t packetHandler;

    run = 1;
    printed = 0;

    configInit();

    configuration_phase = 1;
    configuration_found = 0;

    pthread_create (&packetHandler, NULL, packetHandlerFunction, NULL);

    usleep(1000000);

    getOldConfig(tx_buf);

    while (configuration_phase);

    if (!configuration_found) configure(tx_buf);
    else set_isConfigured();

    printConfiguration();
    
    printCommand();

    shellFunction();

    pthread_join(packetHandler, NULL);

    free(tx_buffer);
    circular_buf_free(tx_buf);

    s_close(fd);

    return 0;
}