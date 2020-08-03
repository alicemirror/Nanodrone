/**
@file arducam_ov5642_capture.cpp

@brief Modified test program for Raspberry Pi 4B. This version will work
only on the 5642 camera chip module.

@note As this originates from the porting of the original Arducam
Arduino library (to which I have added optimizations and improvements)
for the Raspberry Pi 4B the appicaiton lifecycle follows the classical
loop design of the Arduino Sketch with a setup() function and the
lopo() function, as well as the pin numbering that follows the 
wiringPi library naming instead of the (more comfortable) BCM or GPIO
numbers of the Raspberry Pi.

Original library porting by UCTronics (Arducam)

@author Enrico Miglino <balearicdynamics@gmail.com>
@version 0.1
@date Augut 2020

@todo Set the hardcoded hex values to symbolic self-explaining precompiler
definitions
*/
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include "arducam_arch_raspberrypi.h"

#define OV5642_CHIPID_HIGH 0x300a
#define OV5642_CHIPID_LOW 0x300b

//! Version and build number
#define _VERSION_BUILD "0.1 build 14"

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
#undef _DEBUG

#define VSYNC_LEVEL_MASK 0x02  // 0 = High active - 1 = Low active
//! Image data acquisitino buffer
uint8_t buf[BUF_SIZE];
//! Image header flag
bool is_header = false;

//! Create the camera library instance
ArduCAM myCAM(OV5642,CAM1_CS);

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
 * Initialization function.
 */
void setup() {
    uint8_t vid,pid;
    uint8_t temp;
    wiring_init();
    pinMode(CAM1_CS, OUTPUT);
#ifdef _DEBUG
    // Set the debug pin and initialize to low
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);
#endif
    
    // Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);

    if(temp != 0x55) {
#ifdef _DEBUG
        printf("SPI interface error!\n");
#endif
        exit(EXIT_FAILURE);
    }  

    // Change MCU mode
    myCAM.write_reg(ARDUCHIP_MODE, 0x00); 
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);

#ifdef _DEBUG
    if((vid != 0x56) || (pid != 0x42))
      printf("Can't find OV5642 module!");
    else
     printf("OV5642 detected.\n");
#endif
}

int main(int argc, char *argv[])
{
    uint8_t temp = 0, temp_last = 0;

    printf("OV5642 Test version %s\n\n", _VERSION_BUILD);

    if (argc < 4)
    {
        printf("Usage: %s [-s <resolution>] | [-c <filename]", argv[0]);
        printf(" -s <resolution> Set resolution, valid resolutions are:\n");
        printf("                   320x240\n");
        printf("                   640x480\n");
        printf("                   1280x960\n");
        printf("                   1600x1200\n");
        printf("                   2048x1536\n");
        printf("                   2592x1944\n");
        printf(" -c <filename>   Capture image\n");
        exit(EXIT_SUCCESS);
    }

    setup(); 
    // Setting image capture mode require 0.568 ms
    myCAM.set_format(JPEG);
    // Initialization take a long time, 15.84 sec.
    myCAM.InitCAM();
    // Set the resolution require 82 ms  
    if (strcmp(argv[3], "320x240")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_320x240);
    else if (strcmp(argv[3], "640x480")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_640x480);
    else if (strcmp(argv[3], "1280x960")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_1280x960);
    else if (strcmp(argv[3], "1600x1200")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);
    else if (strcmp(argv[3], "2048x1536")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);
    else if (strcmp(argv[3], "2592x1944") == 0) myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);
    else {
    printf("Unknown resolution %s\n", argv[3]);
    exit(EXIT_FAILURE);
    }

    sleep(1); // Let auto exposure do it's thing after changing image settings
#ifdef _DEBUG
    printf("Changed resolution1 to %s\n", argv[3]); 
#endif
    myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH   	  
     // Flush the FIFO
    myCAM.flush_fifo();    
    // Clear the capture done flag
    myCAM.clear_fifo_flag();
    // Capture an image
#ifdef _DEBUG
    printf("Start capture\n");  
#endif
    myCAM.start_capture();
    while (!(myCAM.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)){}
#ifdef _DEBUG
    printf("CAM Capture Done, prepare for saving on file\n");
#endif
              
    // Save the image on file
    FILE *fp1 = fopen(argv[2], "w+");   
    if (!fp1) {
	printf("Error: could not open %s\n", argv[2]);
	exit(EXIT_FAILURE);
    }
    size_t length = myCAM.read_fifo_length();
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
    myCAM.CS_LOW();  //Set CS low       
    myCAM.set_fifo_burst();

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
	temp =  myCAM.transfer(0x00);
	// Read JPEG data from FIFO and if find the end break while
	if ( (temp == 0xD9) && (temp_last == 0xFF) ) 
	{
	    buf[i++] = temp;  //save the last  0XD9     
	    //Write the remain bytes in the buffer
	    myCAM.CS_HIGH();
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
		myCAM.CS_HIGH();
#ifndef _DEBUG_BUFFER_TIMING
		fwrite(buf, BUF_SIZE, 1, fp1);
#endif
		i = 0;
		buf[i++] = temp;
		myCAM.CS_LOW();
		myCAM.set_fifo_burst();
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
