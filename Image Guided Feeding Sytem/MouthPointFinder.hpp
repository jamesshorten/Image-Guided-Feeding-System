/**
 * @file
 * @author James Shorten
 * @section Descriptiomn
 *
 * The MouthPointFinder class describes an object that will find the corners, top and bottom points of a mouth in an image of a face.
 * It will also let us find the centre point of the mouth and whether the mouth is open  or closed.
 */
#ifndef MOUTHPOINTFINDER_HPP
#define MOUTHPOINTFINDER_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <exception>
#include "CoreFoundation/CoreFoundation.h"
using namespace cv;

class FileFailedToLoad: public std::exception
{
    inline virtual const char* what() const throw()
    {
        return "File failed to load\n";
    }
};

class MouthPointFinder
{
    std::string faceCascadeName;
    std::string mouthCascadeName;
    CascadeClassifier faceCascade; // The face cascade classifier
    CascadeClassifier mouthCascade; // The mouth cascade classifier
public:
	inline MouthPointFinder();
	/**
	 * detectMouthCentre finds the centre of the mouth if there is a face in the frame given to it. It uses haar cascade calssifiers
	 * to segment that image. First it finds a rectangle arount the face and then a rectangle arount the mouth. Then the area aroiund the mouth
	 * is blured (Low pass filter) and thresholded. This thresholded image is then used to find contours and finally a convex hull around the mouth.
	 * A rectangle is that contains the hull is then found and the centre of the mouth comuted from that. The width and height of the rectangle are used
	 * to determine if the mouth is open.
	 * @param  frame       reference to the frame of interest
	 * @param  mouthCentre reference to a point where the mouth centre will be stored;
	 * @param  mouthIsOpen reference to a boolean value that will let the caller know if the mouth is open (true if open).
	 * @return             true if successful, false otherwise
	 */
	inline bool detectMouthCentre(Mat &frame, Point2d &mouthCentre, bool &mouthIsOpen);
};

inline MouthPointFinder::MouthPointFinder() {
    FileFailedToLoad exception;
    faceCascadeName = "Resources/Cascades/haarcascade_frontalface_alt.xml"; // File path for the face haar cascade classifier
    mouthCascadeName = "Resources/Cascades/haarcascade_mcs_mouth.xml"; // File path for the mouth haar cascade classifier
    
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef fileURL1 = CFBundleCopyResourceURL( mainBundle, CFSTR("haarcascade_frontalface_alt"), CFSTR("xml"), NULL );
    char path1[512];
    if (!CFURLGetFileSystemRepresentation(fileURL1, TRUE, (UInt8 *)path1, 512))
        throw exception;
    
    
    CFURLRef fileURL2 = CFBundleCopyResourceURL( mainBundle, CFSTR("haarcascade_mcs_mouth"), CFSTR("xml"), NULL );
    char path2[512];
    if (!CFURLGetFileSystemRepresentation(fileURL2, TRUE, (UInt8 *)path2, 512))
        throw exception;
    
    if(!faceCascade.load(path1)){
        throw exception;
    }
    if(!mouthCascade.load(path2)){
        throw exception;
    }
}

inline bool MouthPointFinder::detectMouthCentre(Mat &frame, Point2d &mouthCentre, bool &mouthIsOpen) {
    bool retFlg = false;
    std::vector<cv::Rect> faces;
    Mat grayScaleFrame;
    
    cvtColor(frame, grayScaleFrame, CV_BGR2GRAY);
    equalizeHist(grayScaleFrame, grayScaleFrame);
    
    // Detect faces
    faceCascade.detectMultiScale(grayScaleFrame, faces, 1.25, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(400, 400));
    
    for( int i = 0; i < faces.size() && i < 1; i++ ) {
        //faces[i].height += faces[i].height/5;
        if(faces[i].height > 0 && faces[i].width > 0 && faces[i].x > 0 && faces[i].y > 0) {
            rectangle(frame, faces[i], Scalar(255,0,0), 2);
            cv::Rect faceRect = faces[i];
            faceRect.y = faceRect.y + faceRect.height/2;
            faceRect.height = faceRect.height/2 + 1;
            Mat faceROI = grayScaleFrame(faceRect);
            std::vector<cv::Rect> mouths;
            
            // In each face, detect mouths
            mouthCascade.detectMultiScale(faceROI, mouths, 1.25, 2, 0 |CV_HAAR_SCALE_IMAGE, cv::Size(faceRect.width/3, faceRect.height/10));
            
            for( int j = 0; j < mouths.size() && j < 1; j++ ) {
                mouths[j].y = mouths[j].y - mouths[j].height/10;
                cv::Point Vert1(faceRect.x + mouths[j].x, faceRect.y + mouths[j].y);
                cv::Point Vert2(Vert1.x + mouths[j].width, Vert1.y + mouths[j].height);
                rectangle(frame, Vert1, Vert2, Scalar(0,0,255), 2);
                
                Mat facePointsLocal;
                if(mouths[j].height > 0 && mouths[j].width > 0 && mouths[j].x > 0 && mouths[j].y > 0) {
                    facePointsLocal =faceROI(mouths[j]);
                    equalizeHist(facePointsLocal, facePointsLocal);
                    Mat copy = facePointsLocal.clone();
                    
                    blur(copy, copy, cv::Size(4,4));
                    threshold(copy, copy, 50, 255, 0);
                    Canny(copy, copy, 10, 30, 3, true);
                    
                    std::vector<std::vector<cv::Point> > contours;
                    std::vector<Vec4i> hierarchy;
                    findContours(copy, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
                    
                    std::vector<cv::Point> hull;
                    double maxArea = 0.0;
                    RotatedRect boundingRect;
                    for(int i = 0; i < contours.size(); i++) {
                        double area = contourArea(contours[i], false);
                        if(area > maxArea) {
                            maxArea = area;
                            convexHull(Mat(contours[i]), hull, false);
                            boundingRect = minAreaRect(hull);
                        }
                    }
                    
                    Point2f boundingRectVertices[4];
                    boundingRect.points(boundingRectVertices);
                    mouthCentre = Point2d((boundingRectVertices[0].x + boundingRectVertices[2].x)*0.5 + mouths[j].x + faceRect.x,
                                          (boundingRectVertices[0].y  + boundingRectVertices[2].y)*0.5 + mouths[j].y + faceRect.y);
                    mouthIsOpen = boundingRect.size.width/boundingRect.size.height > 2;
                    
                    if(mouthIsOpen) {
                        circle(frame, mouthCentre, 10, Scalar(0, 255, 0), -1);
                    } else {
                        circle(frame, mouthCentre, 10, Scalar(0, 255, 0), 2);
                    }
                    
                    retFlg = true;
                }
            }
        }
    }
    return retFlg;
}


#endif