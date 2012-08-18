#ifndef PLAYA_H_
#define PLAYA_H_

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include "Keyboard.h"
#include "SerialClass.h"

#define MOVESIZEMAX 3
#define MOVERLIMIT 150
#define MOVELLIMIT 40
#define MOVEULIMIT 105
#define MOVEDLIMIT 130
#define NOSIDE -5000

class Playa {
	
	public:
		struct note {
			std::string key;
			long beat;
			long nexttime;
			note() {};
			note(std::string key, long beat, long nexttime) {
				this->key = key;
				this->beat = beat;
				this->nexttime = nexttime;
				if (nexttime < 0) this->nexttime = beat+50;
			};
		};
		Playa();
		Playa(Keyboard &keyboard, Serial *ardSerial);
		Keyboard::keyStruct getPosFromNote(std::string note, int finger);
		void readjustDest(int finger);
		void move();
		void moveFinger(int finger, int destind);
		void pressFinger(int finger);
		void unpressFinger(int finger);
		void movePlatform();
		void calibrate();
		void setFinger(int finger);
		int moveSize[4];
		int fingHeight;
		long nextNoteTime;
		Keyboard *keyboard;
		CvPoint camCent;

	private:
		void init();
		Serial *ardSerial;
		char updown[3];
		char ltor[3];
		char currPos[4];
		char rotServ[4];
		char pressServ[4];
		char driveltor[2];
		double press[4];
		int pressCount[4];
		long starttime[4];
		long endtime[4];
		int borders[4][2];
		int direction;
		bool canPress[4], canMove[4];
		Keyboard::keyStruct dest[4];
		std::vector<note> notes;
		int currNote;
		int prevSide[4];
};


#endif /*PLAYA_H_ */