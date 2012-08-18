#include "stdafx.h"
#include "Playa.h"

void Playa::init() {
	updown[0] = 'm';
	updown[1] = 8;
	updown[2] = 100;
	ltor[0] = 'm';
	ltor[1] = 9;
	ltor[2] = 90;

	rotServ[0] = 9;
	rotServ[1] = 7;
	pressServ[0] = 8;
	pressServ[1] = 6;

	driveltor[0] = '2';
	driveltor[1] = 255;
	int section = WIDTH/4;
	for (int i = 0; i < 4; ++i) {
		press[i] = 90;
		currPos[i] = 90;
		pressCount[i] = 0;
		starttime[i] = 0;
		endtime[i] = 0;
		canPress[i] = false;
		canMove[i] = true;
		moveSize[i] = MOVESIZEMAX;
		prevSide[i] = NOSIDE;
		borders[i][0] = section*i;
		borders[i][1] = section*(i+1);
	}
	borders[0][0] = 15;
	borders[0][1] = 2*WIDTH/6+10;
	borders[1][0] = borders[0][1]+30;
	borders[1][1] = WIDTH-10;
	direction = 0;
	notes.clear();
	notes.push_back(note("D6",100,-1));
	notes.push_back(note("B6",100,-1));
	notes.push_back(note("A6",400,-1));
	notes.push_back(note("D5",100,-1));
	notes.push_back(note("B5",100,-1));
	notes.push_back(note("A5",400,-1));
	notes.push_back(note("D7",100,-1));
	notes.push_back(note("B7",100,-1));
	notes.push_back(note("A7",200,-1));
	notes.push_back(note("B7",200,-1));
	notes.push_back(note("A7",400,-1));
	currNote = 0;
	nextNoteTime = -1;
	fingHeight = 7;
	camCent = cvPoint(WIDTH/2,HEIGHT/2);
}

Playa::Playa() {
	init();
}

Playa::Playa(Keyboard &keyboard, Serial *ardSerial) {
	this->keyboard = &keyboard;
	this->ardSerial = ardSerial;
	init();
}

Keyboard::keyStruct Playa::getPosFromNote(std::string note, int finger) {
	Keyboard::keyStruct pos(cvPoint(BORDER,0),note,1000);

	if (keyboard->keys.size() > 3) {
		int i = 0;
		int size = keyboard->keys.size();
		for (; i < size; ++i) {
			if (!keyboard->keys[i].note.compare(note)) {
				pos = keyboard->keys[i];
				break;
			}
		}
		std::string s = keyboard->keys[size-1].note;
		if (s[0] + s[s.length()-1]*20 < note[0] + note[note.length()-1]*20) {
			pos.pos.x = WIDTH-BORDER;
		}
	}

	CvScalar color;
	if (finger == 0) color = CV_RGB(0,255,0);
	else color = CV_RGB(0,100,0);

	cvLine( keyboard->src, getPointFromDist(pos.pos, 300, pos.slope, 'u'), 
		getPointFromDist(pos.pos, 300, pos.slope, 'd'), color, 1, 8 );

	CvPoint otherDest = intersectLine(cvPoint(0,fingHeight), pos.pos, 0.00001, pos.slope);
	CvPoint vpline[] = {camCent, otherDest};
	CvPoint higherDest = intersectLine(camCent, cvPoint(0,0), getSlope(vpline), 0.00001);

	cvLine( keyboard->src, higherDest, 
		getPointFromDist(higherDest, 300, pos.slope, 'd'), color, 2, 8 );

	pos.pos = higherDest;
	return pos;
}

void Playa::readjustDest(int destind) {
	int size = WIDTH+HEIGHT;
	CvPoint moveTopMid = keyboard->blobD.fingers[0];
	for (int i = 0; i < keyboard->blobD.fingNum; ++i) {
		CvPoint tmp = keyboard->blobD.fingers[i];
		int tmpsize = getDist(cvPoint(0,0),getVector(tmp,dest[destind].pos,dest[destind].slope));
		if (tmpsize < size) {
			size = tmpsize;
			moveTopMid = tmp;
		}
	}
	CvPoint vect = getVector(moveTopMid,dest[destind].pos,dest[destind].slope);
	dest[destind].pos.x = moveTopMid.x-vect.x;
	dest[destind].pos.y = moveTopMid.y-vect.y;
}

