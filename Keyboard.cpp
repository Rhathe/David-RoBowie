#include "Keyboard.h"
#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <string>
#include <sstream>
#include <algorithm>

// returns the average Color of the Pixels in the rect starting at (x,y) with size (width, height)
int Keyboard::mean(CvPoint *lines[], int x, int y, int width, int height) {

	CvPoint *a,*b,*c,*d;
	if (lines[0][0].y < lines[0][1].y) {
		b = &lines[0][0];
		a = &lines[0][1];
	}
	else {
		b = &lines[0][1];
		a = &lines[0][0];
	}
	if (lines[1][0].y < lines[1][1].y) {
		c = &lines[1][0];
		d = &lines[1][1];
	}
	else {
		c = &lines[1][1];
		d = &lines[1][0];
	}
	CvPoint pts[] = {*a,*b,*c,*d};

	IplImage* tmpImg = cvCreateImage(cvSize(WIDTH,HEIGHT), pKBOut->depth, pKBOut->nChannels);
	cvRectangle( tmpImg, cvPoint(0, 0), cvPoint(WIDTH, HEIGHT), CV_RGB(0,0,0), CV_FILLED);
	cvFillConvexPoly(tmpImg, pts, 4, cvScalar(255,255,255));

	cvAnd(pKBOut,tmpImg,tmpImg);
	
	double area = 0;
	for(int i = 0; i < 4; ++i) {
		area += 0.5*(pts[i].x*pts[(i+1)%4].y-pts[i].y*pts[(i+1)%4].x);
	}

	CvScalar sum = cvSum(tmpImg);
	sum.val[0] /= area;
	if (sum.val[0] < 150) {
		cvFillConvexPoly(src, pts, 4, cvScalar(255,0,0));
		cvLine( src, *a, *b, CV_RGB(0,255,0), 2, 8 );
		cvLine( src, *c, *d, CV_RGB(0,255,0), 2, 8 );
	}

	cvReleaseImage(&tmpImg);

	return sum.val[0];
}

void Keyboard::init() {
	STOP = 0;
	speedLimit = 15;
	globOctaveC = '5';
	cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX_SMALL, 0.5, 0.5, 0, 1, CV_AA);
	horizLine.clear();
	prevHorizLine.clear();
	keys.clear();
	prevkeys.clear();
	blackKeys.clear();
}

Keyboard::Keyboard() {
	init();
}

Keyboard::Keyboard(Images x, CvMemStorage *storage) {
	init();
	this->src = x.pInpImg;
	this->pCanny = x.pCanny;
	this->planeV = x.planeV;
	this->pGrayImg = x.pGrayImg;
	this->pKBOut= x.pKBOut;
	this->storage = storage;
	LINE_THRESHOLD = x.LINE_THRESHOLD;
	blobD = BlobDetector(x);
}

void Keyboard::addKey(CvPoint pt, std::string note, double slope, CvPoint *prevLine, CvPoint *line) {
	keyStruct newkey;
	newkey.note = note;
	newkey.pos = pt;
	newkey.slope = slope;
	newkey.line1 = prevLine;
	newkey.line2 = line;
	keys.push_back(newkey);
}

void Keyboard::addBlackKey(CvPoint pt, double slope, CvPoint *prevLine, CvPoint *line) {
	keyStruct newkey;
	newkey.note = "";
	newkey.pos = pt;
	newkey.slope = slope;
	newkey.line1 = prevLine;
	newkey.line2 = line;
	blackKeys.push_back(newkey);
}

void Keyboard::clearKeys() {
	prevHorizLine = horizLine;
	prevkeys = keys;

	blackKeys.clear();
	horizLine.clear();
	keys.clear();
}

