//
//  MouthTrackerAndArmCommander.h
//  Image Guided Feeding Sytem
//
//  Created by James Shorten on 2013/09/11.
//  Copyright (c) 2013 James Shorten. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ThreeDMouthLocationFinder.hpp"
#import "NSImage_OpenCV.h"
#import "ORSSerialPort.h"

@class MouthTrackerAndArmCommander;

@protocol ThreeDMouthLocationFinderDelegate <NSObject>

-(void)newDataIsAvailableWithSender: (MouthTrackerAndArmCommander*) sender;

@end


@interface MouthTrackerAndArmCommander: NSObject <ORSSerialPortDelegate> {
    cv::Mat leftImageMat, rightImageMat;
    ThreeDMouthLocationFinder mouthFinder;
    ORSSerialPort* serialPort;
    NSTimer* nextStepTimer;
}
@property NSImage *leftImage;
@property NSImage *rightImage;
@property double x;
@property double y;
@property double z;
@property BOOL  MouthIsOpen;
@property (readonly) NSString* CoordinateString;
@property (readonly) double xArm;
@property (readonly) double yArm;
@property (readonly) double zArm;

@property id<ThreeDMouthLocationFinderDelegate> delegate;

-(void) updateImagesAndCoordinates;
-(void) feedUser;
-(void) Abort;
-(MouthTrackerAndArmCommander*) init;

- (void)serialPort:(ORSSerialPort *)serialPort didReceiveData:(NSData *)data;
- (void)serialPortWasRemovedFromSystem:(ORSSerialPort *)serialPort;

@end
