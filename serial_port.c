#include "serial_port.h"


int s_open(const char* name) {

    int serial_port = open(name, O_RDWR | O_NOCTTY | O_SYNC);

    // Check for errors
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    return serial_port;
}

int s_set_attributes(int serial_port) {

    // Create new termios struc
    struct termios tty;

    // NOTE: POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~ (PARENB | CSTOPB | CSIZE | CRTSCTS);
    tty.c_cflag |= CS8 | CREAD | CLOCAL;

    tty.c_lflag &= ~ (ICANON | ECHO | ECHOE | ECHONL | ISIG);

    tty.c_iflag &= ~ (IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~ (OPOST | ONLCR);

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    cfsetispeed(&tty, B19200);
    cfsetospeed(&tty, B19200);

    

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    return 0;
}

int s_close(int serial_port) {

    close (serial_port);
    return 0;
    
}
