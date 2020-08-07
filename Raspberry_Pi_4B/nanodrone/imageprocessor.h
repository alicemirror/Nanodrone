/**
 * @file imageprocessor.h
 * @brief Image processing methods based on Open CV and custom algorithms.
 * 
 * @todo Implement a strong set of controls to avoid crash and error codes
*/

#ifndef _IMAGEPROCESSOR
#define _IMAGEPROCESSOR

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

#define PROCESSOR_MAJOR "1"     ///< Version
#define PROCESSOR_MINOR "0"     ///< Subversion
#define PROCESSOR_BUILD "1"     ///< Build #
//! The image that is in process
#define IMAGE_WINDOW "In-process Image"

/** 
 * Image related information. This structure contains the information related to
 * the \e default image \e that is the image currently processed by the ImageProcessor
 * class.
*/
struct ImageInfo {
    string source;              ///< Image source file name
    Mat img;                    ///< The image source corresponding matrix
    bool hasImage = false;      ///< If true, an image has been loaded
    bool hasUIWindow = false;   ///< UI Window created or not
    string nameUIWindow;        ///< The currently open UI window name    
};
 
class ImageProcessor {

public:
    /**
     * Class constructor
     */
    ImageProcessor(void);

    /**
     * Class constructor
     * 
     * @param The name of the default image
     * @todo Add file validity check
     */
    ImageProcessor(string fileName);
    
    /**
     * Class distructor
     * 
     * If the image window is shown, the image is deleted
     */
    ~ImageProcessor(void);
    
    /**
     * Load the image in a Mat object part of the global ImageInfo structure.
     * 
     * If the structure already contains and image it is overwritten
     * 
     * @todo add check for previous image
     */
    void loadDefaultImage(string fileName);
    
    /**
     * Show the current loaded image on a window
     */
    void showImage();

    /**
     * Close che current UI image window
     */
    void closeImage();
    
    /**
     * Check if the class has already a loaded image
     */
    bool hasImage();

private:
    //! Information related to the current image
    ImageInfo imageInfo;
    
    //! Load in CV the current source image
    //! @todo Add check for CV image loaded
    void infoLoadImage();
    
    /**
     * Show the ImageInfo current Mat in a dedicated window
     * 
     * @param window The window name
     */
    void displayUIWindow(string window);
    
    /**
     * Destroy the UI window to save memory and reset the fag
     */
    void destroyUIWindow();
};
        
#endif
