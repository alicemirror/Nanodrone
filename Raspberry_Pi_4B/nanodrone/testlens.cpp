/**
@file testlens.cpp

@brief Test the lens on the 5642 camera chip module. Test program for setting
the lens focus and check the image kind, testing the algorithms and profiling
the application.

@author Enrico Miglino <balearicdynamics@gmail.com>
@version 0.1
@date Augut 2020
*/

#include "testlens.h"

using namespace std;


/* ----------------------------------------------------------------------
 * Application functions and textual interface
   ---------------------------------------------------------------------- */

//! Print program version number
void pVersion() {
    cout << "Test Lens " << testlens_VERSION_MAJOR <<
            "." << testlens_VERSION_MINOR << " build " <<
            testlens_VERSION_BUILD << endl <<
            "Image Processor " << PROCESSOR_MAJOR << "." <<
            PROCESSOR_MINOR << " build " << PROCESSOR_BUILD << endl;
}

/**
 * When _DEBUG is defined generate a signal on the DEBUG_PIN high or low
 * according to the state flag.
 * 
 * To calculate the timing of a certain event, generate a debug state high
 * when the event start and a state low when the event ends. Set the 
 * oscilloscope to trigger the pin status change and precisely measure the
 * duration of the event.
 * 
 * @param state The level of the debug pin to be set
 */
void debugOsc(bool state) {
#ifdef _DEBUG
    digitalWrite(DEBUG_PIN, state);
#endif  
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
        return CAM_SPI_ERROR;
    }

    // Change MCU mode
    Cam5642.write_reg(ARDUCHIP_MODE, 0x00); 
    Cam5642.wrSensorReg16_8(0xff, 0x01);
    Cam5642.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    Cam5642.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);

    // Check if the camera is detected
    if((vid != 0x56) || (pid != 0x42)) {
        return CAM_NOT_FOUND;
    } else {
        // Setting image capture mode require 0.568 ms
        Cam5642.set_format(JPEG);
        // Initialization take a long time, 15.84 sec.
        Cam5642.InitCAM();
        return CAM_INIT_OK;
    }
}

//! Clear screen
void cls() {
    cout << "\033[2J\033[1;1H";
}

//! Create the image file name. Depending on the appication options the image is
//! a default name of a timestamped unique name.
string createImageFileName() {
    if(saveImages) {
        return string(TEST_FILE) + string("_") + getDateSuffix() + string(".jpg");
    } else {
        return string(TEST_FILE) + string(".jpg");
    }
}

//! Create the log session file name
string createLogFileName() {    
    // Create the session log file name
    return string(LOG_FILE) + string("_(") + to_string(testlens_VERSION_MAJOR) +
            string(".") + to_string(testlens_VERSION_MINOR) + 
            string(".") + to_string(testlens_VERSION_BUILD) +
            string("[") + to_string(PROCESSOR_MAJOR) + 
            string(".") + to_string(PROCESSOR_MINOR) + 
            string(".") + to_string(PROCESSOR_BUILD) + string("])_") +
            string(getDateSuffix()) + string(".csv");
}

//! Initiate a new log session (when the program starts) and write che
//! columns header in the csv format.
void openLogFile() {
    string fn = createLogFileName();
#ifdef _DEBUG
cout << "log file name : " << fn << endl;
#endif
    const char* fnp = fn.c_str();

    logFHandler = fopen(fnp, "w+");
    // Add the CSV log header
    fprintf(logFHandler,LOG_HEADER);
}

//! Write a new record to the log. The image file name is an optional parameter
//! to be used only when the event is related to the image
void writeLog(string message, string image) {
    string record = getLogTimestamp() + string(CSV_SEPARATOR) + 
                    message + string(CSV_SEPARATOR) + image + string("\n");
    const char* recordp = record.c_str();
    fprintf(logFHandler, recordp);
}

//! Write a new record to the log. The image file name is an optional parameter
//! to be used only when the event is related to the image
void writeLog(string message) {
    string record = getLogTimestamp() + string(CSV_SEPARATOR) + 
                    message + string(CSV_SEPARATOR) + string("--\n");
    const char* recordp = record.c_str();
    fprintf(logFHandler, recordp);
}

//! Close the log header
void closeLogFile() {
    fclose(logFHandler);
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
    cout << msgCam[code] << endl;
}

void outMessage(string msg) {
    cout << msg << endl;
}

void showEqParams(LightIndexes* lc) {
    cout << "Eq param: light index " << lc->lightingIndex <<
            " light % " << lc->lightingPerc << 
            " max retries " << lc->maxExposureAdjust << endl;
}

