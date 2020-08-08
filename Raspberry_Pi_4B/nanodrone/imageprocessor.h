/**
 * @file imageprocessor.h
 * @brief Image processing methods based on Open CV and custom algorithms.
 * 
 * @todo Implement a strong set of controls to avoid crash and error codes
*/

#ifndef _IMAGEPROCESSOR
#define _IMAGEPROCESSOR

//! Undef the debug below to remove the console messages
#undef _IMG_DEBUG

#include <iostream>
#include <string.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

#define PROCESSOR_MAJOR 1     ///< Version
#define PROCESSOR_MINOR 0     ///< Subversion
#define PROCESSOR_BUILD 5     ///< Build #
//! The image that is in process
#define IMAGE_WINDOW "Image"
//! The processed image prefix (the file name is the same as the source)
#define PROCESSED_FILE_PREFIX "_"

/** 
 * Image related information. This structure contains the information related to
 * the \e default image \e that is the image currently processed by the ImageProcessor
 * class.
*/
struct ImageInfo {
    string source;              ///< Image source file name
    bool hasImage = false;      ///< If true, an image has been loaded
    bool hasUIWindow = false;   ///< UI Window created or not
    string nameUIWindow;        ///< The currently open UI window name 
    double lightingIndex;
    double lightingPerc;
};

/**
 * @brief The lightIndexes struct defines the estimated values of lighting index
 * and light intensity percentage used to improve the image exposure before processing
 * if needed.
 */
struct LightIndexes {
    /**
     * @brief lightingIndex The lighting index based on the non zero pixels
     * calculated with the formula (ImageSize = NonZeroPixels) / ImageSize
     * This is a fixed parameter indicating the maximum value that can be
     * assumed by the image.
     */
    double lightingIndex;
    /**
     * @brief lightingPerc The lighting percentage, also called the estimated brightness.
     * This is a fixed parameter, indicating the minimum non-black pixels percentage should
     * be present on the image to make it acceptable for processing.
     */
    int lightingPerc;
    /**
     * @brief Maximum number or image equalization retries before the algorithm is forced to stop.
     * 
     * As the exposure adjustment respect the optimal parameters is appliled in 
     * a loop, it is possible that for particularly wrong images or for some other conditions
     * the loop should be forced to stop the image equalization before the optimal limits
     * are reached. If this occurs after a certain number of retries, when this value is reached
     * the equalization algorithm stops and exit with the image adjusted in the best possible
     * conditions.
     */
    int maxExposureAdjust;
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
     * Save the processed Mat image with a prefix and the same source file name
     */
    void saveProcessedImage();
    
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
    
    /**
     * @brief correctExposure Check the lighting (exposure) levels of the imageto be
     * equalised. If the lighting is out of range after a number of retries the image is discarded
     *
     * @note This is an image postprocess that not necessarily applies. In most of the images no
     * lighting adjustmend is needed. If the image is excessively lighter or darker, a limiting
     * loop avoid too many exposure steps correction.
     *
     * @param idx The parameters control structure
     * @return The number of equalization loops.
     */
    int correctExposure(LightIndexes* idx);

private:
    //! Information related to the current image
    ImageInfo imageInfo;
    LightIndexes light;
    Mat img;
    
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

    /**
     * @brief checkLighting Get the lighting level of the current image based on the greyscale
     * conversion and thresholding the image to the middle values.
     */
    void checkLighting();
    
    /**
     * @brief adjustExposure Get the lighting level of the current image and adjust exposure
     * (brightness/contrast) to omologate the image before the recognition process
     * starts.
     * 
     * The algorithm uses two indexes alpha and beta representing respectively the contrast and
     * brightness control.
     * 
     * It calculates new values for image correction.
     * 
     * @note The values of lighting are multiplied by a factor of ten to move them
     * in a range. 
     * The alpha/beta increment value corresponds to the difference of the idetected 
     * lighting of the image respect to the optimized values passed as a parameter.
     * 
     * @param li Light intensity
     * @param lp Light percentage
     */
    void adjustExposure(LightIndexes* idx);
};
        
#endif
