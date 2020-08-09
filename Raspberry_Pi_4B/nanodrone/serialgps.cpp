/**
 * @file serialgps.cpp
 * @brief Class to manage the serial stream from a GPS receiver.
 *
 */

#include "serialgps.h"

SerialGPS::SerialGPS() {
    uartFilestream = -1;
    uartLine = "";
    isOpen = false;
    hasPort = false;
}

SerialGPS::SerialGPS(string port) {
    setUARTPort(port);
    uartFilestream = -1;
    uartLine = "";
    isOpen = false;
}

SerialGPS::~SerialGPS() {
    closeUART();
}

void SerialGPS::setUARTPort(string port) {
    uartPort = port;
    hasPort = true;
}

int SerialGPS::openUART() {
    const char* p = uartPort.c_str();
    
    uartFilestream = open(p, O_RDWR | O_NOCTTY | O_NDELAY);

    if (uartFilestream == -1) {
        isOpen = false;
        return UART_ERROR;
    } else {
        isOpen = true;
        return UART_OK;
    }
}

void SerialGPS::configUART() {
    struct termios options;
    
    tcgetattr(uartFilestream, &options);
    
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    
    tcflush(uartFilestream, TCIFLUSH);
    tcsetattr(uartFilestream, TCSANOW, &options);
}

int SerialGPS::readGPS() {
    //! Serial line buffer. GPS sentences are shorter than 128 char
    char read_buf [128];
    // Empty the buffer
    memset(&read_buf, '\0', sizeof(read_buf));
    
    int n = read(uartFilestream, &read_buf, sizeof(read_buf));

    if (n <= 0) {
        // No data in the stream
        return UART_ERROR;
    } else {
        // Use the string overload character '=' for the conversion
        uartLine = read_buf;
        return n;
    }
}

string SerialGPS::getGPS() {
    return uartLine;
}

void SerialGPS::closeUART() {
    if( (hasPort == true) && (isOpen == true) ) {
        close(uartFilestream);
    }
}
