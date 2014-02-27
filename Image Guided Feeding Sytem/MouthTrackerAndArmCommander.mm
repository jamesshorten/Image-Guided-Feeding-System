//
//  MouthTrackerAndArmCommander.m
//  Image Guided Feeding Sytem
//
//  Created by James Shorten on 2013/09/11.
//  Copyright (c) 2013 James Shorten. All rights reserved.
//

#import "MouthTrackerAndArmCommander.h"

@implementation MouthTrackerAndArmCommander

-(double) xArm {
    return self.x - 22.0;
}

-(double) yArm {
    return self.z - 18;
}

-(double) zArm {
    return self.y + 19.5;
}

-(void) updateHelperWithDelegate: (id<ThreeDMouthLocationFinderDelegate>) delegate {
    cv::Point3f point;
    bool isOpen = false;
    mouthFinder.getData(leftImageMat, rightImageMat, isOpen, point);
    self.MouthIsOpen = isOpen ? FALSE : TRUE;
    self.x = point.x * -2.0;
    self.y = point.y * 2.0;
    self.z = point.z * -2.0;
    
    self.leftImage = [NSImage imageWithCVMat:leftImageMat];
    self.rightImage = [NSImage imageWithCVMat:rightImageMat];
    [self.delegate newDataIsAvailableWithSender: self];
}

-(void) updateImagesAndCoordinates {
    [self updateHelperWithDelegate:self.delegate];
}

-(void) Abort {
    //[serialPort open];
    
    [nextStepTimer invalidate];
    NSString* command = @"A";
    NSLog(@"Command: %@",command);
    [serialPort sendData:[command dataUsingEncoding:NSUTF8StringEncoding]];
    //[serialPort close];
}

-(void) rest {
    [self Abort];
}

-(void) retrieve {
    //[serialPort open];

    
    NSString* command = [NSString stringWithFormat:@"M %1.2f %1.2f %1.2f %1.2f %1.2f",self.xArm, self.yArm-15, self.zArm, 0.0, 1.0];
    [serialPort sendData:[command dataUsingEncoding:NSUTF8StringEncoding]];
    //[serialPort close];
    NSLog(@"Command: %@",command);
    
    nextStepTimer = [NSTimer scheduledTimerWithTimeInterval:5.0
                                                     target:self selector:@selector(rest)
                                                   userInfo:nil repeats:NO];
}

-(void) insert {
    //[serialPort open];

    
    NSString* command =[NSString stringWithFormat:@"M %1.2f %1.2f %1.2f %1.2f %1.2f",self.xArm, self.yArm, self.zArm+3.0, 0.0, 1.0];
    [serialPort sendData:[command dataUsingEncoding:NSUTF8StringEncoding]];
    //[serialPort close];
    NSLog(@"Command: %@",command);
    
    nextStepTimer = [NSTimer scheduledTimerWithTimeInterval:5.0
                                                     target:self selector:@selector(retrieve)
                                                   userInfo:nil repeats:NO];

}
-(void) scoop {
    //[serialPort open];
    
    NSString* command = [NSString stringWithFormat:@"S %1.2f %1.2f %1.2f %1.2f %1.2f",self.xArm, self.yArm-15, self.zArm+3.0, 0.0, 1.0];
    
    [serialPort sendData:[command dataUsingEncoding:NSUTF8StringEncoding]];
    //[serialPort close];
    NSLog(@"Command: %@",command);
    
    nextStepTimer = [NSTimer scheduledTimerWithTimeInterval:15.0
                                                    target:self selector:@selector(insert)
                                                   userInfo:nil repeats:NO];

}


-(void) feedUser {
        [self scoop];
}

-(NSString*) CoordinateString {
    return [NSString stringWithFormat:@"x: %1.2f, y: %1.2f, z: %1.2f, Open: %@", self.x, self.y, self.z, self.MouthIsOpen ? @"true" : @"false"];
}

-(id) init {
    _x = 0.0;
    _y = 0.0;
    _z = 0.0;
    _MouthIsOpen = NO;
    serialPort = [ORSSerialPort serialPortWithPath:@"/dev/cu.usbmodem14121"];
    serialPort.baudRate = [NSNumber numberWithInt:115200];
    serialPort.numberOfStopBits = 1;
    serialPort.parity = ORSSerialPortParityNone;
    serialPort.delegate = self;
    [serialPort open];
    return self;
}


- (void)serialPort:(ORSSerialPort *)serialPort didReceiveData:(NSData *)data {
    NSString *string = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSLog(@"%@", string);
    
}
- (void)serialPortWasRemovedFromSystem:(ORSSerialPort *)serialPort {
    
}
@end
