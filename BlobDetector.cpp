#include "stdafx.h"
#include "BlobDetector.h"

void BlobDetector::init() {
	storage = cvCreateMemStorage(0);
	approxPoly = cvCreateMemStorage(0);
	hueThresh = 100;
	satThresh = 95;
	valThresh = 100;
	windowRect = cvRect(0, 0, WIDTH, HEIGHT);
	keyboardRect = cvRect(0, 0, WIDTH, HEIGHT);
}

BlobDetector::BlobDetector() {
	init();
}

BlobDetector::BlobDetector(Images x) {
	init();
	src = x.pInpImg;
	pHSV = x.pHSV;
	planeH = x.planeH;
	planeS = x.planeS;
	planeV = x.planeV;
	pCanny = x.pCanny;
	pCannySrc = x.pCannySrc;
	pGrayImg = x.pGrayImg;
	pKBOut = x.pKBOut;
	KB_THRESHOLD = x.KB_THRESHOLD;
}

void BlobDetector::FindFingerTips() {
	//Find contours for FingerTips
	cvClearMemStorage(storage);
	cvFindContours( planeV, storage, &contours, sizeof(CvContour),
                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	CvSeq* contourIter = contours;
	CvScalar color = CV_RGB( 255, 0, 255 );
	
	fingNum = 0;
	for( ; contourIter != 0; contourIter = contourIter->h_next )
	{
		cvDrawContours( planeV, contourIter, color, color, 0, 1, 8 );
		CvMoments moments; 
		double M00, M01, M10;

		cvMoments(contourIter,&moments); 
		M00 = cvGetSpatialMoment(&moments,0,0); 
		M10 = cvGetSpatialMoment(&moments,1,0); 
		M01 = cvGetSpatialMoment(&moments,0,1); 
		fingers[fingNum] = cvPoint((int)(M10/M00), (int)(M01/M00));
		cvLine( planeV, fingers[fingNum], fingers[fingNum], CV_RGB(0,255,255), 2, 8 );
		cvLine( src, fingers[fingNum], fingers[fingNum], CV_RGB(255,255,255), 2, 8 );
		fingNum++;
	}
	//CvFont font;
	//cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX_SMALL, 0.5, 0.5, 0, 1, CV_AA);
	insSort(fingers,fingNum);
	//cvPutText(planeV, "#1", fingers[0], &font, cvScalar(255,255,255,255));
	cvClearMemStorage(storage);
}

void BlobDetector::FindFingers() {
	//Find contours for Fingers
	cvFindContours( planeV, storage, &contours, sizeof(CvContour),
                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	CvSeq* contourIter = contours;
	int boxExtend = 5;
	CvScalar color = CV_RGB( 255, 0, 255 );

	//Clear planeV
	cvResetImageROI(planeV);
	cvRectangle( planeV, cvPoint(0,0), cvPoint(WIDTH,HEIGHT), cvScalar(0), CV_FILLED);
	cvSetImageROI(planeV, keyboardRect);
	
	for( ; contourIter != 0; contourIter = contourIter->h_next )
	{
		if (cvContourArea(contourIter) > 200) {
			cvDrawContours( planeV, contourIter, color, color, 0, CV_FILLED, 8 );
			CvRect box = cvBoundingRect(contourIter,1);
			CvRect biggerBox = cvRect(box.x-boxExtend,box.y-boxExtend,
				box.width+2*boxExtend,box.height+2*boxExtend);
			cvRectangle( pCanny, cvPoint(biggerBox.x,biggerBox.y), 
					cvPoint(biggerBox.x+biggerBox.width,biggerBox.y+biggerBox.height), 
					cvScalar(0), CV_FILLED);
			cvRectangle( src, cvPoint(biggerBox.x,biggerBox.y), 
					cvPoint(biggerBox.x+biggerBox.width,biggerBox.y+biggerBox.height), 
					cvScalar(0,255,0,0), 1);
			cvRectangle( src, cvPoint(box.x,box.y), 
					cvPoint(box.x+box.width,box.y+box.height), 
					cvScalar(0,255,0,0), 1);
			
			minRect = cvMinAreaRect2(contourIter);
			minRect.size.width+=3;
			minRect.size.height+=3;
			CvPoint2D32f tmp_points[4];
			cvBoxPoints( minRect, (CvPoint2D32f *) tmp_points );
			
			int y = 5000;
			int highestInd = 0;
			for( int j = 0; j < 4; j++ ) {
				rect_points[j] = cvPointFrom32f(tmp_points[j]);
				if (rect_points[j].y < y) {
					highestInd = j;
					y = rect_points[j].y;
				}
			}

			int smallSide = 5000, smallInd = 0;
			int largeSide = 5000, largeInd = 0;
			int oppInd = 0;

			for( int j = 0; j < 4; j++ ) {
				if (j == highestInd) continue;
				int dist = getDist(rect_points[j],rect_points[highestInd]);
				if (dist < smallSide && dist < largeSide) {
					largeSide = smallSide;
					largeInd = smallInd;
					smallSide = dist;
					smallInd = j;
				}
				else if (dist < largeSide) {
					largeSide = dist;
					largeInd = j;
				}
				else oppInd = j;
			}

			//cvLine( src, rect_points[highestInd], rect_points[highestInd], CV_RGB(255,255,255), 20, 8 );
			//cvLine( src, rect_points[smallInd], rect_points[smallInd], CV_RGB(255,255,255), 10, 8 );
			CvPoint line[] = {rect_points[highestInd],rect_points[largeInd]};
			double slope = getSlope(line);
			if (rect_points[highestInd].y == rect_points[largeInd].y &&
				rect_points[highestInd].x < rect_points[largeInd].x)
				slope *= -1;

			rect_points[highestInd]=getPointFromDist(rect_points[highestInd], 15, slope, 'd');
			rect_points[smallInd]=getPointFromDist(rect_points[smallInd], 15, slope, 'd');
			rect_points[largeInd]=getPointFromDist(rect_points[largeInd], 15, slope, 'd');
			rect_points[oppInd]=getPointFromDist(rect_points[oppInd], 15, slope, 'd');
			cvFillConvexPoly( planeV, rect_points, 4, cvScalar(0));
		}
	}
	cvClearMemStorage(storage);
}

void BlobDetector::DetectBlobs() {
	CvSeq* contourIter;
	CvScalar color = CV_RGB( 255, 0, 255 );

	//Set Up Threshhold of Gray Image
	cvCvtColor(src, pGrayImg, CV_RGB2GRAY);
	//cvEqualizeHist( pGrayImg, pGrayImg );
	cvThreshold(pGrayImg, pKBOut, *KB_THRESHOLD, 255, CV_THRESH_BINARY);
	cvDilate( pKBOut, pKBOut, 0, 1 );
	cvErode( pKBOut, pKBOut, 0, 1 );
	cvRectangle( pCanny, cvPoint(0,0), cvPoint(WIDTH,HEIGHT), cvScalar(0), CV_FILLED);

	//Get HSV from RGB picture
	cvCvtColor(src, pHSV, CV_BGR2HSV);
	cvSmooth( pHSV, pHSV, CV_MEDIAN, 7, 7 );
	cvCvtPixToPlane(pHSV, planeH, planeS, planeV, 0);

	//Threshold individual HSV channels
	cvThreshold(planeH, planeH, hueThresh, UCHAR_MAX, CV_THRESH_BINARY);
	cvThreshold(planeS, planeS, satThresh, UCHAR_MAX, CV_THRESH_BINARY);
	cvThreshold(planeV, planeV, valThresh, UCHAR_MAX, CV_THRESH_BINARY_INV);
	
	cvAnd(planeS, planeV, planeV);
	cvAnd(planeH, planeV, planeV);

	//Find contours for Keyboard
	IplImage* tmpImg = cvCloneImage(pKBOut);
	cvFindContours( tmpImg, storage, &contours, sizeof(CvContour),
                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	cvReleaseImage(&tmpImg);
	contourIter = contours;
	
	bool gettingKeyboardRect = false;
	for( ; contourIter != 0; contourIter = contourIter->h_next )
	{
		CvRect box = cvBoundingRect(contourIter,1);
		if (box.width*box.height > WIDTH*HEIGHT/200) {
			if (gettingKeyboardRect == false) {
				gettingKeyboardRect = true;
				keyboardRect = box;
			}
			else {
				keyboardRect = cvMaxRect(&keyboardRect,&box);
			}
		}
	}
	cvRectangle( src, cvPoint(keyboardRect.x,keyboardRect.y),
				cvPoint(keyboardRect.x+keyboardRect.width,keyboardRect.y+keyboardRect.height), color, 2);

	cvSetImageROI(pKBOut, keyboardRect);
	cvSetImageROI(pGrayImg, keyboardRect);
	cvSetImageROI(pCannySrc, keyboardRect);
	cvSetImageROI(pCanny, keyboardRect);
	cvSetImageROI(planeH, keyboardRect);
	cvSetImageROI(planeS, keyboardRect);
	cvSetImageROI(planeV, keyboardRect);
	cvSetImageROI(src, keyboardRect);

	//Canny Image
	cvCanny(pKBOut, pCanny, 50, 240, 3);
	cvCanny(pGrayImg, pCannySrc, 50, 240, 3);
	cvDilate( pCannySrc, pCannySrc, 0, 1 );
	//cvAnd(pCanny,pCannySrc,pCanny);

	FindFingers();

	cvResetImageROI(pKBOut);
	cvResetImageROI(pCanny);
	cvResetImageROI(pCannySrc);
	cvResetImageROI(pGrayImg);
	cvResetImageROI(planeH);
	cvResetImageROI(planeS);
	cvResetImageROI(planeV);
	cvResetImageROI(src);

	FindFingerTips();
}