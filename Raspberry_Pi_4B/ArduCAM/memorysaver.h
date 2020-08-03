/**
 * @file memorysaver.h
 * @brief Version cleaned and modified removing all the unwanted references
 * 
 * This version of the memory saver configuration is tailored for the use
 * with Raspberry Pi 4B and the OV5642 IC
 */
#ifndef _MEMORYSAVER_
#define _MEMORYSAVER_

#define RASPBERRY_PI

//Step 1: select the hardware platform, only one at a time
#define OV5642_MINI_5MP


//Step 2: Select one of the camera module, only one at a time
#if (defined(ARDUCAM_SHIELD_REVC) || defined(ARDUCAM_SHIELD_V2))
	#define OV5642_CAM
#endif 

#endif	//_MEMORYSAVER_
