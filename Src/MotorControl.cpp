#include "MotorControl.h"
#include "tim.h"


void setWheelDirection(int wheel,bool forward, bool backward)
{
	if (forward && backward) {
		return; // error
	}
	/* pin drecription from mxconstants.h
#define T1_MOTOR1_D6_Pin GPIO_PIN_10
#define T1_MOTOR1_D6_GPIO_Port GPIOB
#define T2_MOTOR2_D9_Pin GPIO_PIN_7
#define T2_MOTOR2_D9_GPIO_Port GPIOC
#define T2_MOTOR1_D7_Pin GPIO_PIN_8
#define T2_MOTOR1_D7_GPIO_Port GPIOA
#define T1_MOTOR2_D8_Pin GPIO_PIN_9
#define T1_MOTOR2_D8_GPIO_Port GPIOA
*/

	if (wheel == RIGHT) {
		HAL_GPIO_WritePin(T1_MOTOR1_D6_GPIO_Port, T1_MOTOR1_D6_Pin, forward ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(T2_MOTOR1_D7_GPIO_Port, T2_MOTOR1_D7_Pin, backward ? GPIO_PIN_SET : GPIO_PIN_RESET);
	} else if (wheel== LEFT){
		HAL_GPIO_WritePin(T1_MOTOR2_D8_GPIO_Port, T1_MOTOR2_D8_Pin, forward ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(T2_MOTOR2_D9_GPIO_Port, T2_MOTOR2_D9_Pin, backward ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

void setWheelSpeed(int wheel, int speed)
{
	if (speed > 1000) {
		speed = 1000;
	} else if (speed < 0) {
		speed = 0; // likely an error
	}
	if (wheel == RIGHT) {
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, speed);
	} else if (wheel == LEFT) {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed);
	}
}


void Motor::drive(int speed)
{
	if (speed > 0) {
		setWheelDirection(RIGHT, 1, 0);
		setWheelDirection(LEFT, 1, 0);
		setWheelSpeed(RIGHT, speed);
		setWheelSpeed(LEFT, speed);
	} else if (speed == 0) {
		setWheelDirection(RIGHT, 0, 0);
		setWheelDirection(LEFT, 0, 0);
		setWheelSpeed(RIGHT, speed);
		setWheelSpeed(LEFT, speed);
	} else if (speed < 0) {
		setWheelDirection(RIGHT, 0, 1);
		setWheelDirection(LEFT, 0, 1);
		setWheelSpeed(RIGHT, -speed);
		setWheelSpeed(LEFT, -speed);
	}
}

void Motor::drive(int rightOrLeft, int speed)
{
	if (rightOrLeft == 0) {
		drive(speed);
	} else if (rightOrLeft == LEFT) {
		setWheelDirection(RIGHT, 1, 0);
		setWheelDirection(LEFT, 0, 1);
		setWheelSpeed(RIGHT, speed/1.5);
		setWheelSpeed(LEFT, speed);
	} else if (rightOrLeft == RIGHT) {
		setWheelDirection(RIGHT, 0, 1);
		setWheelDirection(LEFT, 1, 0);
		setWheelSpeed(RIGHT, speed);
		setWheelSpeed(LEFT, speed/1.5);
	}
}