/**
	Shows the applicaiton menu
*/
void help() {
    cls();
	cout << "---------------------------------" << endl;
    pVersion();
    if(saveImages)
        cout << SAVE_IMAGES << endl;
    else
        cout << OVERWRITE_IMAGES << endl;
	cout << "---------------------------------" << endl;
	for(int j = 0; j < NUM_COMMANDS; j++) {
		cout << helpCommands[j] << endl;
	}
	cout << "---------------------------------" << endl << endl;
    showEqParams(&lightCorrector);
	cout << "---------------------------------" << endl << endl;
}

/* ----------------------------------------------------------------------
 * Image acquisition and camera driver settings
   ---------------------------------------------------------------------- */

/**
 * Initialize the camera driver.
 * 
 * As the camera initialization needs about 15 seconds, it is called only when
 * the first acquisition is requested.
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
            debugOsc(false);
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

/* ----------------------------------------------------------------------
 * Setup and main application loop
   ---------------------------------------------------------------------- */

/**
 * Initialization function.
 */
void setup() {
    wiring_init();
    pinMode(CAM1_CS, OUTPUT);
#ifdef _DEBUG
    // Set the debug pin and initialize to low
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);
#endif
    // Camera not yet initializaed
    isCamStarted = false;
    // Images are overwritten on startup
    saveImages = false;
    // Create the session log file
    openLogFile();
    writeLog(LOG_CREATED);
}

/**
 * Main application. Shows the menu and ignore the parameters via command line
 */
