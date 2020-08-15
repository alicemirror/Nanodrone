/**
@file firstfly.cpp

@brief Automatic shooting with data retrieval. The application after started
initializes the ssytem and start shooting images at the desired interval until
the control switch is on.
It is useful to run in background. If launched by cron or run in background with
the terminal command suffix '&' keep disabled the logging features (need to be
recompiled)

@author Enrico Miglino <balearicdynamics@gmail.com>
@version 1.0
@date Augut 2020
*/

#include "firstfly.h"

using namespace std;

/* ----------------------------------------------------------------------
 * Application functions and textual interface
   ---------------------------------------------------------------------- */

//! Print program version number
void pVersion() {
    cout << "First Fly " << testlens_VERSION_MAJOR <<
            "." << testlens_VERSION_MINOR << "." <<
            testlens_VERSION_BUILD << endl <<
            " Image Processor " << PROCESSOR_MAJOR << "." <<
            PROCESSOR_MINOR << "." << PROCESSOR_BUILD << endl;
}

/**
 * Flash the notification LED for 18 seconds before starting the acqusition sequence.
 * 
 * @todo Make this function better, maybe using PWM
 */
void preFlight() {
    // Blink every second
    for(int j = 0; j < 6; j++) {
        digitalWrite(LED_PIN, true);
        delay(1000);
        digitalWrite(LED_PIN, false);
        delay(1000);
    }
    for(int j = 0; j < 12; j++) {
        digitalWrite(LED_PIN, true);
        delay(500);
        digitalWrite(LED_PIN, false);
        delay(500);
    }
    for(int j = 0; j < 24; j++) {
        digitalWrite(LED_PIN, true);
        delay(250);
        digitalWrite(LED_PIN, false);
        delay(250);
    }
}

//! Blink the control signal 200 ms
void testFlash() {
    digitalWrite(LED_PIN, true);
    delay(50);
    digitalWrite(LED_PIN, false);
    }

/**
 * Initialize the camera driver and set it to jpg mode
 * 
 * @return The camera initialization status
 */
int initCamera() {
    uint8_t temp, pid, vid;

    cout << CAMERA_STARTING << endl;

    // Check if the ArduCAM SPI bus is OK
    Cam5642.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = Cam5642.read_reg(ARDUCHIP_TEST1);
    // Check the SPI interface status
    if(temp != 0x55) {
        outMessage(CAMERA_ERROR_ON_START);
        return CAM_SPI_ERROR;
    }

    // Change camera MCU mode
    Cam5642.write_reg(ARDUCHIP_MODE, 0x00); 
    Cam5642.wrSensorReg16_8(0xff, 0x01);
    Cam5642.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    Cam5642.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);

    // Check if the camera is detected
    if((vid != 0x56) || (pid != 0x42)) {
        outMessage(CAMERA_ERROR_ON_START);
        return CAM_NOT_FOUND;
    } else {
        // Setting image capture mode require 0.568 ms
        Cam5642.set_format(JPEG);
        // Initialization take a long time, 15.84 sec.
        Cam5642.InitCAM();
        outMessage(CAMERA_STARTED);
        return CAM_INIT_OK;
    }
}

//! Clear screen (used by the terminal textual interface)
void cls() {
    cout << "\033[2J\033[1;1H";
}

//! Create the image file name as timestamped unique name.
string createImageFileName() {
    return string(REPORT_FOLDER) + string(TEST_FILE) + string("_") + 
            getDateSuffix() + string(".jpg");
}

/** 
 * Create the image file name to store a CV Mat image object. This image
 * contains the processed image of the last captured camera image
 */
string createMatFileName() {
    return string(REPORT_FOLDER) + string("_") + string(TEST_FILE) + 
            string("_") + getDateSuffix() + string(".jpg");
}

//! Display a log message with image name.
void writeLog(string message, string image) {
    cout << getLogTimestamp() << " | " << message << " | " << image << endl;
}

