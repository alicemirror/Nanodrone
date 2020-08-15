/**
@file firstfly.h

@brief Automatic shooting with data retrieval. This application should be used
to test the flying unit and LoRa connection.

@author Enrico Miglino <balearicdynamics@gmail.com>
@version 1.0
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
#include "imageprocessor.h"
#include "serialgps.h"

// ----------------------------- Application version, subversion and build number
#define testlens_VERSION_MAJOR 1
#define testlens_VERSION_MINOR 0
#define testlens_VERSION_BUILD 15

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
//! the events duration. Uses BCM 23 (physical pin 16, wiring pin 4)
#define DEBUG_PIN 4
//! Led indicator pin. Uses BCM 23 (physical pin 24, wiring pin 5)
#define LED_PIN 5

//! Number of seconds between the capture of two images
#define DEFAULT_CAPTURE_INTERVAL 5

//! Undef to avoid the debug messages
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
//! Light correction parameters for the image equalization after the capture
//! The default values are an average that has almost no impact on the original
//! image, to avoid a crash when the image is acquired without the user has set
//! the parameters before.
LightIndexes lightCorrector = { 0.7, 3, 3 };
//! The name of the last captured and saved ima
string lastSavedImage = "";
//! Serial GPS manager
SerialGPS GPS;
//! Number of times the image equalization has been applied
int eq;

// ----------------------------- Messages
#define CAMERA_STARTING "Initializing camera"
#define CAMERA_ERROR_ON_START "Error during camera initialization"
#define CAMERA_STARTED "Camera initialization complete"
#define CON_DASHES "---------------------------------"

// ----------------------------- File & Log
#define TEST_FILE "firstfly"        ///< Camera capture image file name
#define REPORT_FOLDER "./data/"
#define LOG_CREATED "Log created"
#define LOG_IMAGE_PROCESS "Image process completed" 
#define LOG_LIGHT_INDEX "Equalization lighting index: "
#define LOG_LIGHT_PERC "Equalization lighting percentage: "
#define LOG_LIGHT_LOOP "Equalization max retries: "
#define LOG_CAMERA_STARTED "OV5642 camera started"
#define LOG_CAMERA_SETRES "Set camera resolution to 1600x1200"
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
string createMatFileName();
void writeLog(string message);
void writeLog(string message, string image);
string getLogTimestamp();
void testFlash();
int argToInt(string arg);
void imageCaptureAndProcess();

