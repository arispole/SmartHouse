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

#include "constants.h"
#include "commands.h"
#include "packets.h"
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
            dev_id = hci_get_route(NULL);
            sock = hci_open_dev( dev_id );
            if (dev_id < 0 || sock < 0) {
                perror("opening socket");
                exit(1);
            }
            len = 8;
            max_rsp = 20;
            flags = IREQ_CACHE_FLUSH;
            ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
            num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
            if( num_rsp < 0 ) {
                perror("hci_inquiry");
                free(ii);
                close(sock);
                return;
            } 
            for (i = 0; i < num_rsp; i++) {
                ba2str(&(ii+i)->bdaddr, dest);
                memset(name, 0, sizeof(name));
                if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
                    strcpy(name, "[unknown]");
                printf("%3d) %s %s\n",i , dest, name);
            }
            printf("\nscegliere device [n] ([-1] per uscire): ");
            scanf("%d", &n);
            if (n == -1) {
                free(ii);
                close(sock);
                return;
            }
            else if (n >= num_rsp) {
                printf("\ndevice non presente nell'elenco\n");
                free(ii);
                close(sock);
                return;   
            }
            ba2str(&(ii+n)->bdaddr, dest);
            if ((fdev = fopen("device.txt", "wt")) == NULL) {
                printf("\nErrore nella creazione del file per memorizzare l'indirizzo del device\n\n");
                return;
            }
            fprintf(fdev,"%s", dest);
            fclose(fdev);          
            free(ii);
            close(sock);
        }
        fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);  // allocate a socket
        addr.rc_family = AF_BLUETOOTH;   // set the connection parameters
        addr.rc_channel = (uint8_t) 1;   //
        str2ba( dest, &addr.rc_bdaddr ); // 
        printf ("\ntentativo di connessione in corso\n\n");
        status = connect(fd, (struct sockaddr *)&addr, sizeof(addr));  // connect to server
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