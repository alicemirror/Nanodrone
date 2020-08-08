/**
	\file globals.h
	\brief Global definitions for terminal interface
	
*/

//! The number of available commands
#define NUM_COMMANDS 13
//! The commands list sown on help
std::string helpCommands[] = { 
	"<l> Capture low-res image (320x240)",
	"<m> Capture med-res image (640x480)",
	"<h> Capture high-res image (1600x1200)",
	"<f> Capture full-res image (2592x1944)",
	"<w> Enable/Disable saving images with separate names",
    "<g> Enable/disable (temporary) the logging",
	"<c> Close image window",
	"<i> Light index reference",
	"<p> Light percentage reference",
	"<r> Max equalization retries",
	"<n> Add one note to the log (single line)",
	"<x> Exit",
	"<?> This list of commands"
	};

// ----------------------------- Commands
#define CAP_NORES 'n'   // Resolution is not set when the program starts
#define CAP_LOWRES 'l'
#define CAP_MEDRES 'm'
#define CAP_HIRES 'h'
#define CAP_FULLRES 'f'
#define CAP_CLOSE 'c'
#define CAP_LIGHT_INDEX 'i'
#define CAP_LIGHT_PERC 'p'
#define CAP_LIGHT_LOOP 'r'
#define CAP_WRITE_IMAGES 'w'
#define CAP_ADD_NOTE 'n'
#define CAP_LOGGING 'g'
#define EXIT 'x'
#define HELP '?'

// ----------------------------- Messages
#define WRONG_COMMAND "Command not recognized. Type '?' for help"
#define CAMERA_STARTING "Camera initialization... wait"
#define SET_LOWRES "Set low resolution"
#define SET_MEDRES "Set medium resolution"
#define SET_HIRES "Set high resolution"
#define SET_FULLRES "Set maximum resolution"
#define EQ_RETURN "Equalization returns "
#define OVERWRITE_IMAGES "Images are overwritten"
#define SAVE_IMAGES "Images are saved separately"
#define LOG_ACTIVE "Log file is active"
#define LOG_DISABLED "Log file is disabled"

// ----------------------------- Console messages
#define CON_CMD "?>"
#define CON_LOG_NOTE "Log note "
#define CON_LIGHT_INDEX "Light index ?>"
#define CON_LIGHT_PERC "Light percentage ?>"
#define CON_RETRIES "Equalization retries ?>"
#define CON_DASHES "---------------------------------"

// ----------------------------- File & Log
#define LOG_NOTE_MAX 40             ///< The max lenght of a log note
#define TEST_FILE "testlens"        ///< Camera capture image file name
#define LOG_FILE "testlens_log"     ///< Session log file name
#define CSV_SEPARATOR ";"           ///< Log file fields separator
#define LOG_HEADER "Timestamp;Event Description;Image\n"
#define LOG_CREATED "Log created"
#define LOG_EQUALIZE1 "Captured image (" 
#define LOG_EQUALIZE2 ") equalized in "
#define LOG_EQUALIZE3 " loops"
#define LOG_OVERWRITE_IMAGES "Set single test image"
#define LOG_MULTIPLE_IMAGES "Set saving timestamped images"
#define LOG_LIGHT_INDEX "Equalization lighting index: "
#define LOG_LIGHT_PERC "Equalization lighting percentage: "
#define LOG_LIGHT_LOOP "Equalization max retries: "
#define LOG_CAMERA_STARTED "OV5642 camera started"
#define LOG_CAMERA_LORES "Set camera resolution to 3620x240"
#define LOG_CAMERA_MEDRES "Set cmaera resolution to 640x480"
#define LOG_CAMERA_HIRES "Set camera resolution to 1600x1200"
#define LOG_CAMERA_FULLRES "Set caemra resolution to 2592x1944"
#define LOG_CAMERA_IMAGE_SAVED "Saved image to file"
#define LOG_LOGGING_ENABLED "Event logging enabled"
#define LOG_LOGGING_DISABLED "Event logging disabled"