void Playa::moveFinger(int finger, int destind) {
	if (canMove[finger] == true) {
		ltor[1] = rotServ[finger];
		CvPoint moveTopMid = keyboard->blobD.fingers[finger];
		CvPoint vect = getVector(moveTopMid,dest[destind].pos,dest[destind].slope);
		int dist = getDist(vect,cvPoint(0,0));
		if (dist < 20) moveSize[finger] = 1;
		else if (dist < 35) moveSize[finger] = 2;
		else moveSize[finger] = MOVESIZEMAX;

		int side = vect.x;
		CvPoint test = cvPoint(moveTopMid.x-vect.x,moveTopMid.y-vect.y);
		cvLine( keyboard->src, dest[destind].pos, dest[destind].pos, CV_RGB(0,255,0), 10, 8 );
		if ((dist < 2 || side*prevSide[finger] < 0) && (nextNoteTime == -1) && (destind == 0)) {
			if (prevSide[finger] != NOSIDE) {
				canMove[finger] = false;
				canPress[finger] = true;
				starttime[finger] = GetTickCount();
				pressCount[finger] = notes[currNote].beat;
				nextNoteTime = starttime[finger];
			}
		}
		else if(side > 0) {
			currPos[finger] -=moveSize[finger];
			ltor[2] = currPos[finger];
		}
		else {
			currPos[finger] +=moveSize[finger];
			ltor[2] = currPos[finger];
		}
		if (finger == 0) {
			if (currPos[finger] > 115) currPos[finger] = 115;
			else if (currPos[finger] < 65) currPos[finger] = 65;
		}
		else {
			if (currPos[finger] > 115) currPos[finger] = 115;
			else if (currPos[finger] < 50) currPos[finger] = 50;
		}
		ardSerial->WriteData(ltor,3);
		prevSide[finger] = side;
	}
}

void Playa::setFinger(int finger) {
	if (canMove[finger] == true) {
		updown[1] = pressServ[finger];
		press[finger] = MOVEULIMIT;
		updown[2] = press[finger];
		ardSerial->WriteData(updown,3);
	}
}

void Playa::pressFinger(int finger) {
	if (canPress[finger] == true) {
		updown[1] = pressServ[finger];
		char stop = '2';
		ardSerial->WriteData(&stop,1);
		press[finger] = MOVEDLIMIT;

		updown[2] = press[finger];
		ardSerial->WriteData(updown,3);
		prevSide[finger] = NOSIDE;
	}
}

void Playa::unpressFinger(int finger) {
	if (press[finger] >= MOVEDLIMIT) {
		updown[1] = pressServ[finger];
		if ((GetTickCount() - starttime[finger]) > pressCount[finger]) {
			starttime[finger] = 0;
			press[finger] = MOVEULIMIT;
			canPress[finger] = false;
			canMove[finger] = true;
		}
	}
	updown[1] = pressServ[finger];
	updown[2] = press[finger];
	ardSerial->WriteData(updown,3);
}

void Playa::movePlatform() {

	for (int i = 0; i < 4; ++i) {
		if (press[i] >= MOVEDLIMIT) return;
	}

	switch(direction) {
		case -1: 
			driveltor[0] = '0'; 
			ardSerial->WriteData(driveltor,2); 
			prevSide[0] = prevSide[1] = prevSide[2] = prevSide[3] = NOSIDE;
			break;
		case 1: 
			driveltor[0] = '1'; 
			ardSerial->WriteData(driveltor,2); 
			prevSide[0] = prevSide[1] = prevSide[2] = prevSide[3] = NOSIDE;
			break;
		case 2:
		case 0: 
			driveltor[0] = '2'; 
			ardSerial->WriteData(driveltor,1);
			break;
	}
}

void Playa::move() {
	CvScalar color0 = cvScalar(0,0,255);
	CvScalar color1 = cvScalar(0,0,255);
	int finger = 0;
	if (keyboard->keys.size() <= 3) return;

	dest[0] = getPosFromNote(notes[currNote].key,0);
	readjustDest(0);
	int nextnote = (currNote + 1)%notes.size();
	dest[1] = getPosFromNote(notes[nextnote].key,1);
	readjustDest(1);
	
	int lookAtDirection = keyboard->direction();

	if (abs(lookAtDirection) == 2) direction = 2;
	else if (dest[0].pos.x <= borders[0][0]) direction = -1;
	else if (dest[0].pos.x > borders[1][1]) direction = 1;
	else if (dest[0].pos.x > borders[0][1] && dest[0].pos.x <= borders[1][0]) {
		direction = lookAtDirection;
		if (direction  == 0) direction = 1;
	}
	else {
		direction = 0;
		if (dest[0].pos.x > borders[0][0] && dest[0].pos.x <= borders[0][1]) {
			color0 = cvScalar(0,255,0);
			finger = 0;
		}
		else {
			color1 = cvScalar(0,255,0);
			finger = 1;
		}
	}

	cvLine( keyboard->src, cvPoint(borders[0][0],0), cvPoint(borders[0][0],HEIGHT),color0, 1, 8 );
	cvLine( keyboard->src, cvPoint(borders[0][1],0), cvPoint(borders[0][1],HEIGHT),color0, 1, 8 );
	cvLine( keyboard->src, cvPoint(borders[1][0]+2,0), cvPoint(borders[1][0]+2,HEIGHT),color1, 1, 8 );
	cvLine( keyboard->src, cvPoint(borders[1][1],0), cvPoint(borders[1][1],HEIGHT),color1, 1, 8 );
	movePlatform();

	if (direction == 0 && !lookAtDirection) {
		setFinger(0);
		setFinger(1);
		moveFinger(finger, 0);
		pressFinger(finger);
	}

	if ((nextNoteTime != -1) && (GetTickCount() - nextNoteTime > notes[currNote].nexttime)) {
		currNote = (currNote + 1)%notes.size();
		nextNoteTime = -1;
		for (int i = 0; i < 4; ++i) prevSide[i] = NOSIDE; 
	}
	unpressFinger(0);
	unpressFinger(1);
}