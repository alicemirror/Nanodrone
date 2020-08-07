/**
@file testlens.cpp

@brief Test the lens on the 5642 camera chip module. Test program for setting
the lens focus and check the image kind.

@author Enrico Miglino <balearicdynamics@gmail.com>
@version 0.1
@date Augut 2020
*/

#include <iostream>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include "arducam_arch_raspberrypi.h"
#include "cam5642_errors.h"
#include "globals.h"
#include "version.h"
#include "imageprocessor.h"

using namespace std;

#define OV5642_CHIPID_HIGH 0x300a
#define OV5642_CHIPID_LOW 0x300b

//! Local buffer to store the captured image before saving it on file
//! With a 512KB buffer a full-resolution image can be stored in a single
//! step on memory. 
//! @warning For performances comparison only, us the original BUF_SIZE 
//! of 4096 (4K) instead of 0x80000 (512K)
#define BUF_SIZE 0x80000

//! The physical connection of the SPI CS pin is the BCM 17 on the 
//! Raspberry Pi GPIO connector (pin 11). The id shoiuld be 0 for 
//! compatibility with the Wiring Pi component of the library.
#define CAM1_CS 0

//! Debug pin to generate a pulse every step and check with the oscilloscope
//! the events duration. Uses BCM 27 (physica pin 13, wiring pin 2)
#define DEBUG_PIN 15
//! Undef to avoid the debug messages and the pin debug pulse
#define _DEBUG

#define VSYNC_LEVEL_MASK 0x02  // 0 = High active - 1 = Low active
//! Image data acquisitino buffer
uint8_t buf[BUF_SIZE];
//! Image header flag
bool is_header = false;
//! Flag indicating is the camera has been initialized
bool isCamStarted = false;
//! Camera driver instance
ArduCAM Cam5642(OV5642, CAM1_CS);
//! Image processor class instance
ImageProcessor imgProcessor;

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

//! Output a camera status message
void outCamError(int code) {
    cout << msgCam[code] << endl;
}

void outMessage(string msg) {
    cout << msg << endl;
}

/**
	Shows the applicaiton menu
*/
void help() {
    cls();
	cout << "---------------------------------" << endl;
    pVersion();
	cout << "---------------------------------" << endl;
	for(int j = 0; j < NUM_COMMANDS; j++) {
		cout << helpCommands[j] << endl;
	}
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
int saveImage() {
    uint8_t temp = 0, temp_last = 0;

    FILE *fp1 = fopen(TEST_FILE, "w+");   
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
}

/**
 * Main application. Shows the menu and ignore the parameters via command line
 */
int main(int argc, char *argv[]) {

    // Initialize the camera
    setup(); 
    // Show the commands
    help();

	// Application loop
    bool exiting = false; ///< True on exit command
    //! Force the resolution set for the first capture
    char lastRes = CAP_NORES;
	while(!exiting) {
        //! Command from the console
        char cmd;
		
		cout << "?>";
		cin >> cmd;
		
		switch(cmd) {
		case CAP_LOWRES:
            // If it is the first capture, initialize the camera
            if(!isCamStarted) {
                startForCapture();
                isCamStarted = true;
            }
            // Set the resolution only if it is changed
            // else onlhy starts the capture
            if(lastRes != CAP_LOWRES) {
                lastRes = CAP_LOWRES;
                Cam5642.OV5642_set_JPEG_size(OV5642_320x240);
                outMessage(SET_LOWRES);
            }
            captureImage();
            outCamError(saveImage());
            imgProcessor.loadDefaultImage(TEST_FILE);
            imgProcessor.showImage();
            break;
		case CAP_MEDRES:
            // If it is the first capture, initialize the camera
            if(!isCamStarted) {
                startForCapture();
                isCamStarted = true;
            }
            // Set the resolution only if it is changed
            // else onlhy starts the capture
            if(lastRes != CAP_MEDRES) {
                lastRes = CAP_MEDRES;
                Cam5642.OV5642_set_JPEG_size(OV5642_640x480);
                outMessage(SET_MEDRES);
            }
            captureImage();
            outCamError(saveImage());
            imgProcessor.loadDefaultImage(TEST_FILE);
            imgProcessor.showImage();
            break;
		case CAP_HIRES:
            // If it is the first capture, initialize the camera
            if(!isCamStarted) {
                startForCapture();
                isCamStarted = true;
            }
            // Set the resolution only if it is changed
            // else onlhy starts the capture
            if(lastRes != CAP_HIRES) {
                lastRes = CAP_HIRES;
                Cam5642.OV5642_set_JPEG_size(OV5642_1600x1200);
                outMessage(SET_HIRES);
            }
            captureImage();
            outCamError(saveImage());
            imgProcessor.loadDefaultImage(TEST_FILE);
            imgProcessor.showImage();
            break;
		case CAP_LAST:
            imgProcessor.loadDefaultImage(TEST_FILE);
            imgProcessor.showImage();
            break;
		case CAP_CLOSE:
            imgProcessor.closeImage();
            break;
		case EXIT:
            cls();
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
