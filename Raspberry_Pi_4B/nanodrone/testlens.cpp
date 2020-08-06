/**
@file testlens.cpp

@brief Test the lens on the 5642 camera chip module. Test program for setting
the lens focus and check the image kind

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
//! Camera driver instance
ArduCAM Cam5642(OV5642, CAM1_CS);

//! Print program version number
void pVersion() {
    cout << "TestLens Version" << testlens_VERSION_MAJOR <<
            "." << testlens_VERSION_MINOR << " build " <<
            testlens_VERSION_BUILD << endl;
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
        cout << PROGRAM_STARTING << endl;
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
    
    // Initialize the camera driver
    int camInitStatus;
    outCamError(camInitStatus = initCamera());
    if(camInitStatus != CAM_INIT_OK) {
        exit(EXIT_FAILURE);
    }
}

/**
 * Main application. Shows the menu and ignore the parameters via command line
 */
int main(int argc, char *argv[]) {
    uint8_t temp = 0, temp_last = 0;

    // Initialize the camera
    setup(); 
    // Show the commands
    help();

    // Set the resolution require 82 ms  
    if (strcmp(argv[3], "320x240")  == 0) Cam5642.OV5642_set_JPEG_size(OV5642_320x240);
    else if (strcmp(argv[3], "640x480")  == 0) Cam5642.OV5642_set_JPEG_size(OV5642_640x480);
    else if (strcmp(argv[3], "1280x960")  == 0) Cam5642.OV5642_set_JPEG_size(OV5642_1280x960);
    else if (strcmp(argv[3], "1600x1200")  == 0) Cam5642.OV5642_set_JPEG_size(OV5642_1600x1200);
    else if (strcmp(argv[3], "2048x1536")  == 0) Cam5642.OV5642_set_JPEG_size(OV5642_2048x1536);
    else if (strcmp(argv[3], "2592x1944") == 0) Cam5642.OV5642_set_JPEG_size(OV5642_2592x1944);
    else {
    printf("Unknown resolution %s\n", argv[3]);
    exit(EXIT_FAILURE);
    }

    sleep(1); // Let auto exposure do it's thing after changing image settings
#ifdef _DEBUG
    printf("Changed resolution1 to %s\n", argv[3]); 
#endif
    Cam5642.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH   	  
     // Flush the FIFO
    Cam5642.flush_fifo();    
    // Clear the capture done flag
    Cam5642.clear_fifo_flag();
    // Capture an image
#ifdef _DEBUG
    printf("Start capture\n");  
#endif
    Cam5642.start_capture();
    while (!(Cam5642.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)){}
#ifdef _DEBUG
    printf("CAM Capture Done, prepare for saving on file\n");
#endif
              
    // Save the image on file
    FILE *fp1 = fopen(argv[2], "w+");   
    if (!fp1) {
	printf("Error: could not open %s\n", argv[2]);
	exit(EXIT_FAILURE);
    }
    size_t length = Cam5642.read_fifo_length();
    if (length >= MAX_FIFO_SIZE){
	  printf("Cameera buffer over size.");
	  exit(EXIT_FAILURE);
      } else if (length == 0 ){
	  printf("Camera buffer is 0.");
	  exit(EXIT_FAILURE);
      } 

#ifdef _DEBUG
    printf("Start reading and saving on file (BUF_SIZE = %d)\n", BUF_SIZE);
#endif
    debugOsc(true);
    int32_t i = 0;
    Cam5642.CS_LOW();  //Set CS low       
    Cam5642.set_fifo_burst();

// ******************** DEBUG ONLY ********************
// Note that to calculate the effective transfer time of the data from 
// the camera buffer to the local buffer, in debug mode the file
// writing is disabled.
// Undef below the _DEBUG_BUFFER_TIMING to enable the file writing!!!
#undef _DEBUG_BUFFER_TIMING
// ******************** DEBUG ONLY ********************

    while( length-- )
    {
	temp_last = temp;
	temp =  Cam5642.transfer(0x00);
	// Read JPEG data from FIFO and if find the end break while
	if ( (temp == 0xD9) && (temp_last == 0xFF) ) 
	{
	    buf[i++] = temp;  //save the last  0XD9     
	    //Write the remain bytes in the buffer
	    Cam5642.CS_HIGH();
#ifndef _DEBUG_BUFFER_TIMING
	    fwrite(buf, i, 1, fp1);    
	    //Close the file
	    fclose(fp1); 
#endif
	    debugOsc(false);
#ifdef _DEBUG
	    printf("IMG save OK !\n"); 
#endif
	    is_header = false;
	    i = 0;
	}
	if (is_header == true)
	{ 
	    if (i < BUF_SIZE) {
		buf[i++] = temp;
	    } else {
		// Write BUF_SIZE bytes image data to file
		Cam5642.CS_HIGH();
#ifndef _DEBUG_BUFFER_TIMING
		fwrite(buf, BUF_SIZE, 1, fp1);
#endif
		i = 0;
		buf[i++] = temp;
		Cam5642.CS_LOW();
		Cam5642.set_fifo_burst();
	    }        
	}
	else if ((temp == 0xD8) & (temp_last == 0xFF))
	{
	    is_header = true;
	    buf[i++] = temp_last;
	    buf[i++] = temp;   
	} 
    }
    exit(EXIT_SUCCESS);
}
