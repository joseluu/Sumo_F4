#pragma once
#ifndef _ACTION_H
#define _ACTION_H

#ifdef __cplusplus
extern "C"{
#endif
	void mainLoop(void);
#ifdef __cplusplus
}


#include <queue>

typedef  enum { 
	Stop,
	Fwd,
	Back,
	Right,
	Left,
	Seek,
	Start,
	Last
 } Move;

class Action {
public: 
	Move move;
	int time_ms;

	Action()
		: move(Stop)
		, time_ms(0) {}
	;
	Action(Move move, int time_ms) : move( move), time_ms(time_ms)
	{
	}
	;
};

extern  std::queue<Action > todos;

void do_startButton(void);

void onFrontEdgeDetect(bool bIsOutRight, bool bIsOutLeft);
void doWakeup0(void);
void doWakeup1(void);
void doWakeup2(void);
void doWakeup3(void);

#endif // __cplusplus
#endif

