#ifndef BLOBDETECTOR_H_
#define BLOBDETECTOR_H_

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>

class BlobDetector {
	
	public:
		BlobDetector();
		void init();
		BlobDetector(Images x) ;
		CvBox2D minRect;
		CvRect windowRect;
		CvPoint fingers[4];
		CvRect keyboardRect;
		CvPoint rect_points[4];
		void DetectBlobs();
		void FindFingers();
		void FindFingerTips();
		int fingNum;

	private:
		IplImage *src, *pCanny, *pCannySrc, *pHSV, *planeH, *planeS, *planeV, *pGrayImg, *pKBOut;
		CvSeq* contours, result;
		CvMemStorage *storage, *approxPoly;
		unsigned char hueThresh, satThresh, valThresh;
		int *KB_THRESHOLD;
		Serial *ardSerial;
};


#endif /*BLOBDETECTOR_H_ */