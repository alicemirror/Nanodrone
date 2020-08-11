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

// --------------------------------------------------------------------------
//                      Serial communication methods
// --------------------------------------------------------------------------

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
    options.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    options.c_oflag = 0;
    options.c_lflag = 0;
    
    tcflush(uartFilestream, TCIFLUSH);
    tcsetattr(uartFilestream, TCSANOW, &options);
}

int SerialGPS::readNMEA() {
    // Empty the buffer
    memset(&uartBuff, '\0', sizeof(uartBuff));
    
    int n = read(uartFilestream, &uartBuff, sizeof(uartBuff));

    if (n <= 0) {
        // No data in the stream
        return UART_ERROR;
    } else {
        // Use the string overload character '=' for the conversion
        uartLine = uartBuff;
        return n;
    }
}

string SerialGPS::getNMEA() {
    readNMEA();
    return uartLine;
}

void SerialGPS::closeUART() {
    if( (hasPort == true) && (isOpen == true) ) {
        close(uartFilestream);
    }
}

// --------------------------------------------------------------------------
//                      GPS location methods
// --------------------------------------------------------------------------

GPSLocation* SerialGPS::getLocation() {
                
    convertGPSDecimalLocation();
    return &locationGPS;
    }

void SerialGPS::convertGPSDecimalLocation() {
    uint8_t status = NMEA_EMPTY;

    while(status != NMEA_COMPLETED) {
        NmeaGPGGA gpgga;
        NmeaGPRMC gprmc;

        // Read an NMEA line from the serial stream
        if(readNMEA() != UART_ERROR) {
        
            cout << "NMEA read" << endl;
            
            cout << uartLine << endl;
            
            switch(nmeaMessageType(uartBuff)) {
                case NMEA_GPGGA:
                
                    cout << "GPGGA - Status " << to_string(status) << endl;
                
                    parseNMEA_GPGGA(uartBuff, &gpgga);

                    convertDegDec(&(gpgga.latitude), gpgga.lat, 
                                &(gpgga.longitude), gpgga.lon);

                    locationGPS.latitude = gpgga.latitude;
                    locationGPS.longitude = gpgga.longitude;
                    locationGPS.altitude = gpgga.altitude;

                    status |= NMEA_GPGGA;
                    break;
                case NMEA_GPRMC:
                
                    cout << "GPRMC - Status " << to_string(status) << endl;

                    /* Temporary disable. Check the parsing method
                    parseNMEA_GPRMC(uartBuff, &gprmc);

                    locationGPS.speed = gprmc.speed;
                    locationGPS.course = gprmc.course;
                    */
                    status |= NMEA_GPRMC;
                    break;
            }
        }
    }
}

void SerialGPS::convertDegDec(double *latitude, char ns,  
                              double *longitude, char we) {

    double lat = (ns == 'N') ? *latitude : -1 * (*latitude);
    double lon = (we == 'E') ? *longitude : -1 * (*longitude);

    *latitude = converDecimalDegreesToDecimal(lat);
    *longitude = converDecimalDegreesToDecimal(lon);
}

double SerialGPS::converDecimalDegreesToDecimal(double deg_point) {
    
    double ddeg;
    double sec = modf(deg_point, &ddeg) * 60;
    int deg = (int)(ddeg / 100);
    int min = (int)(deg_point - (deg * 100));

    double absdlat = round(deg * 1000000.);
    double absmlat = round(min * 1000000.);
    double absslat = round(sec * 1000000.);

    return round(absdlat + (absmlat / 60) + (absslat / 3600)) / 1000000;
}

// --------------------------------------------------------------------------
//                      NMEA parsing methods
// --------------------------------------------------------------------------

void SerialGPS::parseNMEA_GPGGA(char *nmea, NmeaGPGGA *loc) {
    //! NMEA local source data array (will be changed)
    char *p = nmea;

    // Find the location information inside the GPGA string 
    // (splits the data before)
    p = strchr(p, ',') + 1;
    p = strchr(p, ',') + 1;

    // Parse the latitude
    loc->latitude = atof(p);
    p = strchr(p, ',') + 1;

    // Saves the latitude, if any. Maybe N or S
    if( (p[0] == 'N') or (p[0] == 'S') ) {
        loc->lat = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lat = '\0';
    }

    // Parse the longitude
    p = strchr(p, ',') + 1;
    loc->longitude = atof(p);
    p = strchr(p, ',') + 1;
 
    // Saves the longitude, if any. Maybe W or E
    if( (p[0] == 'W') or (p[0] == 'E') ) {
        loc->lon = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lon = '\0';
    }

    // Transmission quality
    p = strchr(p, ',') + 1;
    loc->quality = (uint8_t)atoi(p);

    // Number of satellites
    p = strchr(p, ',') + 1;
    loc->satellites = (uint8_t)atoi(p);

    p = strchr(p, ',')+1;

    // Altitude on the sea level
    p = strchr(p, ',')+1;
    loc->altitude = atof(p);
}

void SerialGPS::parseNMEA_GPRMC(char *nmea, NmeaGPRMC *loc) {
    //! NMEA local source data array (will be changed)
    char *p = nmea;

    cout << "parseNMEA_GPRMC" << endl << nmea << endl << endl;

    // Find the location information inside the GPGA string 
    // (splits the data before)
    p = strchr(p, ',') + 1;
    p = strchr(p, ',') + 1;

    // Parse the latitude
    loc->latitude = atof(p);
    p = strchr(p, ',') + 1;

    // Saves the latitude, if any. Maybe N or S
    if( (p[0] == 'N') or (p[0] == 'S') ) {
        loc->lat = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lat = '\0';
    }

    // Parse the longitude
    p = strchr(p, ',') + 1;
    loc->longitude = atof(p);
    p = strchr(p, ',')+1;
 
    // Saves the longitude, if any. Maybe W or E
    if( (p[0] == 'W') or (p[0] == 'E') ) {
        loc->lon = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lon = '\0';
    }

    // Satellite speed
    p = strchr(p, ',')+1;
    loc->speed = atof(p);

    // Satellite direction
    p = strchr(p, ',')+1;
    loc->course = atof(p);
}

uint8_t SerialGPS::nmeaMessageType(const char *message) {
    //! Calculated checksum of the message
    uint8_t checksum = 0;

    if(checksum = nmeaVerifyChecksum(message) != NMEA_EMPTY)
        return checksum; // Checksum error

    if (strstr(message, NMEA_GPGGA_STR) != nullptr)
        return NMEA_GPGGA; // Valid checksum, GPGGA message type

    if (strstr(message, NMEA_GPRMC_STR) != nullptr)
        return NMEA_GPRMC; // Valid checksum, GPRMC message type

    return NMEA_UNKNOWN; // Unknown message type (or not useful for parsing)
}

/**
 * Calculate the checksum of the NMEA message and compare is with the value 
 * included in the message itself. The method returns the error condition or
 * _EMPTY if the checksum matches.
 * 
 * @param The NMEA message
 */
uint8_t SerialGPS::nmeaVerifyChecksum(const char *message) {
    //! Checksum extracted from the message
    uint8_t checksum = (uint8_t)strtol(strchr(message, '*')+ 1, NULL, 16);
    char p;
    uint8_t sum = 0;

    // Calculate the checksum
    ++message;
    while ((p = *message++) != '*') {
        sum ^= p;
    }

    if (sum != checksum) {
        return NMEA_CHECKSUM_ERR;
    }

    return NMEA_EMPTY;
}
