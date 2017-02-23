#pragma once



#define FRONT_RIGHT 0
#define FRONT_LEFT 1
#define BACK_RIGHT 2
#define BACK_LEFT 3

#define FRONT_RIGHT_RADAR  4
#define FRONT_CENTER_RADAR 0
#define FRONT_LEFT_RADAR 5
#define RIGHT_RADAR 1
#define LEFT_RADAR 2
#define BACK_RADAR 3

#define NUM_RADAR 6

extern volatile float radarDistances[NUM_RADAR];


void do_startButton();


#ifdef __cplusplus

extern volatile bool OutDetect[4];
extern "C" {
#endif


void doWakeup(int which);

void do_initializeControl();

void do_stepInLoop();

	void do_radarDetect(int n, unsigned int time);

	void do_frontEdgeDetect(int leftOrRight);

	void do_backEdgeDetect(int leftOrRight);

#ifdef __cplusplus
}
#endif

