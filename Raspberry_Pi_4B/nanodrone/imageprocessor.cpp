/**
 * @file imageprocessor.cpp
 * @brief Image processing methods based on Open CV and custom algorithms.
 * 
*/

#include "imageprocessor.h"

ImageProcessor::ImageProcessor(void) {
    imageInfo.hasImage = false;
    }

ImageProcessor::~ImageProcessor(void) {
    if(imageInfo.hasUIWindow) {
        destroyUIWindow();
    }
}
    
ImageProcessor::ImageProcessor(string fileName) {
    imageInfo.hasImage = false;
    imageInfo.source = fileName;
    infoLoadImage();
}
    
void ImageProcessor::infoLoadImage() {
    imageInfo.img = imread(imageInfo.source, CV_LOAD_IMAGE_COLOR);
    imageInfo.hasImage = true;
}
    
void ImageProcessor::loadDefaultImage(string fileName) {
    imageInfo.source = fileName;
    infoLoadImage();
}

bool ImageProcessor::hasImage() {
    return imageInfo.hasImage;
}
    
void ImageProcessor::showImage() {
    displayUIWindow(IMAGE_WINDOW);
}
    
void ImageProcessor::closeImage() {
    destroyUIWindow();
}
    
void ImageProcessor::displayUIWindow(string window) {
    if(!imageInfo.hasUIWindow) {
        namedWindow(window, WINDOW_AUTOSIZE );
        imageInfo.hasUIWindow = true;
        imageInfo.nameUIWindow = window;
    }
    imshow(window, imageInfo.img);
}

void ImageProcessor::destroyUIWindow() {
    if(imageInfo.hasUIWindow) {
        destroyWindow(imageInfo.nameUIWindow);
        imageInfo.hasUIWindow = false;
    }
}
