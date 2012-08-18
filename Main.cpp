/*
 * Main.cpp
 *
 */

#include "stdafx.h"
#include <windows.h>

#include <iostream>
#include <stdio.h>
#include <vector>
#include "Keyboard.h"
#include "Playa.h"
#include "SerialClass.h"

using namespace std;

int KB_THRESHOLD = 115;
int LINE_THRESHOLD = 30;

void trackbarHandler(int pos) {
    KB_THRESHOLD = pos;
}

void lineTrackbarHandler(int pos) {
    LINE_THRESHOLD = pos;
}

int main(int argc, char* argv[])
{
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	Images x = Images();
	IplImage *pCapImg, *pInpImg, *pSmoothedHand;
	IplImage *pOutHand, *pGrayImg, *pKBOut;
	IplImage *pCanny, *pCannySrc, *pHSV;
	IplImage *planeH, *planeS, *planeV;

	//in stdafx.h
	starttime = endtime = 0;

	//CvCapture *capture = cvCaptureFromFile("PWV-00006.wmv");
	CvCapture *capture = cvCaptureFromCAM(1);
	pCapImg = cvQueryFrame(capture);
	int fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS );
	if( fps == 0 ) fps = 25;
	CvVideoWriter *fvid = cvCreateVideoWriter("Zelda.avi", 0, fps, cvSize(WIDTH,HEIGHT), 1);

	if( !pCapImg ) 
	{
		printf("Cannot capture frame from video/camera");
		return 1;
	}

	pInpImg = cvCreateImage(cvSize(WIDTH,HEIGHT), pCapImg->depth, pCapImg->nChannels);
	pOutHand = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	pSmoothedHand = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	pGrayImg = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	pKBOut = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	pCanny = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	pCannySrc = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	pHSV = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), pInpImg->depth, pInpImg->nChannels);
	planeH = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	planeS = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);
	planeV = cvCreateImage(cvSize(pInpImg->width,pInpImg->height), IPL_DEPTH_8U, 0);

	x.pInpImg = pInpImg;
	x.pSmoothedHand = pSmoothedHand;
	x.pOutHand = pOutHand;
	x.pGrayImg = pGrayImg;
	x.pKBOut = pKBOut;
	x.pCanny = pCanny;
	x.pCannySrc = pCannySrc;
	x.pHSV = pHSV;
	x.planeH = planeH;
	x.planeS = planeS;
	x.planeV = planeV;
	x.KB_THRESHOLD = &KB_THRESHOLD;
	x.LINE_THRESHOLD = &LINE_THRESHOLD;

	Keyboard keyboard = Keyboard(x,storage);
	Serial ardSerial = Serial("COM5");
	Playa playa = Playa(keyboard, &ardSerial);

	cvNamedWindow("Project Window", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Project Window",0,0);
	cvNamedWindow("Hand Binary Window", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Hand Binary Window",330,0);
	cvNamedWindow("Keyboard Binary Window", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Keyboard Binary Window",660,0);
	cvNamedWindow("Input Hue", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Input Hue",0,320);
	cvNamedWindow("Input Saturation", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Input Saturation",330,320);
	cvNamedWindow("Input Value", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Input Value",660,320);
	cvNamedWindow("Raw Image Window", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Raw Image Window",0,640);

	cvCreateTrackbar("KB Threshold", "Keyboard Binary Window", &KB_THRESHOLD, 255, trackbarHandler);
	cvCreateTrackbar("Line Threshold", "Project Window", &LINE_THRESHOLD, 255, lineTrackbarHandler);

	int key = 'z';
	while (key != 'q')
	{
		if( !pCapImg ) break;
		cvResize(pCapImg, pInpImg);
		cvShowImage("Raw Image Window", pInpImg);

		//////////////////////////////////////////////////////
		IplImage* tmp = cvCloneImage(pGrayImg);
		keyboard.GetBlackKeys();
		CvRect box = keyboard.blobD.keyboardRect;
		playa.move();
		cvShowImage("Keyboard Binary Window", pCanny);
		///////////////////////////////////////////////////
		
		cvShowImage("Hand Binary Window", pKBOut);
		cvShowImage("Project Window", pInpImg);
		cvWriteFrame(fvid,pInpImg);
		cvShowImage("Input Hue", planeH);
		cvShowImage("Input Saturation", planeS);
		cvShowImage("Input Value", planeV);
		////////////////////////
		cvReleaseImage(&tmp);
		if (keyboard.STOP == 1) {
			keyboard.STOP = 0;
			while(key != 'a') {
				key = cvWaitKey(40);
			}
		}
		///////////////////////
		key = cvWaitKey(1);

		endtime = starttime;
		pCapImg = cvQueryFrame(capture);
		starttime = GetTickCount();
	}

	cvDestroyWindow("Project Window");
	cvDestroyWindow("Hand Binary Window");
	cvDestroyWindow("Keyboard Binary Window");
	cvDestroyWindow("Raw Image Window");
	cvDestroyWindow("Input Hue");
	cvDestroyWindow("Input Saturation");
	cvDestroyWindow("Input Value");

	cvReleaseImage(&pCapImg);
	cvReleaseImage(&pInpImg);
	cvReleaseImage(&pSmoothedHand);
	cvReleaseImage(&pOutHand);
	cvReleaseImage(&pGrayImg);
	cvReleaseImage(&pKBOut);
	cvReleaseImage(&pCanny);
	cvReleaseImage(&pHSV);
	cvReleaseImage(&planeH);
	cvReleaseImage(&planeS);
	cvReleaseImage(&planeV);
	cvReleaseVideoWriter(&fvid);
	cout << "this is fine" << endl;

	return 0;
}
