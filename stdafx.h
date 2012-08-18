// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include "SerialClass.h"
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <time.h>

#define HEIGHT 240
#define WIDTH 320
#define BORDER 1

extern long starttime;
extern long endtime;

class Images {
	public:
	IplImage *pCapImg, *pInpImg, *pSmoothedHand, *pOutHand, 
		*pGrayImg, *pKBOut, *pCanny, *pCannySrc, *pHSV,
		*planeH, *planeS, *planeV;
	int *KB_THRESHOLD;
	int *LINE_THRESHOLD;
	Serial *ardSerial;
};

std::string printPts(CvPoint pt[], int num);
CvPoint getVector(CvPoint &p1, CvPoint &p2);
CvPoint getVector(CvPoint &p1, CvPoint &p2, double slope2);
int getDist(CvPoint &p1, CvPoint &p2);
CvPoint centerOfLines(CvPoint *line1, CvPoint *line2);
CvPoint intersectLine(CvPoint *line1, CvPoint *line2);
CvPoint intersectLine(CvPoint pt1, CvPoint p2, double m1, double m2);
CvPoint getPointFromDist(CvPoint pt, int distance, double slope, char direction);
CvPoint getPointFromDist(CvPoint pt, int distance, double slope, CvPoint destpt);
CvPoint relocatePt(CvPoint pt1, CvPoint pt2, double slope1, double slope2, int distance,char direction);
int mean(CvPoint *lines[], int x, int y, int width, int height);
double getSlope(CvPoint* line);
double getAvgSlope(double slope1, double slope2);
int lineSim(CvPoint* line1, CvPoint* line2);
int cmp_func_x( const void* _a, const void* _b, void* userdata );
int cmp_func_y( const void* _a, const void* _b, void* userdata );
void insSort(CvPoint arr[], int n);

// TODO: reference additional headers your program requires here
