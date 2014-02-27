//
//  AppDelegate.h
//  Image Guided Feeding Sytem
//
//  Created by James Shorten on 2013/09/11.
//  Copyright (c) 2013 James Shorten. All rights reserved.
//
#import <Cocoa/Cocoa.h>
#import "MouthTrackerAndArmCommander.h"

@interface AppDelegate : NSObject <NSApplicationDelegate,ThreeDMouthLocationFinderDelegate> {
    MouthTrackerAndArmCommander* commandAndTrack;
    NSTimer* videoUpdateTimer;
    
}

@property (assign) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSImageView *rightImageView;
@property (weak) IBOutlet NSImageView *leftImageView;
@property (weak) IBOutlet NSTextField *CoordinatesField;

- (IBAction)buttonPressedWithButton:(id)sender;
-(void)newDataIsAvailableWithSender: (MouthTrackerAndArmCommander*) sender;
@end