int Keyboard::direction() {
	int noteCount = 0, i = 0, j = 0, notes = 0;
	float posThresh = 1;
	float negThresh = -posThresh;
	float sumy = 0, sumx = 0;
	int prevsize = prevkeys.size(), size = keys.size();
	for (; i < prevsize; ++i) {
		if (size > 0 && !prevkeys[i].note.compare(keys[0].note))
			break;
	}
	if (i == prevsize && prevsize > 0) {
		i = 0;
		for (; j < size; ++j) {
			if (!keys[j].note.compare(prevkeys[0].note))
				break;
		}
	}

	int ret = 0;
	CvPoint tmp;
	for (; i < prevsize && j < size; i++,j++,notes++) {
		tmp = getVector(prevkeys[i].pos, keys[j].pos);
		sumx += tmp.x;
		sumy += tmp.y;
	}

	CvScalar barColor = CV_RGB(0,255,0);
	if (notes > 0) {
		int diff;
		if (starttime - endtime) diff = starttime - endtime;
		else diff = 1;

		float test = FPS*diff*sumx/notes/1000;
		if (abs(test) > speedLimit) barColor = CV_RGB(255,255,0);
		float bar = test*5;
		if (test <= negThresh) {
			cvRectangle(src, cvPoint(WIDTH/2, 0), cvPoint(WIDTH/2+30, 30), CV_RGB(0,0,255), CV_FILLED);
			cvRectangle(src, cvPoint(WIDTH/2+30, 0), cvPoint(WIDTH/2+30-bar, 30), barColor, CV_FILLED);
			if (abs(test) > speedLimit) return 2;
			return 1;
		}
		else if (test >= posThresh) {
			cvRectangle(src, cvPoint(WIDTH/2-30, 0), cvPoint(WIDTH/2, 30), CV_RGB(0,0,255), CV_FILLED);
			cvRectangle(src, cvPoint(WIDTH/2-30-bar, 0), cvPoint(WIDTH/2-30, 30), barColor, CV_FILLED);
			if (abs(test) > speedLimit) return -2;
			return -1;
		}
		return 0;
	}
	else return 0;
}

int Keyboard::getFirstKey(std::vector<int> &wskey, char** keyNotes) {
	const unsigned int sharpNum = 5;
	char *sharpNotes[sharpNum] = {"A#","C#","D#","F#","G#"};

	unsigned int sharpPos = 0;
	unsigned int wssize = wskey.size();

	if (wssize > 1) {
		if ((wskey[1] - wskey[0]) == 3)
			sharpPos = ((EF-wskey[0])+sharpNum)%sharpNum;
		else
			sharpPos = ((BC-wskey[0])+sharpNum)%sharpNum;
	}
	else if (wssize == 1) {
		if (blackKeys.size()/2 >= wskey[0])
			sharpPos = ((EF-wskey[0])+sharpNum)%sharpNum;
		else
			sharpPos = ((BC-wskey[0])+sharpNum)%sharpNum;
	}

	for(int i = 0; i < keyNum; ++i) {
		if(!strcmp(sharpNotes[sharpPos],keyNotes[i]))
			return i;
	}

	return 0;
}

char Keyboard::octave(char* keyNotes[]) {
	int i,j;
	const char *prevKey = prevkeys[0].note.c_str();
	const char *newKey = keys[0].note.c_str();
	char octaveC = '5';

	for (i = 0; i < keyNum; ++i) {
		if (prevKey[0] == keyNotes[i][0]) {
			if (prevKey[1] == '#' && prevKey[1] == keyNotes[i][1]) {
				octaveC = prevKey[2];
				break;
			}
			else if (prevKey[2] == keyNotes[i][1]) {
				octaveC = prevKey[1];
				break;
			}
		}
	}
	for (j = 0; j < keyNum; ++j) {
		if (newKey[0] == keyNotes[j][0] && newKey[1] == keyNotes[j][1])
			break;
	}

	if (octaveC < '0' || octaveC > '9') return globOctaveC;
	if (i == j) return octaveC;

	if (i <= 5) {
		if (j > i+6) return octaveC-1;
		return octaveC;
	}
	else {
		if (j < i-6) return octaveC+1;
		return octaveC;
	}
}

void Keyboard::labelOctaves(char octave, char *keyNotes[]) {
	const char *key = keys[0].note.c_str();
	int i;
	for (i = 0; i < keyNum; ++i) {
		if (key[0] == keyNotes[i][0] && key[1] == keyNotes[i][1])
			break;
	}
	
	for (int j = 0; j < keys.size(); ++j) {
		keys[j].note.append(&octave,1);
		++i;
		if (i >= keyNum) {
			++octave;
			i = 0;
		}
	}
}