//! Display a log message.
void writeLog(string message) {
    cout << getLogTimestamp() << " | " << message << endl;
}

//! Return the date suffix in the format yyyy-mm-dd-hhmmss to make unique strings
//! for file names.
string getDateSuffix() {
    time_t now = time(NULL);
    struct tm tstruct;
    char buf[40];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d-%H.%M.%S", &tstruct);
    return buf;
}

//! Return the date in the format neede by the log record field.
string getLogTimestamp() {
    time_t now = time(NULL);
    struct tm tstruct;
    char buf[40];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tstruct);
    return buf;
}

//! Output a camera status message
void outCamError(int code) {
    cout << getLogTimestamp() << " - " << msgCam[code] << endl;
}

//! Show a message to the terminal
void outMessage(string msg) {
    cout << getLogTimestamp() << " - " << msg << endl;
}

//! Show the current equalization parameters applied to the caputred images
void showEqParams(LightIndexes* lc) {
    cout << "Eq param: light index " << lc->lightingIndex <<
            " light % " << lc->lightingPerc << 
            " max retries " << lc->maxExposureAdjust << endl;
}

/**
	Shows the initial message when program starts
*/
void help() {
    cls();
	cout << CON_DASHES << endl;
    pVersion();
	cout << CON_DASHES << endl;
}

/* ----------------------------------------------------------------------
 * Image acquisition and camera driver settings
   ---------------------------------------------------------------------- */

/**
 * Initialize the camera driver.
 * 
 * The camera initialization needs about 15 seconds.
 */
int startForCapture() {
    // Initialize the camera driver
    int camInitStatus;
    outCamError(camInitStatus = initCamera());
    if(camInitStatus != CAM_INIT_OK) {
        return CAM_INIT_ERROR;
    } else {
        return CAM_INIT_OK;
    }
}

/**
 * Capture an image from the camera according to the current settings
 * 
 * At the end of the capture the image is in the camera buffer
 */
void captureImage() {
    sleep(1); // Let auto exposure do it's thing after changing image settings
    // VSYNC is active HIGH
    Cam5642.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		
     // Flush the FIFO
    Cam5642.flush_fifo();    
    // Clear the capture done flag
    Cam5642.clear_fifo_flag();
    // Capture an image
    Cam5642.start_capture();
    // Wait for the triggering image capture end
    while (!(Cam5642.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)){ }
}

/**
 * Save the image to file
 */
int saveImage(string fn) {
    uint8_t temp = 0, temp_last = 0;
    const char* fnp = fn.c_str();

    FILE *fp1 = fopen(fnp, "w+");   
    if (!fp1) {
        return CAM_FILE_ERROR;
        }
    size_t length = Cam5642.read_fifo_length();
    if (length >= MAX_FIFO_SIZE) {
        return CAM_BUF_OVERSIZE;
        } 
    else if (length == 0 ) {
        return CAM_BUF_ZERO;
        } 

    int32_t i = 0;
    Cam5642.CS_LOW();      
    Cam5642.set_fifo_burst();

    while(length--) {
        temp_last = temp;
        temp =  Cam5642.transfer(0x00);
        // Read JPEG data from FIFO and if find the end break while
        if ( (temp == 0xD9) && (temp_last == 0xFF) ) {
            buf[i++] = temp;  //save the last  0XD9     
            //Write the remain bytes in the buffer
            Cam5642.CS_HIGH();
            fwrite(buf, i, 1, fp1);    
            //Close the file
            fclose(fp1); 
            is_header = false;
            i = 0;
        }
        if (is_header == true) { 
            if (i < BUF_SIZE) { 
                buf[i++] = temp;
            } 
            else {
                // Write BUF_SIZE bytes image data to file
                Cam5642.CS_HIGH();
                fwrite(buf, BUF_SIZE, 1, fp1);
                i = 0;
                buf[i++] = temp;
                Cam5642.CS_LOW();
                Cam5642.set_fifo_burst();
            }        
        }
        else if ((temp == 0xD8) & (temp_last == 0xFF)) {
            is_header = true;
            buf[i++] = temp_last;
            buf[i++] = temp;
        }
    } // While until the end of buffer
    
    return CAM_FILE_OK;
}

