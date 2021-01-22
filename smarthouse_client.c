#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h> 
#include <stdbool.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "../common/constants.h"
#include "../common/commands.h"
#include "../common/packets.h"
#include "configuration.h"
#include "shell.h"
#include "packet_handler_client.h"

int fd;

OperationPacket op = {
    .command = 0,
    .pin_num = 0,
    .intensity = 0
};

ConfigurationPacket cp = {
    .command = 0,
    .pin_num = 0,
    .pin_name = 0
};

ControlPacket rc = {
    .command = 0
};

void main()
{
    FILE *fdev = NULL;
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i, n, ret;
    char dest[18] = {""};
    char name[248] = {""};
	struct sockaddr_rc addr = { 0 };
    int status = 1;
	
	if((fdev = fopen("device.txt", "r+t")) == NULL) {
        if ((fdev = fopen("device.txt", "w+t")) == NULL) {
            printf("\nErrore nella creazione del file per memorizzare l'indirizzo del device\n\n");
            return;
        }
    }   
    do {
        fscanf(fdev, "%s", dest);
        if ((strlen(dest) != 17) || (status < 0)) {
            printf("\nscansione device in esecuzione\n\n");
            dev_id = hci_get_route(NULL);                       //searches for the first local bluetooth adapter available and returns the resource number
            sock = hci_open_dev( dev_id );                      //allocates resources to use that adapter (opens socket connection to the microcontroller of the local adapter)
            if (dev_id < 0 || sock < 0) {
                perror("opening socket");
                fclose(fdev);
                exit(1);
            }
            len = 8;
            max_rsp = 20;
            flags = IREQ_CACHE_FLUSH;                                           //the cache of previously detected devices is flushed before performing the current inquiry
            ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
            num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);      //scans for nearby Bluetooth devices
            if( num_rsp < 0 ) {
                perror("hci_inquiry");
                free(ii);
                close(sock);
                fclose(fdev);
                return;
            } 
            for (i = 0; i < num_rsp; i++) {
                ba2str(&(ii+i)->bdaddr, dest);                                  //bluetooth address to string
                memset(name, 0, sizeof(name));
                if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)     //tries to query the user-friendly name of the device
                    strcpy(name, "[unknown]");
                printf("%3d) %s %s\n",i , dest, name);
            }
            printf("\nscegliere device [n] ([-1] per uscire): ");
            scanf("%d%*c", &n);
            if (n == -1) {
                free(ii);
                close(sock);
                fclose(fdev);
                return;
            }
            else if (n >= num_rsp) {
                printf("\ndevice non presente nell'elenco\n");
                free(ii);
                close(sock);
                fclose(fdev);
                return;   
            }
            ba2str(&(ii+n)->bdaddr, dest);
            if (freopen("device.txt", "wt", fdev) == NULL) {
                printf("\nErrore nella creazione del file per memorizzare l'indirizzo del device\n\n");
                fclose(fdev);
                return;
            }
            fprintf(fdev,"%s", dest);          
            free(ii);
            close(sock);
        }
        fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);                         // allocate a socket with RFCOMM protocol 
        addr.rc_family = AF_BLUETOOTH;                                                  // addressing family of the socket
        addr.rc_channel = (uint8_t) 1;                                                  // port number to connect to
        str2ba( dest, &addr.rc_bdaddr );                                                // chosen device name to bluetooth address 
        printf ("\ntentativo di connessione in corso\n\n");
        status = connect(fd, (struct sockaddr *)&addr, sizeof(addr));                   // connect to arduino
        if (status < 0) printf("connessione non riuscita - scegliere nuova device\n\n");
    }
    while ( status < 0 );
    usleep(1000000);
    configInit();
    ret = getOldConfig();
    if ( ret == -1) {
        printf("\nBluetooth non funzionante o Connessione caduta\n\n");
        return;
    }
    else if (ret == 0) configure();
    printConfiguration();
    printCommand();
    ret = shellFunction();
    if ( ret == -1) {
        printf("\nBluetooth non funzionante o Connessione caduta\n\n");
        return;
    }
    fclose(fdev); 
    close(fd);
    return;
}