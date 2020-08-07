/**
	\file globals.h
	\brief Global definitions
	
*/

//! The number of available commands
#define NUM_COMMANDS 6
//! The commands list sown on help
std::string helpCommands[] = { 
	"<l> Capture low-res image (320x240)",
	"<m> Capture med-res image (1600x1200)",
	"<h> Capture high-res image (5 Mp)",
	"<s> Show last image",
	"<x> Exit",
	"<?> This list of commands"
	};

// ----------------------------- Commands
#define CAP_NORES 'n'   // Resolution is not set when the program starts
#define CAP_LOWRES 'l'
#define CAP_MEDRES 'm'
#define CAP_HIRES 'h'
#define EXIT 'x'
#define HELP '?'

// ----------------------------- Messages
#define WRONG_COMMAND "Command not recognized. Type '?' for help"
#define PROGRAM_STARTING "Starting... wait"
#define SET_LOWRES "Set low resolution"
#define SET_MEDRES "Set medium resolution"
#define SET_HIRES "Set high resolution"

// ----------------------------- Other
#define TEST_FILE "testlens.jpg"
#define VIEWER_TIT "Last Captured Image"