void Keyboard::labelKeys() {
	//0th is index, 1st is distance between pts
	int smallest[2];
	int largest[2];
	std::vector<int> whiteSpace;
	CvPoint prevPt;

	int key = 0;
	char *keyNotes[keyNum] = {"A","A#","B","C","C#","D","D#","E","F","F#","G","G#"};

	if ( blackKeys.size() > 1 ) {
		prevPt = blackKeys[1].pos;
		smallest[0] = largest[0] = 1;
		smallest[1] = largest[1] = getDist(blackKeys[0].pos,blackKeys[1].pos);
	}

	for (int i = 2; i < blackKeys.size(); i++) {
		int diff = getDist(blackKeys[i].pos,prevPt);
		if ( diff < smallest[1]) {
			smallest[0] = i;
			smallest[1] = diff;
		}
		if ( diff > largest[1]) {
			largest[0] = i;
			largest[1] = diff;
		}
		prevPt = blackKeys[i].pos;
	}

	if ( blackKeys.size() > 0 ) {
		prevPt = blackKeys[0].pos;
	}
	for (int i = 1; i < blackKeys.size(); i++) {
		int diff = getDist(blackKeys[i].pos,prevPt);

		///////////////////
		if (diff < 10)
			STOP = 1;
		//////////////////

		int smallSide = abs(diff - smallest[1]);
		int largeSide = abs(diff - largest[1]);

		int tmpx = (blackKeys[i].pos.x + prevPt.x)/2;
		int tmpy = (blackKeys[i].pos.y + prevPt.y)/2;

		if (smallSide > largeSide) {
			CvPoint tmpPt = cvPoint(tmpx, tmpy);
			cvLine( src, tmpPt, tmpPt, CV_RGB(0,255,255), 10, 8 );
			whiteSpace.push_back(i);
		}
		prevPt = blackKeys[i].pos;
	}

	if (whiteSpace.size() > 0) {
		int start = getFirstKey(whiteSpace, keyNotes);
		int tmpx, tmpy;
		int yOffset = 70;
		double slope, leftSlope, rightSlope;
		CvPoint *leftLine, *rightLine;
		CvPoint tmpPt;

		key = start;
		for (int i = 0; i < blackKeys.size(); i++) {
			slope = blackKeys[i].slope;
			addKey(blackKeys[i].pos,keyNotes[key],slope,blackKeys[i].line1,blackKeys[i].line2);
			key = (key + 1)%keyNum;
			double x1,x2,y;

			while ((keyNotes[key][1] == 0) && (i < blackKeys.size() - 1)) {
				leftLine = blackKeys[i].line2;
				rightLine = blackKeys[i+1].line1;
				leftSlope = getSlope(leftLine);
				rightSlope = getSlope(rightLine);
				slope = getAvgSlope(leftSlope,rightSlope);
				double ang1,ang2,avgang;
				switch(keyNotes[key][0]) {
					case 'B':
					case 'E':
						tmpPt = cvPoint(leftLine[0].x+2,leftLine[0].y);
						slope = leftSlope;
						break;
					case 'C':
					case 'F':
						tmpPt = cvPoint(rightLine[0].x-2,rightLine[0].y);
						slope = rightSlope;
						break;
					default:
						tmpPt = centerOfLines(leftLine,rightLine);
						break;
				}
				
				if (horizLine.size() > 1) {
					tmpPt = relocatePt(tmpPt,cvPoint(horizLine[0].val[0],horizLine[0].val[1]),
						slope, horizLine[0].val[2],30,'u');
				}
				else {
					tmpy += yOffset;
					tmpx += yOffset/slope;
					tmpPt = cvPoint(tmpx,tmpy);
				}
				addKey(tmpPt,keyNotes[key],slope,leftLine,rightLine);
				key = (key + 1)%keyNum;
			}
		}

		if (prevkeys.size() > 1 && 
			*(prevkeys[0].note.end()-1) >= '0' && *(prevkeys[0].note.end()-1) <= '9') {
			globOctaveC = octave(keyNotes);
		}
		labelOctaves(globOctaveC,keyNotes);

		for (int i = 0; i < keys.size(); ++i) {
			cvPutText(src, (const char *)keys[i].note.c_str(), keys[i].pos, &font, cvScalar(0, 0, 255, 0));
			cvLine( src, keys[i].pos, keys[i].pos, CV_RGB(0,255,255), 4, 8 );
		}
	}
}

