/**
 * @file cam5642_errors.h
 * @brief Camera driver error codes
 * 
 */

#define CAM_INIT_OK 0       ///< Initialization complete
#define CAM_SPI_ERROR 1     ///< SPI protocol communication error
#define CAM_NOT_FOUND 2     ///< Camera not detected

//! Camera return codes messages
std::string msgCam[] = {
    "Initialization completed",
    "SPI protocol communication error",
    "Camera sensor OV5642 not found"
    };
