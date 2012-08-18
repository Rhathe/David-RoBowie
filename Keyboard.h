#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "cv.h"
#include "highgui.h"
#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include "BlobDetector.h"

//number of corners to find
#define CORNER_COUNT 200
#define NUMBER_OF_KEYS 12
#define LINE_COLOR CV_RGB(0,255,0)
#define LINE_THICKNESS 1
#define FINGER_COUNT 4
#define MAX_DISTANCE 1000000
#define EF 3
#define BC 1
#define FPS 20

class Keyboard
{
	public:
		struct keyStruct {
			std::string note;
			CvPoint pos, *line1, *line2;
			double slope;
			keyStruct(CvPoint pos, std::string note, double slope) {
				this->note = note;
				this->pos = pos;
				this->slope = slope;
				line1 = 0;
				line2 = 0;
			};
			keyStruct(CvPoint pos, std::string note, double slope, CvPoint *line1, CvPoint *line2) {
				this->note = note;
				this->pos = pos;
				this->slope = slope;
				this->line1 = line1;
				this->line2 = line2;
			};
			keyStruct() {
				note = "";
				pos = cvPoint(0,0);
				slope = 0;
				line1 = 0;
				line2 = 0;
			};
		};
		void init();
		Keyboard();
		Keyboard(Images x, CvMemStorage *storage);

		void GetBlackKeys();
		void GetHorizLines(CvSeq* lines);
		void addKey(CvPoint pt, std::string note, double slope, CvPoint *prevLine, CvPoint *line);
		void addBlackKey(CvPoint pt, double slope, CvPoint *prevLine, CvPoint *line);
		void labelKeys();
		void clearKeys();
		int direction();
		int mean(CvPoint *lines[], int x, int y, int width, int height);
		int getFirstKey(std::vector<int> &wskey, char** keyNotes);
		char octave(char* keyNotes[]);
		void labelOctaves(char octave, char *keyNotes[]);
		std::vector<CvScalar> horizLine;
		std::vector<keyStruct> blackKeys, keys, prevkeys;
		int STOP;
		BlobDetector blobD;
		static const int keyNum = 12;
		IplImage *src;
		float speedLimit;

private:
		IplImage *pCapImg, *pSmoothedHand, *pOutHand, 
		*pGrayImg, *pKBOut, *pCanny, *pHSV,
		*planeH, *planeS, *planeV;
		CvMemStorage *storage;
		int *LINE_THRESHOLD;
		std::vector<CvScalar> prevHorizLine;
		CvFont font;
		char globOctaveC;
};

#endif /*KEYBOARD_H_ */
