#pragma once


const int notStarted = 0;
const int startedDelaying = 1;
const int seeking = 2;
const int located = 3;
const int bulling = 4;
const int running = 5;
const int adjust = 6;
const int adjust2 = 7;
const int adjust1 = 8;
const int backWhite = 9;
const int backRecover = 10;
const int		countStates = 9;


typedef int State;


#define RIGHT 1
#define LEFT 2

#define FRONT_RIGHT_RADAR  4
#define FRONT_CENTER_RADAR 0
#define FRONT_LEFT_RADAR 5
#define RIGHT_RADAR RIGHT
#define LEFT_RADAR LEFT
#define BACK_RADAR 3

#define NUM_RADAR 6


#ifdef __cplusplus
extern "C" {
#endif

extern	volatile State state[4];

void doWakeup(int which);

void do_startButton();

void do_initializeControl();

void do_stepInLoop();

	void do_radarDetect(int n, unsigned int time);

	void do_frontEdgeDetect(int leftOrRight);

	void do_backEdgeDetect(int leftOrRight);

#ifdef __cplusplus
}
#endif

