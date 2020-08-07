/**
 * @file cam5642_errors.h
 * @brief Camera driver error codes
 * 
 */

#define CAM_INIT_OK 0       ///< Initialization complete
#define CAM_SPI_ERROR 1     ///< SPI protocol communication error
#define CAM_NOT_FOUND 2     ///< Camera not detected
#define CAM_FILE_ERROR 3    ///< File can't be saved
#define CAM_BUF_OVERSIZE 4  ///< Camera buffer overszie
#define CAM_BUF_ZERO 5      ///< Camera buffer is 0
#define CAM_FILE_OK 6       ///< File saved correctly    
#define CAM_INIT_ERROR 7

//! Camera return codes messages
std::string msgCam[] = {
    "Camera initialization completed",
    "SPI protocol communication error",
    "Camera sensor OV5642 not found",
    "Test file can't be saved",
    "Camera buffer oversize",
    "Camera buffer empty",
    "Image saved",
    "Can't initialize the camera"
    };
