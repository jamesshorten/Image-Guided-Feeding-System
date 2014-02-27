//
//  AppDelegate.m
//  Image Guided Feeding Sytem
//
//  Created by James Shorten on 2013/09/11.
//  Copyright (c) 2013 James Shorten. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    commandAndTrack = [[MouthTrackerAndArmCommander alloc] init];
    commandAndTrack.delegate = self;
    
    videoUpdateTimer = [NSTimer scheduledTimerWithTimeInterval:0.067
                                                        target:commandAndTrack selector:@selector(updateImagesAndCoordinates)
                                                      userInfo:nil repeats:YES];
}

- (IBAction)buttonPressedWithButton:(id)sender {
    NSButton* button = sender;
    NSString* buttonName = button.title;
    
    if([buttonName isEqualToString:@"Feed"])
        [commandAndTrack feedUser];
    else if ([buttonName isEqualToString:@"Abort"])
        [commandAndTrack Abort];
}

-(void)newDataIsAvailableWithSender: (MouthTrackerAndArmCommander*) sender  {
    self.rightImageView.image = sender.rightImage;
    [self.rightImageView setNeedsDisplay];
    self.leftImageView.image = sender.leftImage;
    [self.leftImageView setNeedsDisplay];
    [self.CoordinatesField setStringValue:sender.CoordinateString];
}
@end
