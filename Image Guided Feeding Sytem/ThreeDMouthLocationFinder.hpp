/**
 *
 */
#ifndef THREE_D_MOUTH_LOCATION_FINDER_HPP
#define THREE_D_MOUTH_LOCATION_FINDER_HPP
#include <opencv2/opencv.hpp>
#include <exception>
#include <iostream>
#include <cmath>
#include "stereoMatcher.hpp"
#include "MouthPointFinder.hpp"
using namespace cv;
class ThreeDMouthLocationFinder
{
    StereoMatcher *stereoMatcher;
    MouthPointFinder *mouthPointFinder;
    Mat leftFrame, rightFrame;
    Point3d triangulatedMouthPoint;
    bool mouthIsOpen;
    bool newDataIsAvailable;
    VideoCapture *leftFrameCapture;
    VideoCapture *rightFrameCapture;
    
    
    
public:
	/**
	 *    Constructor for the ThreeDMouthLocationfinder.
	 */
    inline ThreeDMouthLocationFinder();
	/**
	 *     Destructor for the ThreeDMouthLocationFinder.
	 */
    inline ~ThreeDMouthLocationFinder();
	/**
	 * Continously compute the mouth position in 3 cordinates;
	 */
    inline void GrabMouthPosition();
    /**
     * Get the data from the grabber.
     * @param leftImage  The openCV mat object in which to place the left image. Passed by reference
     * @param rightImage The openCV mat object in which to place the right image. Passed by reference
     * @param open       A boolean passed by reference that will be true if the system thinks the mouth is open or false otherwise.
     * @param position   An openCV Point3f, passed by reference, that will store the postion of the mouth centre.
     */
    inline void getData(Mat& leftImage, Mat& rightImage, bool& open, Point3f& position);
    /**
     * Tells us if there is new data available;
     * @return true if there is new data otherwise false;
     */
    inline bool isNewDataAvailable();
    
	/* data */
};
inline ThreeDMouthLocationFinder::ThreeDMouthLocationFinder(): triangulatedMouthPoint(0,0,0), mouthIsOpen(false), newDataIsAvailable(false) {
    stereoMatcher = 0;
    mouthPointFinder = new MouthPointFinder();
    leftFrameCapture = new VideoCapture(0); // open Camera attached to usb port 2;
    rightFrameCapture = new VideoCapture(1); // open Camera attached to usb port 1;
}

inline ThreeDMouthLocationFinder::~ThreeDMouthLocationFinder() {
    if(stereoMatcher)
        delete stereoMatcher;
    delete mouthPointFinder;
    delete leftFrameCapture;
    delete rightFrameCapture;
}

inline void ThreeDMouthLocationFinder::GrabMouthPosition() {
    
    if(!(leftFrameCapture->isOpened() && rightFrameCapture->isOpened())) {  // check if we succeeded
        std::cout << "Failed to open cameras" << std::endl;
        return;
    }
    *leftFrameCapture >> leftFrame; // get a new frame from camera 1
    *rightFrameCapture >> rightFrame; // geta new frame from camera 2
    if(!stereoMatcher)
        stereoMatcher = new StereoMatcher("Resources/intrinsic.yml", "Resources/extrinsic.yml", leftFrame.size());
    
    stereoMatcher->rectifyImages(leftFrame, rightFrame);
    
    Point2d leftMouthPoint, rightMouthPoint;
    
    bool isOpenLeft = false;
    bool isOpenRight = false;
    if(mouthPointFinder->detectMouthCentre(leftFrame, leftMouthPoint, isOpenLeft) &&
       mouthPointFinder->detectMouthCentre(rightFrame, rightMouthPoint, isOpenRight)) {
        if (fabs(leftMouthPoint.y - rightMouthPoint.y) < 30) {
            stereoMatcher->triangulateSinglePoint(leftMouthPoint, rightMouthPoint, triangulatedMouthPoint);
            mouthIsOpen = isOpenLeft && isOpenRight;
            newDataIsAvailable = true;
        }
    }
    
}

inline void ThreeDMouthLocationFinder::getData(Mat& leftImage, Mat& rightImage, bool& open, Point3f& position) {
    this->GrabMouthPosition();
    newDataIsAvailable = false;
    leftImage = leftFrame.clone();
    rightImage = rightFrame.clone();
    open = mouthIsOpen;
    position = triangulatedMouthPoint;
}

inline bool ThreeDMouthLocationFinder::isNewDataAvailable() {
    bool retFlg;    retFlg = newDataIsAvailable;
    return retFlg;
}

#endif