void Keyboard::GetHorizLines(CvSeq* lines) {
	int y0,y1,x0,x1;
	CvRect box = blobD.keyboardRect;
	CvPoint *base, *blackbase;
	if (lines->total > 2) {
		cvSeqSort( lines, cmp_func_y, 0 );
		CvPoint *prevLine = (CvPoint*)cvGetSeqElem(lines,0);
		CvPoint *line;
		double slope = getSlope(prevLine);
		if (abs(slope) < 1) {
			horizLine.push_back(cvScalar(prevLine[0].x,prevLine[0].y, slope));
			x0 = box.x;
			x1 = box.x+box.width;
			y0 = box.y+box.height;
			y1 = slope*(x1-prevLine[0].x)+y0;
			base = prevLine;

			if ((prevLine[0].y+prevLine[0].y)/2 < box.y+3*box.height/4) {
				horizLine.clear();
				horizLine.push_back(cvScalar(x0,y0, slope));
				if (prevLine[0].y >= box.y+box.height/4 || prevLine[0].y <= box.y+3*box.height/4) {
					horizLine.push_back(cvScalar(prevLine[0].x,prevLine[0].y, slope));
				}
				else {
					horizLine.push_back(cvScalar(x0,box.y+6*box.height/10, slope));
				}
				cvLine( src, prevLine[0], prevLine[1], CV_RGB(255,255,0), 20, 8 );
				cvLine( src, cvPoint(x0,y0), cvPoint(x1,y1), CV_RGB(255,255,255), 10, 8 );
				return;
			}
		}
		else {
			horizLine = prevHorizLine;
			return;
		}

		for( int i = 1; i < lines->total; i++ ) {
			line = (CvPoint*)cvGetSeqElem(lines,i);
			slope = getSlope(line);

			if((abs(slope) < 1)) {
				if(!lineSim(line,prevLine)) {
					blackbase = line;
					horizLine.push_back(cvScalar(line[0].x,line[0].y, slope));
					if (line[0].y < box.y+box.height/4 || line[0].y > box.y+3*box.height/4) {
						//STOP = 1;
						cvLine( src, prevLine[0], prevLine[1], CV_RGB(255,255,255), 20, 8 );
						cvLine( src, line[0], line[1], CV_RGB(255,0,0), 10, 8 );

						int tmpx, tmpy, tmpx2, tmpy2,tmpslope;
						if (prevHorizLine.size() > 1) {
							tmpslope = prevHorizLine[0].val[2];
							tmpx2 = prevHorizLine[0].val[0];
							tmpy2 = prevHorizLine[0].val[1];
						}
						else {
							tmpslope = 0.00001;
							tmpx2 = 0;
							tmpy2 = 0;
						}
						tmpx = tmpx2 + sqrt((float)2500.0/(tmpslope*tmpslope+1.0));
						tmpy = tmpy2 + sqrt((float)2500.0/(1.0/tmpslope/tmpslope+1.0));
						CvPoint someline[] = {cvPoint(tmpx2,tmpy2), cvPoint(tmpx,tmpy)};

						if(lineSim(someline,prevLine) && prevHorizLine.size() > 1) {
							horizLine[1] = prevHorizLine[1];
						}
						else {
							horizLine[1].val[0] = horizLine[0].val[0];
							horizLine[1].val[1] = horizLine[0].val[1] - 4*box.height/10;
							horizLine[1].val[2] = horizLine[0].val[2];
						}

						tmpslope = horizLine[1].val[2];
						tmpx2 = horizLine[1].val[0];
						tmpy2 = horizLine[1].val[1];
						tmpx = tmpx2 + sqrt((float)2500.0/(tmpslope*tmpslope+1.0));
						tmpy = tmpy2 + sqrt((float)2500.0/(1.0/tmpslope/tmpslope+1.0));
						CvPoint someline2[] = {cvPoint(tmpx2,tmpy2), cvPoint(tmpx,tmpy)};
						cvLine( src, someline2[0], someline2[1], CV_RGB(255,255,0), 10, 8 );
						blackbase = someline2;
					}
					break;
				}
			}
			else {
				horizLine.push_back(cvScalar(line[0].x,line[0].y, slope));
				horizLine[1].val[0] = horizLine[0].val[0];
				horizLine[1].val[1] = horizLine[0].val[1] - 4*box.height/10;
				horizLine[1].val[2] = horizLine[0].val[2];
				return;
			}
		}

		cvLine( src, base[0], base[1], CV_RGB(255,255,255), 20, 8 );
		cvLine( src, blackbase[0], blackbase[1], CV_RGB(255,255,0), 10, 8 );
	}
}