int main(int argc, char *argv[]) {
    int eq; ///< Return value from the equalization method
    bool exiting = false; ///< True on exit command
    //! Force the resolution set for the first capture
    char lastRes = CAP_NORES;

    // Initialize the camera
    setup(); 
    // Show the commands
    help();

	// Application loop
    
	while(!exiting) {
        //! Command from the console
        char cmd;
        
		cout << "?>";
		cin >> cmd;
		
		switch(cmd) {
        case CAP_ADD_NOTE: {
            string note;
            cout << "Log note ";
            cin.ignore();
            getline(cin, note);
            writeLog(note);
            }
            break;
        case CAP_WRITE_IMAGES:
            if(saveImages) {
                saveImages = false;
                outMessage(OVERWRITE_IMAGES);
                writeLog(LOG_OVERWRITE_IMAGES);
            }
            else {
                saveImages = true;
                outMessage(SAVE_IMAGES);
                writeLog(LOG_MULTIPLE_IMAGES);
            }
            break;
        case CAP_LIGHT_INDEX: {
            cout << "Light index ?>";
            cin >> lightCorrector.lightingIndex;
            writeLog(LOG_LIGHT_INDEX + to_string(lightCorrector.lightingIndex));
            }
            break;
        case CAP_LIGHT_PERC: {
            cout << "Light percentage ?>";
            cin >> lightCorrector.lightingPerc;
            writeLog(LOG_LIGHT_PERC + to_string(lightCorrector.lightingPerc));
            }   
            break;
        case CAP_LIGHT_LOOP: {
            cout << "Equaization retries ?>";
            cin >> lightCorrector.maxExposureAdjust;
            writeLog(LOG_LIGHT_LOOP + to_string(lightCorrector.maxExposureAdjust));
            }
            break;
		case CAP_LOWRES:
            // If it is the first capture, initialize the camera
            if(!isCamStarted) {
                startForCapture();
                writeLog(LOG_CAMERA_STARTED);
                writeLog(LOG_LIGHT_INDEX + to_string(lightCorrector.lightingIndex));
                writeLog(LOG_LIGHT_PERC + to_string(lightCorrector.lightingPerc));
                writeLog(LOG_LIGHT_LOOP + to_string(lightCorrector.maxExposureAdjust));
                isCamStarted = true;
            }
            // Set the resolution only if it is changed
            // else onlhy starts the capture
            if(lastRes != CAP_LOWRES) {
                lastRes = CAP_LOWRES;
                Cam5642.OV5642_set_JPEG_size(OV5642_320x240);
                outMessage(SET_LOWRES);
                writeLog(LOG_CAMERA_LORES);
            }
            captureImage();
            lastSavedImage = createImageFileName();
            outCamError(saveImage(lastSavedImage));
            writeLog(LOG_CAMERA_IMAGE_SAVED, lastSavedImage);
            imgProcessor.loadDefaultImage(lastSavedImage);
            eq = imgProcessor.correctExposure(&lightCorrector);
            writeLog(string(LOG_EQUALIZE1) + string("320x240") +
                    string(LOG_EQUALIZE2) + to_string(eq) + 
                    string(LOG_EQUALIZE3), lastSavedImage);
            showEqParams(&lightCorrector);
            imgProcessor.showImage();
            break;
		case CAP_MEDRES:
            // If it is the first capture, initialize the camera
            if(!isCamStarted) {
                startForCapture();
                writeLog(LOG_CAMERA_STARTED);
                writeLog(LOG_LIGHT_INDEX + to_string(lightCorrector.lightingIndex));
                writeLog(LOG_LIGHT_PERC + to_string(lightCorrector.lightingPerc));
                writeLog(LOG_LIGHT_LOOP + to_string(lightCorrector.maxExposureAdjust));
                isCamStarted = true;
            }
            // Set the resolution only if it is changed
            // else onlhy starts the capture
            if(lastRes != CAP_MEDRES) {
                lastRes = CAP_MEDRES;
                Cam5642.OV5642_set_JPEG_size(OV5642_640x480);
                outMessage(SET_MEDRES);
                writeLog(LOG_CAMERA_MEDRES);
            }
            captureImage();
            lastSavedImage = createImageFileName();
            outCamError(saveImage(lastSavedImage));
            writeLog(LOG_CAMERA_IMAGE_SAVED, lastSavedImage);
            imgProcessor.loadDefaultImage(lastSavedImage);
            eq = imgProcessor.correctExposure(&lightCorrector);
            writeLog(string(LOG_EQUALIZE1) + string("640x480") +
                    string(LOG_EQUALIZE2) + to_string(eq) + 
                    string(LOG_EQUALIZE3), lastSavedImage);
            showEqParams(&lightCorrector);
            imgProcessor.showImage();
            break;
		case CAP_HIRES:
            // If it is the first capture, initialize the camera
            if(!isCamStarted) {
                startForCapture();
                writeLog(LOG_CAMERA_STARTED);
                writeLog(LOG_LIGHT_INDEX + to_string(lightCorrector.lightingIndex));
                writeLog(LOG_LIGHT_PERC + to_string(lightCorrector.lightingPerc));
                writeLog(LOG_LIGHT_LOOP + to_string(lightCorrector.maxExposureAdjust));
                isCamStarted = true;
            }
            // Set the resolution only if it is changed
            // else onlhy starts the capture
            if(lastRes != CAP_HIRES) {
                lastRes = CAP_HIRES;
                Cam5642.OV5642_set_JPEG_size(OV5642_1600x1200);
                outMessage(SET_HIRES);
                writeLog(LOG_CAMERA_HIRES);
            }
            captureImage();
            lastSavedImage = createImageFileName();
            outCamError(saveImage(lastSavedImage));
            writeLog(LOG_CAMERA_IMAGE_SAVED, lastSavedImage);
            imgProcessor.loadDefaultImage(lastSavedImage);
            eq = imgProcessor.correctExposure(&lightCorrector);
            writeLog(string(LOG_EQUALIZE1) + string("1600x1200") +
                    string(LOG_EQUALIZE2) + to_string(eq) + 
                    string(LOG_EQUALIZE3), lastSavedImage);
            showEqParams(&lightCorrector);
            imgProcessor.showImage();
            break;
        case CAP_FULLRES:
            // If it is the first capture, initialize the camera
            if(!isCamStarted) {
                startForCapture();
                writeLog(LOG_CAMERA_STARTED);
                writeLog(LOG_LIGHT_INDEX + to_string(lightCorrector.lightingIndex));
                writeLog(LOG_LIGHT_PERC + to_string(lightCorrector.lightingPerc));
                writeLog(LOG_LIGHT_LOOP + to_string(lightCorrector.maxExposureAdjust));
                isCamStarted = true;
            }
            // Set the resolution only if it is changed
            // else onlhy starts the capture
            if(lastRes != CAP_FULLRES) {
                lastRes = CAP_FULLRES;
                Cam5642.OV5642_set_JPEG_size(OV5642_2592x1944);
                writeLog(LOG_CAMERA_FULLRES);
                outMessage(SET_FULLRES);
            }
            captureImage();
            lastSavedImage = createImageFileName();
            outCamError(saveImage(lastSavedImage));
            writeLog(LOG_CAMERA_IMAGE_SAVED, lastSavedImage);
            imgProcessor.loadDefaultImage(lastSavedImage);
            eq = imgProcessor.correctExposure(&lightCorrector);
            writeLog(string(LOG_EQUALIZE1) + string("2592x1944") +
                    string(LOG_EQUALIZE2) + to_string(eq) + 
                    string(LOG_EQUALIZE3), lastSavedImage);
            showEqParams(&lightCorrector);
            imgProcessor.showImage();
            break;
		case CAP_CLOSE:
            imgProcessor.closeImage();
            break;
		case EXIT:
            cls();
            closeLogFile();
            exiting = true;
			break;
		case HELP:
			help();
			break;
		default:
			cout << WRONG_COMMAND << endl;
		}
	}

    return 0;
}
