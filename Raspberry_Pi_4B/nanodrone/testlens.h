/**
@file testlens.h

@brief Test the lens on the 5642 camera chip module. Test program for setting
the lens focus and check the image kind, testing the algorithms and profiling
the application.

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

// ----------------------------- Camera driver parameters and global variables
//! Camera driver high memory address
#define OV5642_CHIPID_HIGH 0x300a
//! Camera driver low memory address
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

//! Undef to avoid the debug messages
#define _DEBUG

#define VSYNC_LEVEL_MASK 0x02  // 0 = High active - 1 = Low active
//! Image data acquisitino buffer
uint8_t buf[BUF_SIZE];
//! Image header flag
bool is_header = false;
//! Flag indicating is the camera has been initialized
bool isCamStarted = false;
//! When this flag is set, the image files are saved with different names to keep
//! track of the modifications
bool saveImages = false;
//! Camera driver instance
ArduCAM Cam5642(OV5642, CAM1_CS);
//! Image processor class instance
ImageProcessor imgProcessor;
//! Light correction parameters for the image equalization after the capture
//! The default values are an average that has almost no impact on the original
//! image, to avoid a crash when the image is acquired without the user has set
//! the parameters before.
LightIndexes lightCorrector = { 0.7, 3, 3 };
//! The name of the last captured and saved ima
string lastSavedImage = "";
//! The log file name
string logFileName = "";
//! Log file handler
FILE * logFHandler;

// ----------------------------- Function prototypes
void pVersion();
void debugOsc(bool state);
int initCamera();
void cls();
void outCamError(int code);
void outMessage(string msg);
void showEqParams(LightIndexes* lc);
void help();
int startForCapture();
void captureImage();
int saveImage(string fn);
void setup();
int main(int argc, char *argv[]);
string getDateSuffix();
string createImageFileName();
string createLogFileName();
void openLogFile();
void closeLogFile();
void writeLog(string message);
void writeLog(string message, string image);
string getLogTimestamp();

