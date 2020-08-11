/**
 * @file serialgps.h
 * @brief Class to manage the serial stream from a GPS receiver.
 * 
 * The GPS is expected sending string lines in the NMEA format. For a full
 * documentation on the NMEA format see the following link: 
 * https://www.gpsinformation.org/dale/nmea.htm 
 * 
 * @todo Optimize and make more cpp compliant the NMEA data parsing.
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <iostream>
#include <string.h>
#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
// Specific for Lnux
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

// Undef below to remove the class debug messages
#undef _GPS_DEBUG

using namespace std;

// UART states -------------------------------------------------------
//! Serial stream read error
#define UART_ERROR -1
//! Serial strea Ok
#define UART_OK 0
//! Default buffer size for the NMEA stream lines read from the uart
#define UART_BUF_SIZE 256

// NMEA parameters ---------------------------------------------------
#define NMEA_EMPTY 0x00
#define NMEA_GPRMC 0x01
#define NMEA_GPRMC_STR "$GPRMC"
#define NMEA_GPGGA 0x02
#define NMEA_GPGGA_STR "$GPGGA"
#define NMEA_UNKNOWN 0x00
#define NMEA_COMPLETED 0x03
#define NMEA_CHECKSUM_ERR 0x80
#define NMEA_MESSAGE_ERR 0xC0

// NMEA Structure for data parsing ------------------------------------
//! The data decoded from the GPGGA type GPS messages
struct NmeaGPGGA {
    //! Latitude in the format XXYY.ZZKK.. DEG, MIN, SEC.SS
    double latitude;
    //! Latitude N, S
    char lat;
    //! Longitude in the format XXXYY.ZZKK.. DEG, MIN, SEC.SS
    double longitude;
    //! Longitude E, W
    char lon;
    //! Signal quality index 0 - n
    uint8_t quality;
    //! Current number of satellites
    uint8_t satellites;
    //! Sea level altitude in meters
    double altitude;
};

//! The data decoded from the GPRMC type GPS messages
struct NmeaGPRMC {
    //! Latitude in the format XXYY.ZZKK.. DEG, MIN, SEC.SS
    double latitude;
    //! Latitude N, S
    char lat;
    //! Longitude in the format XXXYY.ZZKK.. DEG, MIN, SEC.SS
    double longitude;
    //! Longitude E, W
    char lon;
    //! Speed
    double speed;
    //! Direction
    double course;
};

//! Defines a GPS location in decimal representation
struct GPSLocation {
    //! Latitude
    double latitude;
    //! Longitude
    double longitude;
    //! Speed
    double speed;
    //! Altitude on the sea level
    double altitude;
    //! Direction
    double course;
    //! Signal quality index 0 - n
    uint8_t quality;
    //! Current number of satellites
    uint8_t satellites;

};


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
     * Close the UART stream if it has been previously opened.
     */
    void closeUART();
     
     /**
      * Set the UART port name
      * 
      * @todo Check the input string has a valid port name
      */
    void setUARTPort(string port);
    
    /**
     * Retrieve the current GPS location
     * 
     * @return A pointer to a GPSLocation structure
     */
    GPSLocation* getLocation();
    
    /**
     * Return the last NMEA line read from the uart.
     * 
     * @return The last NMEA line
     */
    string getNMEA();
    
private:
    //! The serial file stream
    int uartFilestream;
    //! The serial port name
    string uartPort;
    //! The last line read from the serial port
    string uartLine;
    //! The char buffer with the last line read from the serial
    char uartBuff[UART_BUF_SIZE];
    //! Flag is true when the UART stream has been opened successfully
    bool isOpen = false;
    //! Flag is true if the UART prot name has been defined
    bool hasPort = false;
    //! The last GPS coordinates position, in decimal format
    GPSLocation locationGPS;
    
    /**
     * Convert the last NMEA read data to decimal notation coordinates.
     */
    void convertGPSDecimalLocation();
    
    /**
     * Read a line from the serial if there are data available.
     * Return UART_ERROR if no data are available or the length of the line.
     * 
     * This method saves both the class NMEA data buffer for further processing
     * (coordinates decoging and GPS data parsing), as well a string that can
     * be retrieved with the getNMEA() method. If no data are read, the last
     * value of the NMEA string remain unchanged.
     */
    int readNMEA();

    /**
     * Parse the GPGGA NMEA string and update the loc sturcture (lat, long, ecc)
     * 
     * @note This method is complementary to the parseNMEA_GPRMC() as it also 
     * updates transmission quality, number of satellites, and altitude while
     * the parseNMEA_GPRMC() updates satellite speed, and direction as well.
     * 
     * @param nmea The NMEA string from the GPS stream
     * @param loc The location structure to be updated
     */
    void parseNMEA_GPGGA(char *nmea, NmeaGPGGA *loc);

    /**
     * Parse the GPRMC NMEA string and update the loc sturcture (lat, long, ecc)
     * 
     * @note This method is complementary to the parseNMEA_GPGA() as it also 
     * updates satellite speed, and direction while the parseNMEA_GPGA() updates 
     * transmission quality, number of satellites, and altitude as well.
     * 
     * @param nmea The NMEA string from the GPS stream
     * @param loc The location structure to be updated
     */
    void parseNMEA_GPRMC(char *nmea, NmeaGPRMC *loc);
    
    /**
     * Get the message type (GPGGA, GPRMC, etc..) and verify the checksoum
     * validity.
     *
     * This function filters out wrong messages (invalid checksum)
     *
     * @param message The NMEA message
     * @return The type of message if it is valid or the checksum error
     */
    uint8_t nmeaMessageType(const char *message);

    /**
     * Calculate the checksum of the NMEA message and compare is with the value 
     * included in the message itself. The method returns the error condition or
     * _EMPTY if the checksum matches.
     * 
     * @param message The NMEA message
     */
    uint8_t nmeaVerifyChecksum(const char *message);

    /**
     * Convert an NMEA coordinates representation to absolute decimal representation
     * 
     * The N-S and W-E of the NMEA position changes the sign of the decimal
     * converted value.
     *
     * @param latitude NMEA latitude position
     * @param ns The char indicating the orientation, N or S
     * @param longitude NMEA longitude position
     * @param we The char indicating the orientation, W or E
     */
    void convertDegDec(double *latitude, char ns, double *longitude, char we);
    
    /**
     * Convert the decimal representation of Degs (hhmm.sec) in decimal absolute
     * value.
     */
    double converDecimalDegreesToDecimal(double deg_point);
};

#endif
