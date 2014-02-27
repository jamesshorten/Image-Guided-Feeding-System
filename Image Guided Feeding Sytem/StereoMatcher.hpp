/**
 * @file
 * @author James Shorten
 * @section Descriptiomn
 *
 * The Stereo Matcher class describes an object to perform stereo matching on a pair of images from a stereo camera.
 * It uses calibration information to rectify the images and then uses the SGBM algorithm to perform the matching.
 * It is capable of genearating a point cloud and scaling the output accoring to a defined scale factor
 */
#ifndef STEREOMATCHER_HPP
#define STEREOMATCHER_HPP
#include <exception>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include "CoreFoundation/CoreFoundation.h"
using namespace cv;

class FileNotOpenedException: public std::exception
{
  inline virtual const char* what() const throw()
  {
    return "File failed to open\n";
  }
};

class StereoMatcher
{
	Mat M1, D1, M2, D2, R, T, R1, R2; // The calibration matrices
	Mat P1, P2, Q; // Projection Matrices
    cv::Rect roi1, roi2; //Not sure what these are for yet
	Mat map11, map12, map21, map22; //Rectification transform maps.
	StereoSGBM sgbm; //SGBM algorithm object.
    cv::Size imageSize; // The size of the input images.
	int numberOfDisparities; // number of disparity levels to compute.
	FileNotOpenedException fileNotOpenedException;
public:
	/**
	 * Constructor to initialize the StereoMatcher object with the camera parameters and the scale factor
	 *
	 * @param intrinsicParameterFileName The path to the YAML/XML file containing the intrinsic parameters for the stereo setup.
	 * @param extrinsicParameterFileName The path to the YAML/XML file containing the extrinsic parameters for the stereo setup.
	 * @param imageS The size of the input images. Get with img.Size().
	 * The intrinsic and extrinsic paramters are used to rectify the images so that the stereo algorithm works correctly.
	 * The rectification transform ensures that the image pair is in the same plane and that each row of pixels in each image corresponds
	 * to a row in the other.
	 *
	 * Calibration is done with the opencv stereo calibration sample program.
	 */
	inline StereoMatcher(std::string intrinsicParameterFileName, std::string extrinsicParameterFileName, cv::Size imageS);
	/**
	 * A method to perform the match
	 * @param left       The left camera's image
	 * @param right      The right camera's image
	 * @param pointCloud A reference to the point cloud in which to sotre the output.
	 * @param disparityMap The output disparity map from the algorithm.
	 */
	inline void Match(Mat left, Mat right, Mat &pointCloud, Mat &disparityMap);
	/**
	 * A mthod to rectify two images from the stereo Camera.
	 * @param left  [description]
	 * @param right [description]
	 */
	inline void rectifyImages(Mat &left, Mat &right);
	/**
	 * Take a point from each image (the same feature) and use traingulation to find a point in 3d space.
	 * @param leftImagePoint  The point from the left image
	 * @param rightImagePoint The  point from the right image.
	 * @param ThreeDPoint     The point reprojected in 3 dimensions.
	 */
	inline void triangulateSinglePoint(Point2d leftImagePoint, Point2d rightImagePoint, Point3d &ThreeDPoint);
	~StereoMatcher();
};

inline StereoMatcher::StereoMatcher(std::string intrinsicParameterFileName, std::string extrinsicParameterFileName, cv::Size imageS) {
	imageSize = imageS;
    
    // Look for a resource in the main bundle by name and type.
    
	CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef fileURL1 = CFBundleCopyResourceURL( mainBundle, CFSTR("intrinsic"), CFSTR("yml"), NULL );
    char path1[512];
    if (!CFURLGetFileSystemRepresentation(fileURL1, TRUE, (UInt8 *)path1, 512))
        throw fileNotOpenedException;

    FileStorage fs(path1, CV_STORAGE_READ);
    
	if(!fs.isOpened())
		throw fileNotOpenedException;
	fs["M1"] >> this->M1;
    fs["D1"] >> this->D1;
    fs["M2"] >> this->M2;
    fs["D2"] >> this->D2;

    CFURLRef fileURL2 = CFBundleCopyResourceURL( mainBundle, CFSTR("extrinsic"), CFSTR("yml"), NULL );
    char path2[512];
    if (!CFURLGetFileSystemRepresentation(fileURL2, TRUE, (UInt8 *)path2, 512))
        throw fileNotOpenedException;
    
    fs.open(path2, CV_STORAGE_READ);
    if(!fs.isOpened())
    	throw fileNotOpenedException;
    fs["R"] >> this->R;
    fs["T"] >> this->T;

    stereoRectify( M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, imageSize, &roi1, &roi2 );
    initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
    initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);
    numberOfDisparities = 256;

}

inline void StereoMatcher::Match(Mat left, Mat right, Mat &pointCloud, Mat &disparityMap) {
	Mat leftRectified, rightRectified;
	remap(left, leftRectified, map11, map12, INTER_LINEAR);
    remap(right, rightRectified, map21, map22, INTER_LINEAR);
    if(numberOfDisparities == 0)
    	numberOfDisparities = ((leftRectified.size().width/8) + 15) & -16;
    int cn = leftRectified.channels();
    sgbm.preFilterCap = 0;
    sgbm.SADWindowSize = 3;
    sgbm.P1 = 8*cn*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.P2 = 32*cn*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.minDisparity = 0;
    sgbm.numberOfDisparities = numberOfDisparities;
    sgbm.uniquenessRatio = 5;
    sgbm.speckleWindowSize = 0;
    sgbm.speckleRange = 1;
    sgbm.disp12MaxDiff = 600;
    sgbm.fullDP = false;
    Mat disp;
    sgbm(leftRectified, rightRectified, disp);
    disp.convertTo(disparityMap, CV_8U, 255/(numberOfDisparities*16.));
    disparityMap = disp;
    reprojectImageTo3D(disparityMap, pointCloud, Q, false);
}

inline void StereoMatcher::rectifyImages(Mat &left, Mat &right) {
	remap(left, left, map11, map12, INTER_LINEAR);
    remap(right, right, map21, map22, INTER_LINEAR);
}

inline void StereoMatcher::triangulateSinglePoint(Point2d leftImagePoint, Point2d rightImagePoint, Point3d &ThreeDPoint) {

	Mat outputArray(1,1,CV_64FC4);
	std::vector<Point2d> leftPoints;
	std::vector<Point2d> rightPoints;
	leftPoints.push_back(leftImagePoint);
	rightPoints.push_back(rightImagePoint);	
	triangulatePoints(P1, P2, leftPoints, rightPoints, outputArray);
	Vec4d pointData = outputArray.at<Vec4d>(0, 0);
	ThreeDPoint.x = pointData[0]/pointData[3];
	ThreeDPoint.y = pointData[1]/pointData[3];
	ThreeDPoint.z = pointData[2]/pointData[3];
	// if (outputPoints.size() > 0) {
	// 	ThreeDPoint = outputPoints[0];
	// }
}
inline StereoMatcher::~StereoMatcher() {

}
#endif