//! Running button status. The status of the button is changed by the on/off
//! switch on the board connected to the CONTROL_PIN
bool isRunning() {
    if(digitalRead(CONTROL_PIN) == HIGH) {
        return true;
    } else {
        return false;
    }
}


/* ----------------------------------------------------------------------
 * Setup and main application loop
   ---------------------------------------------------------------------- */

/**
 * Initialization function.
 */
void setup() {
    wiring_init();
    pinMode(CAM1_CS, OUTPUT);
    pinMode(CONTROL_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    // Camera not yet initializaed
    isCamStarted = false;
    // Test the LED on startup
    testFlash();
    // Initialize the GPS
    GPS.setUARTPort(GPS_UART);
    if(GPS.openUART() != UART_OK) {
        cout << GPS_UART_ERROR << endl;
    }
}

//! Convert a string to integer with validity check
int argToInt(string arg) {
    int x;

    try {
    size_t pos;

    x = stoi(arg, &pos);

    if (pos < arg.size()) {
        cerr << "Trailing characters after number: " << arg << '\n';
        return -1;
        }
    } catch (invalid_argument const &ex) {
        cerr << "Invalid number: " << arg << '\n';
        return -1;
    } catch (out_of_range const &ex) {
        cerr << "Number out of range: " << arg << '\n';
        return -1;
    }
    return x;
}

void imageCaptureAndProcess() {
    digitalWrite(LED_PIN, true);
    captureImage();
    lastSavedImage = createImageFileName();
    saveImage(lastSavedImage);
    writeLog(LOG_IMAGE_SAVED, lastSavedImage);
    imgProcessor.loadDefaultImage(lastSavedImage);
    eq = imgProcessor.correctExposure(&lightCorrector);
    writeLog(LOG_IMAGE_PROCESS);
    digitalWrite(LED_PIN, false);
}

/**
 * Main application.
 * 
 * Usage: firstfly <interval>
 * 
 * @note The capture delay (sec) of the series of images
 * do not consider the time needed to capture and process an image.
 */
int main(int argc, char *argv[]) {
    bool exiting = false; ///< True on exit command
    // Number of seconds between the capture of two images
    int capInterval = DEFAULT_CAPTURE_INTERVAL;

    // Check for the parameter
    if(argc == 2) {
        capInterval = argToInt(argv[1]);
    }

    // Initialization and setup
    setup(); 
    // Show the welcome message
    help();

    // Start the camera
    if(startForCapture() == CAM_INIT_ERROR) {
        // Camera not started
        return 0;
    }
    // Camera initialization complete
    writeLog(LOG_CAMERA_STARTED);
    writeLog(LOG_LIGHT_INDEX + to_string(lightCorrector.lightingIndex));
    writeLog(LOG_LIGHT_PERC + to_string(lightCorrector.lightingPerc));
    writeLog(LOG_LIGHT_LOOP + to_string(lightCorrector.maxExposureAdjust));

    // Set the camera resolution
    Cam5642.OV5642_set_JPEG_size(OV5642_1600x1200);
    writeLog(LOG_CAMERA_SETRES);


    // Wait for the switch enabled
    while(!isRunning()) {
        testFlash();
        delay(1000);
    }
    
    // Wait for capture starting to set the dron in position
    preFlight();
    
    // Image capture and process
    // Acquire at least on image when the program is started, regardless of the
    // loop duration 
    imageCaptureAndProcess(); 
    
    // Capture images until the switch is enabled
    while(isRunning()) {
        delay(capInterval * 1000);
        imageCaptureAndProcess();
    }

    digitalWrite(LED_PIN, false);
    return 0;
}
