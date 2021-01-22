#
# A very simple makefile
#

# The default C compiler
CC = gcc

# The CFLAGS variable sets compile flags for gcc:
#  -g          compile with debug information
#  -Wall       give verbose compiler warnings
#  -O0         do not optimize generated code
#  -std=gnu99  use the GNU99 standard language definition
CFLAGS = -g -O0 -std=gnu99 -Wall

LDFLAGS = -lm -lbluetooth 

SOURCES = smarthouse_client.c configuration.c packet_handler_client.c shell.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = smarthouse_client

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJECTS) core

