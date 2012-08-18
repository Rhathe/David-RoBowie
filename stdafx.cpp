// stdafx.cpp : source file that includes just the standard includes
// 537Project.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

long starttime;
long endtime;

std::string printPts(CvPoint pt[], int num) {
	std::stringstream s;
	std::string str, str2;
	for (int i = 0; i < num; ++i) {
		s << "(" << pt[i].x << "," << pt[i].y << ") ";
		s >> str2;
		str += str2;
	}
	return str;
}

CvPoint getVector(CvPoint &p1, CvPoint &p2) {
	return cvPoint(p2.x-p1.x, p2.y-p1.y);
}

CvPoint getVector(CvPoint &p1, CvPoint &p2, double slope2) {
	double slope1 = -1/slope2;
	CvPoint intersect = intersectLine(p1,p2,slope1,slope2);
	return cvPoint(p1.x-intersect.x,p1.y-intersect.y);
}

int getDist(CvPoint &p1, CvPoint &p2) {
	float x = abs(p1.x-p2.x);
	float y = abs(p1.y-p2.y);
	return sqrt(x*x+y*y);
}

CvPoint centerOfLines(CvPoint *line1, CvPoint *line2) {
	CvPoint a, b, c, d;
	if (line1[0].y < line1[1].y) std::swap(line1[0],line1[1]);
	if (line2[0].y < line2[1].y) std::swap(line2[0],line2[1]);

	a = cvPoint((line1[0].x + line1[1].x)/2,(line1[0].y + line1[1].y)/2);
	b = cvPoint((line2[0].x + line2[1].x)/2,(line2[0].y + line2[1].y)/2);
	c = cvPoint((line1[0].x + line2[0].x)/2,(line1[0].y + line2[0].y)/2);
	d = cvPoint((line1[1].x + line2[1].x)/2,(line1[1].y + line2[1].y)/2);

	CvPoint nline1[] = {a,b}, nline2[] = {c,d};
	return intersectLine(nline1,nline2);
}

CvPoint intersectLine(CvPoint *line1, CvPoint *line2) {
	double m1 = getSlope(line1);
	double m2 = getSlope(line2);

	int x = (m1*line1[0].x-line1[0].y-m2*line2[0].x+line2[0].y)/(m1-m2);
	int y = (line1[0].y/m1-line1[0].x-line2[0].y/m2+line2[0].x)/(1/m1-1/m2);

	return cvPoint(x,y);
}

CvPoint intersectLine(CvPoint pt1, CvPoint pt2, double m1, double m2) {
	int x = (m1*pt1.x-pt1.y-m2*pt2.x+pt2.y)/(m1-m2);
	int y = (pt1.y/m1-pt1.x-pt2.y/m2+pt2.x)/(1/m1-1/m2);

	return cvPoint(x,y);
}

CvPoint getPointFromDist(CvPoint pt, int distance, double slope, char direction) {

	int dirx = -1, diry = -1;
	if (direction == 'd') {
		diry *= -1;
		dirx *= -1;
	}
	if (slope < 0)
		dirx *= -1;

	int x = pt.x + dirx*distance/sqrt(slope*slope+1);
	int y = pt.y + diry*distance/sqrt(1/(slope*slope)+1);;

	return cvPoint(x,y);
}

CvPoint getPointFromDist(CvPoint pt, int distance, double slope, CvPoint destpt) {

	int dirx = -1, diry = -1;
	if (destpt.y < pt.y) {
		diry *= -1;
		dirx *= -1;
	}
	if (slope < 0)
		dirx *= -1;

	int x = pt.x + dirx*distance/sqrt(slope*slope+1);
	int y = pt.y + diry*distance/sqrt(1/(slope*slope)+1);;

	return cvPoint(x,y);
}

CvPoint relocatePt(CvPoint pt1, CvPoint pt2, double slope1, double slope2, int distance,char direction) {

	CvPoint intersection = intersectLine(pt1,pt2,slope1,slope2);
	return getPointFromDist(intersection,distance,slope1,direction);
}

double getSlope(CvPoint* line) {
	double slope;
	double run = line[0].x-line[1].x;
	double rise = line[0].y-(double)line[1].y;
	if (!run)
		run = 0.00001;
	if (!rise)
		rise = 0.00001;

	return rise/run;
}

double getAvgSlope(double slope1, double slope2) {
	double ang1, ang2, PI = atan(1.0)*4;
	if ((ang1 = atan(slope1)) < 0) ang1 += PI;
	if ((ang2 = atan(slope2)) < 0) ang2 += PI;

	double avgang = (ang1+ang2)/2;
	double avgslope = tan(avgang);

	if (avgslope == 0) avgslope = 0.00001;

	return avgslope;
}

int lineSim(CvPoint* line1, CvPoint* line2) {
	CvPoint *smallLine, *largeLine;
	double d1, d2;
	double x1,x2,y1,y2;
	x1 = abs(line1[0].x-line1[1].x) << 1;
	y1 = abs(line1[0].y-line1[1].y) << 1;
	x2 = abs(line2[0].x-line2[1].x) << 1;
	y2 = abs(line2[0].y-line2[1].y) << 1;

	d1 = sqrt(x1 + y1);
	d2 = sqrt(x2 + y2);

	if (d1 < d2) {
		smallLine = line1;
		largeLine  = line2;
	}
	else {
		smallLine = line2;
		largeLine  = line1;
	}

	double slope = getSlope(largeLine);

	if (abs(slope) > 1) {
		int y1 = smallLine[0].y;
		int y2 = smallLine[1].y;

		int x1 = largeLine[0].x+(y1-largeLine[0].y)/slope;
		int x2 = largeLine[1].x+(y2-largeLine[1].y)/slope;

		float test = WIDTH/32.0;
		if ((abs(x1-smallLine[0].x) < test) && (abs(x2-smallLine[1].x) < test)) {
			return 1;
		}
		else
			return 0;
	}
	else {
		int x1 = smallLine[0].x;
		int x2 = smallLine[1].x;

		int y1 = largeLine[0].y+(x1-largeLine[0].x)*slope;
		int y2 = largeLine[1].y+(x2-largeLine[1].x)*slope;

		float test = WIDTH/32.0;
		if ((abs(y1-smallLine[0].y) < test) && (abs(y2-smallLine[1].y) < test)) {
			return 1;
		}
		else
			return 0;
	}
}

int cmp_func_x( const void* _a, const void* _b, void* userdata )
{
    CvPoint* a = (CvPoint*)_a;
    CvPoint* b = (CvPoint*)_b;
	int x_diff = (a[0].x < a[1].x ? a[1].x : a[0].x) - (b[0].x < b[1].x ? b[1].x : b[0].x);
    return x_diff;
}

int cmp_func_y( const void* _a, const void* _b, void* userdata )
{
    CvPoint* a = (CvPoint*)_a;
    CvPoint* b = (CvPoint*)_b;

	if (abs(getSlope(a)) > 1)
		return 1;
	if (abs(getSlope(b)) > 1)
		return -1;

	int y_diff = (b[0].y > b[1].y ? b[1].y : b[0].y) - (a[0].y > a[1].y ? a[1].y : a[0].y);
    return y_diff;
}

void insSort(CvPoint arr[], int n) {
	int i,j;
	for (i = 1; i < n; ++i) {
		CvPoint key = arr[i];
		for(j = i - 1; j >= 0 && key.x < arr[j].x; j--) {
			arr[j+1] = arr[j];
		}
		arr[j+1] = key;
	}
}


// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
