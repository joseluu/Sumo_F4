#pragma once
#ifndef _MOTORCONTROL_H
#define _MOTORCONTROL_H

#include "stm32f4xx_hal.h"


#define RIGHT 1
#define LEFT 2

class Motor {
private:
	bool bForward;
	int command;
	uint32_t pin;

public:
	void drive(int speed);
	void drive(int rightOrLeft, int speed);
};

extern Motor motor;
#endif