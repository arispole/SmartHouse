# "Arduino Smart-House with web-server and Bluetooth" Project

<ins>WHAT</ins>

The project consists in the realization of two programs written in C language.

A program developed for the control of a microcontroller with ATMEGA2560 microprocessor, according to the following specifications:
- control of 8 switches/dimmers
- reading of 8 ADC channels
- reading of 8 digital inputs
- memorization of the configuration on the microcontroller’s EEPROM 

A PC program, in Linux environment, which consists of an interface with the microcontroller to send commands and interrogate it.

<ins>HOW</ins>

Communication occurs via Bluetooth interface, having equipped the microcontroller with the DSD TECH SH-H3 module (Bluetooth 2.0). 
The communication protocol takes place by sending packets, which are provided with a start and an end character. The packages are of 3 types:
- configuration packet → to send and receive device and pin names
- operation packet → to send commands and to receive the readings
- control packet → for communication control and to read or reset the configuration

In addition, the protocol provides for an error check via checksum, an ACK (for the acknowledgment of the receipt), and a NACK (to request the return of the packet). 

<ins>HOW TO RUN</ins>

*Compile*

To compile and load the program into the microcontroller, open the terminal in the “arduino” directory and execute the *make clean* command and then *make smarthouse.hex*.

To compile the program for the PC, open the terminal in the "client" directory and execute the *make* command.

*Execute*

To run the client shell, execute the *./smarthouse_client* command. The use of the latter is guided and assisted right from the start of the connection.