void Keyboard::GetBlackKeys() {
	blobD.DetectBlobs();
	clearKeys();
	CvSeq* lines = 0;
	int prevColor = 0;
	lines = cvHoughLines2( pCanny, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, *LINE_THRESHOLD, 25, 50 );
	GetHorizLines(lines);

	cvSeqSort( lines, cmp_func_x, 0 );

	double basey = 100000;

	if (horizLine.size() > 1) {
		basey = horizLine[1].val[1];
		if (abs(basey-blobD.keyboardRect.y-blobD.keyboardRect.height) < 30) {
			STOP = 1;
		}
	}
	else {
		basey = blobD.keyboardRect.y+blobD.keyboardRect.height/2;
	}

	int i = 0;
	CvPoint *prevLine;
	for(; i < lines->total; i++ ) {
		prevLine = (CvPoint*)cvGetSeqElem(lines,i);
		double slope = getSlope(prevLine);
		double testy = (prevLine[0].y+(double)prevLine[1].y)/2;

		if((abs(slope) > 1) && (testy <= basey)) {
			break;
		}
	}
	int sum = 0, count =  0;
	
    for(; i < lines->total; i++ )
    {
        CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		double slope = getSlope(line);
		double testy = (line[0].y+(double)line[1].y)/2;

		if((abs(slope) > 1) && (testy <= basey)) {
			if(lineSim(line,prevLine)) {
				continue;
			}
			
			if (prevLine[0].x < BORDER || line[0].x > WIDTH - BORDER ||
				prevLine[1].x < BORDER || line[1].x > WIDTH - BORDER) {
				prevLine = line;
				continue;
			}
				
			int x = MIN(prevLine[0].x,prevLine[1].x);
			int y = MIN(MIN(line[0].y,line[1].y),MIN(prevLine[0].y,prevLine[1].y));
			int width = abs(MAX(line[0].x,line[1].x) - x);
			int height = MAX(MAX(line[0].y,line[1].y),MAX(prevLine[0].y,prevLine[1].y)) - y;

				
			if(y+height > basey)
				height = basey-y;

			CvPoint *lines[] = {prevLine, line};
			if(mean(lines, x, y, width, height) < 150) {

				//Check if previous area was also black
				//If so, something's wrong, since
				//black keys are not adjacent
				if (prevColor == 1) {
					keys = prevkeys;
					for (int i = 0; i < keys.size(); ++i) {
						cvPutText(src, (const char *)keys[i].note.c_str(), 
							keys[i].pos, &font, cvScalar(0, 0, 255, 0));
					}
					return;
				}
				prevColor = 1;

				double prevSlope = getSlope(prevLine);
				double avgSlope = getAvgSlope(prevSlope,slope);
				CvPoint tmpPt = centerOfLines(prevLine,line);
				if (horizLine.size() > 1) {
					tmpPt = relocatePt(tmpPt,cvPoint(horizLine[1].val[0],horizLine[1].val[1]),
						avgSlope, horizLine[1].val[2],20,'u');
				}

				addBlackKey(tmpPt, avgSlope,prevLine,line);
				count++;
			}
			else prevColor = 0;

			prevLine = line;
		}
        else if (abs(slope) <= 1){
			cvLine( src, line[0], line[1], CV_RGB(255,0,0), 2, 8 );
		}

    }

	labelKeys();
	direction();
}