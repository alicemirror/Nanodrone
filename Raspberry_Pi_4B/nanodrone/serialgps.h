/**
 * @file serialgps.h
 * @brief Class to manage the serial stream from a GPS receiver.
 * 
 * The GPS is expected sending string lines in the NMEA format. For a full
 * documentation on the NMEA format see the following link: 
 * https://www.gpsinformation.org/dale/nmea.htm 
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <iostream>
#include <string.h>
#include <bits/stdc++.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

//! Serial stream read error
#define UART_ERROR -1
//! Serial strea Ok
#define UART_OK 0

using namespace std;

/**
 * SerialGPS manages the UART connection and receives the GPS stream
 */
class SerialGPS {
    
public:
    /**
     * Class constructor
     */
    SerialGPS();
    
    /**
     * Class constructor
     * 
     * @param port The name of the linux device *(e.g. /dev/tty...)
     */
    SerialGPS(string port);
    
    /**
     * Class destructor. Closes the serial stream.
     * 
     * @todo Check if the stream is open
     */
    ~SerialGPS();
    
    /**
     * Initialize the UART stream opening the connection.
     * 
     * The uart stream is connected with the following flags:
     * O_RDWR | O_NOCTTY | O_NDELAY
     * 
     * @return The status of the UART
     */
    int openUART();
    
    /**
     * Configure the serial with the default parameters to receive the GPS data.
     */
    void configUART();
    
    /**
     * Read a line from the serial if there are data available.
     * 
     * Return UART_ERROR if no data are available or the length of the line
     */
    int readGPS();

    /**
     * Close the UART stream if it has been previously opened.
     */
    void closeUART();
    
    /**
     * Return the last line read from the GPS stream
     */
    string getGPS();
     
     /**
      * Set the UART port name
      * 
      * @todo Check the input string has a valid port name
      */
    void setUARTPort(string port);
    
private:
    //! The serial file stream
    int uartFilestream;
    //! The serial port name
    string uartPort;
    //! The last line read from the serial port
    string uartLine;
    //! Flag is true when the UART stream has been opened successfully
    bool isOpen = false;
    //! Flag is true if the UART prot name has been defined
    bool hasPort = false;
};

#endif
