#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
  
  //open serial port
  int s_open(const char* name);

  //sets port attributes
  int s_set_attributes(int serial_port);

