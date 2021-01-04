// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

  
  //open serial port
  int s_open(const char* name);

  //sets port attributes
  int s_set_attributes(int serial_port);

  //close serial port
  int s_close(int serial_port);