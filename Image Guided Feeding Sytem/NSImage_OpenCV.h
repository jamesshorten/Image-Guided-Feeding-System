//
//  NSImage_OpenCV.h
//  Image Guided Feeding Sytem
//
//  Created by James Shorten on 2013/10/11.
//  Copyright (c) 2013 James Shorten. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// NSImage+OpenCV.h
@interface NSImage (OpenCV)
+ (NSImage*)imageWithCVMat:(const cv::Mat&)cvMat;
- (id)initWithCVMat:(const cv::Mat&)cvMat;
- (cv::Ptr<cv::Mat>)cvMat;
@end

using namespace cv;
@implementation NSImage (OpenCV)

- (id)initWithCVMat:(const cv::Mat&)cvMat {
    cvtColor( cvMat, cvMat, CV_BGR2RGB);
    NSData *data = [NSData dataWithBytes:cvMat.data
                                  length:cvMat.total()*cvMat.elemSize()];
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    CGColorSpaceRef colourSpace =  CGColorSpaceCreateDeviceRGB();
    
    CGImageRef imageRef = CGImageCreate(cvMat.cols,
                                        cvMat.rows,
                                        8,
                                        8 * cvMat.elemSize(),
                                        cvMat.step[0],
                                        colourSpace,
                                        kCGImageAlphaNone |
                                        kCGBitmapByteOrderDefault,
                                        provider,
                                        NULL,
                                        false,
                                        kCGRenderingIntentDefault);
    
    NSImage *image = [[NSImage alloc] initWithCGImage:imageRef size:CGSizeMake(cvMat.cols,cvMat.rows)];
    
    CGColorSpaceRelease(colourSpace);
    CGDataProviderRelease(provider);
    CGImageRelease(imageRef);
    
    return image;
}

+(NSImage*)imageWithCVMat:(const cv::Mat&)cvMat {
    return [[NSImage alloc] initWithCVMat:cvMat];
}

- (cv::Ptr<cv::Mat>)cvMat {
    CGImageSourceRef source =  CGImageSourceCreateWithData((__bridge CFDataRef)[self TIFFRepresentation],  NULL);
    CGImageRef imageRef =  CGImageSourceCreateImageAtIndex(source, 0, NULL);
    cv::Ptr<cv::Mat> cvMat = new cv::Mat(self.size.height, self.size.width, CV_8UC4);
    
    CGContextRef contextRef = CGBitmapContextCreate(cvMat->data,
                                                    cvMat->cols,
                                                    cvMat->rows,
                                                    8,
                                                    cvMat->step[0],
                                                    CGImageGetColorSpace(imageRef),
                                                    kCGImageAlphaNoneSkipLast |
                                                    kCGBitmapByteOrderDefault);
    
    CGContextDrawImage(contextRef,
                       CGRectMake(0, 0, cvMat->cols, cvMat->rows),
                       imageRef);
    
    CGContextRelease(contextRef);
    CGImageRelease(imageRef);
    
    return cvMat;
}
@end
