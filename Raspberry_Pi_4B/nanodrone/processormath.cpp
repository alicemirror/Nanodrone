/**
 * @file processormath.cpp
 * @brief Image processing algorithms and calculations.
 * 
*/

#include "imageprocessor.h"

void ImageProcessor::checkLighting() {

    //! The size of the processed image
    int imgSize = img.rows * img.cols;
    //! Temporary image
    cv::Mat tImg;
    //! Grey level image used for calculations
    cv::Mat greyImg; 

    // Note that the threshold max val is set to -1
    // as it's not used in CV_THRESH_TOZERO
    cv::cvtColor(img, greyImg, cv::COLOR_BGR2GRAY);
    // Lighting thereshold level for lighting calculation is fixed to the
    // mid of the grayscale values range (0-255)
    cv::threshold(greyImg, tImg, 127, -1, cv::THRESH_TOZERO);

    //! Count the non-zero pixels on the whole image
    int nonzero = cv::countNonZero(tImg);

    light.lightingIndex = (imgSize - nonzero) / double(imgSize);
    light.lightingPerc = double(nonzero * 100) / double(imgSize);
}

int ImageProcessor::correctExposure(LightIndexes* idx) {
    // Get the image current lighting levels
    checkLighting();

    //! Counter to avoid for some unpredictable reason an infinite loop on the
    //! adjustExposure() while loop
    int checkLightingExitCondition = 0;

    // Check if the image needs lighting enhancement
    while( (light.lightingIndex > idx->lightingIndex) &&
            (light.lightingPerc < idx->lightingPerc) &&
            (checkLightingExitCondition++ <= idx->maxExposureAdjust) ) {
        adjustExposure(idx);
        checkLighting();
    } // Exposure adjustment loop
    
    // Check for the exit condition
    if(checkLightingExitCondition > idx->maxExposureAdjust)
        return idx->maxExposureAdjust;
    else
        return checkLightingExitCondition;
}

void ImageProcessor::adjustExposure(LightIndexes* idx) {
    double alpha = 1.0; ///< Contrast control
    int beta = 0; ///< Brightness control

    alpha += (light.lightingIndex - idx->lightingIndex) * 10;
    beta = int(light.lightingPerc - idx->lightingPerc) * 10;

    for( int y = 0; y < img.rows; y++ ) {
        for( int x = 0; x < img.cols; x++ ) {
            for( int c = 0; c < 3; c++ ) {
                img.at<cv::Vec3b>(y, x)[c] =
                  cv::saturate_cast<uchar>( alpha * ( img.at<cv::Vec3b>(y, x)[c] ) + beta );
            } // Loop on channels (RGB)
        } // Loop on columns
    } // Loop on rows
}



