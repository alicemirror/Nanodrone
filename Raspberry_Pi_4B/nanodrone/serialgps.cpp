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
    
    tcgetattr(uartFilestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    tcflush(uartFilestream, TCIFLUSH);
    tcsetattr(uartFilestream, TCSANOW, &options);
}

int SerialGPS::readNMEA() {
    // Empty the buffer
    memset(&uartBuff, '\0', sizeof(uartBuff));

    int n = read(uartFilestream, &uartBuff, sizeof(uartBuff) - 1);

    if (n <= 0) {
        // No data in the stream
        return UART_ERROR;
    } else {
        // Use the string overload character '=' for the conversion
        uartLine = uartBuff;
#ifdef _GPS_DEBUG        
cout << "readNMEA() " << uartLine << endl;
#endif        
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
    //! The nmeaparser status
    uint8_t status = NMEA_EMPTY;
    //! The number of characters read from the UART.
    int charRead;
    //! The array containing a single line of the last reading split from the buffer
    char* tokenized;
    //! The GPGGA NMEA decoded data
    NmeaGPGGA gpgga;
    //! The GPRMC NMEA decoded data
    NmeaGPRMC gprmc;

    // Read from the uart
    charRead = readNMEA();

    if(charRead != UART_ERROR) {
        // No errors ocurred
#ifdef _GPS_DEBUG
cout << "Char read from UART : " << charRead << endl <<
        "--- source buffer ---" << endl << uartLine << 
        endl << "--- end buffer ---" << endl;
#endif
        // Start splitting the buffer into null-terminated lines
        // extracting the first lline
        tokenized = strtok(uartBuff,"\n\r");

#ifdef _GPS_DEBUG
if(tokenized != nullptr) cout << "token: " << endl << tokenized << endl;
#endif 

        // Then loop until the end of the buffer
        while(tokenized != nullptr) {

            // Check for the NMEA message type of the current split line
            switch(nmeaMessageType(tokenized)) {
                case NMEA_GPGGA:
#ifdef _GPS_DEBUG
cout << "Parsing GPGGA message" << endl;
#endif 
                    parseNMEA_GPGGA(tokenized, &gpgga);

                    convertDegDec(&(gpgga.latitude), gpgga.lat, 
                                &(gpgga.longitude), gpgga.lon);

                    locationGPS.latitude = gpgga.latitude;
                    locationGPS.longitude = gpgga.longitude;
                    locationGPS.altitude = gpgga.altitude;
                    locationGPS.quality = gpgga.quality;
                    locationGPS.satellites = gpgga.satellites;
                    // Update the NMEA status
                    status |= NMEA_GPGGA;
                    break;
                case NMEA_GPRMC:
#ifdef _GPS_DEBUG
cout << "Parsing GPRMC message" << endl;
#endif 
                    parseNMEA_GPRMC(tokenized, &gprmc);
                    
                    convertDegDec(&(gprmc.latitude), gprmc.lat, 
                                &(gprmc.longitude), gprmc.lon);

                    locationGPS.latitude = gpgga.latitude;
                    locationGPS.longitude = gpgga.longitude;
                    locationGPS.speed = gprmc.speed;
                    locationGPS.course = gprmc.course;
                    // Update the NMEA status
                    status |= NMEA_GPRMC;
                    break;
            } // Switch NMEA parsers
            // Split the next token
            tokenized = strtok (nullptr, "\n\r");
#ifdef _GPS_DEBUG
if(tokenized != nullptr)
    cout << "token: " << endl << tokenized << endl;
#endif 
        } // While all tokens are split
#ifdef _GPS_DEBUG
cout << "No more tokens. Position :" << endl;
cout << "Lat,Long " << to_string(locationGPS.latitude) << 
        "," << to_string(locationGPS.longitude) << endl <<
        "Alt " << to_string(locationGPS.altitude) << "sea level" << endl <<
        "Speed " << to_string(locationGPS.speed) << endl;
#endif 
    } // If no UART errors
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
    p = strchr(p, int(',')) + 1;
    if(p == nullptr)
        return;
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;

    // Parse the latitude
    loc->latitude = atof(p);
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;

    // Saves the latitude, if any. Maybe N or S
    if( (p[0] == 'N') or (p[0] == 'S') ) {
        loc->lat = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lat = 0.0;
    }

    // Parse the longitude
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;
    loc->longitude = atof(p);
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;
 
    // Saves the longitude, if any. Maybe W or E
    if( (p[0] == 'W') or (p[0] == 'E') ) {
        loc->lon = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lon = '\0';
    }

    // Transmission quality
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;
    loc->quality = (uint8_t)atoi(p);

    // Number of satellites
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;
    loc->satellites = (uint8_t)atoi(p);

    p = strchr(p, ',')+1;
    if(p == nullptr)
        return;

    // Altitude on the sea level
    p = strchr(p, ',')+1;
    if(p == nullptr)
        return;
    loc->altitude = atof(p);
}

void SerialGPS::parseNMEA_GPRMC(char *nmea, NmeaGPRMC *loc) {
    //! NMEA local source data array (will be changed)
    char *p = nmea;

    // Find the location information inside the GPGA string 
    // (splits the data before)
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;

    // Parse the latitude
    loc->latitude = atof(p);
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;

    // Saves the latitude, if any. Maybe N or S
    if( (p[0] == 'N') or (p[0] == 'S') ) {
        loc->lat = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lat = '\0';
    }

    // Parse the longitude
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;
    loc->longitude = atof(p);
    p = strchr(p, ',')+1;
    if(p == nullptr)
        return;
 
    // Saves the longitude, if any. Maybe W or E
    if( (p[0] == 'W') or (p[0] == 'E') ) {
        loc->lon = p[0];
    } else if(p[0] == ',') {
        // Add the string terminator
        loc->lon = '\0';
    }

    // Satellite speed
    p = strchr(p, ',') + 1;
    if(p == nullptr)
        return;
    loc->speed = atof(p);

    // Satellite direction
    p = strchr(p, ',')+ 1 ;
    if(p == nullptr)
        return;
    loc->course = atof(p);
}

uint8_t SerialGPS::nmeaMessageType(const char *message) {
    //! Calculated checksum of the message
    uint8_t checksum = 0;

//    if(checksum = nmeaVerifyChecksum(message) != NMEA_EMPTY)
//        return NMEA_CHECKSUM_ERR; // Checksum error

    if (strstr(message, NMEA_GPGGA_STR) != nullptr) {
        return NMEA_GPGGA; // Valid checksum, GPGGA message type
    }

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
    uint8_t checksum;
    char p;
    uint8_t sum = 0;

    if(strchr(message, '*') != nullptr) {
        checksum = (uint8_t)strtol(strchr(message, '*') + 1, NULL, 16);
    } else {
        return NMEA_CHECKSUM_ERR;
    }

    // Calculate the checksum
    ++message;
    p = *message;
    
#ifdef _GPS_DEBUG        
cout << "Checksum " << to_string(checksum) << endl << " Message: " << message << endl;
#endif
    
    while (p != '*') {
        message++;
        p = *message;
        sum ^= (p);
    }

    if (sum != checksum) {
        return NMEA_CHECKSUM_ERR;
    }

    return NMEA_EMPTY;
}
