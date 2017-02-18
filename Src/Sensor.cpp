#include "Control.h"
#include "MotorControl.h"
#include "tim.h"


// module variables
volatile State state[4] = {notStarted,0,0,0};
State nextState[4];
Motor motor;

volatile float radarDistances[NUM_RADAR];
const int medianSize = 11;
volatile char dState[20] = "";

// forward definitions
void schedWakeup(int which, int ms, State next);
void do_action0(State next);
void do_action1(State next);
void do_action2(State next);
void do_action3(State next);
void timedDrive(int ms, int speed);
void timedDrive(int ms, int rightOrLeft, int speed);
bool isOutFront(int rightOrLeft = 0);
bool isOutBack(int rightOrLeft = 0);
void setDebugState(const char* state);
void displayOutFront(void);

// code

void do_action0( State next)
{
	state[0] = next;
	if (next == seeking) {
		timedDrive(2000, RIGHT,400);
	} else if (next == backRecover) {
		timedDrive(600, -200);

	}
	if (!isOutFront()){
		//timedDrive(500, 300);
	}
}


void do_startButton()
{
	state[0] = startedDelaying;
	schedWakeup(0, 1000, seeking);
}

void doWakeup(int which){
	State next = nextState[which];
	switch (which) {
	case 0:
		do_action0(next);
		break;
	case 1:
		do_action1(next);
		break;
	case 2:
		do_action2(next);
		break;
	case 3:
		do_action3(next);
		break;
	//default:
	// error
	}
}




volatile unsigned int medianTime = 0;
class MedianEntry {
public:
	unsigned int number;
	float distance;

	MedianEntry(unsigned int number, float distance): number(number), distance(distance)
	{}
	;
	MedianEntry(void): number(0), distance(1000.0)
	{}
	;
} ;

unsigned int numberMin[NUM_RADAR];
unsigned int numberMax[NUM_RADAR];

MedianEntry medianEntries[NUM_RADAR][medianSize];

void do_initializeControl()
{
	for (int n=0;n<NUM_RADAR;n++){
	motor.drive(1);
	int i;
	for (i=0;i<medianSize;i++){
		medianEntries[n][i] = MedianEntry(i, 500.0);
	}
	numberMin[n] = 0;
	numberMax[n] = i-1;
	}
}

void do_radarDetect(int n,unsigned int time)
{
	unsigned int nStart=__HAL_TIM_GET_COUNTER(&htim5);
	HAL_GPIO_WritePin(SYNC2_GPIO_Port, SYNC2_Pin, GPIO_PIN_SET);
	float distance = time; 

	distance = distance - 500;
	distance = distance / 58;

// 6us in debug mode, 1us in O3
	int i;
	bool bFound = false;
	for (i=0;i<medianSize;i++){
		if (bFound){
			medianEntries[n][i - 1] = medianEntries[n][i];
		}
		if (medianEntries[n][i].number == numberMin[n]){
			bFound = true;
		}
	}
	numberMax[n]++;
	numberMin[n]++;
	if (distance > medianEntries[n][medianSize - 2].distance) {
		medianEntries[n][medianSize - 1] = MedianEntry(numberMax[n], distance);
	} else {
		bFound = false;
		for (i=medianSize-2; i>0 ;i--){
			medianEntries[n][i + 1] = medianEntries[n][i];
			if (distance < medianEntries[n][i].distance &&
				distance > medianEntries[n][i-1].distance) {
				medianEntries[n][i] = MedianEntry(numberMax[n], distance);
				goto done;
			}
		}
		medianEntries[n][1] = medianEntries[n][0];
		medianEntries[n][0] = MedianEntry(numberMax[n], distance);
	}
done:
	radarDistances[n] = medianEntries[n][medianSize/2].distance;
		medianTime = __HAL_TIM_GET_COUNTER(&htim5) - nStart;
	HAL_GPIO_WritePin(SYNC2_GPIO_Port, SYNC2_Pin, GPIO_PIN_RESET);
}

void do_frontEdgeDetect(int leftOrRight)
{
	bool bIsOutRight = (GPIO_PIN_SET == HAL_GPIO_ReadPin(CNY1_GPIO_Port, CNY1_Pin));
	bool bIsOutLeft = (GPIO_PIN_SET == HAL_GPIO_ReadPin(CNY2_GPIO_Port, CNY2_Pin));
	bool bIsOut = (bIsOutRight || bIsOutLeft);

	HAL_GPIO_WritePin(LED2_D5_GPIO_Port,
		LED2_D5_Pin,
		bIsOutRight ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 
		bIsOutLeft ? GPIO_PIN_SET : GPIO_PIN_RESET);

	if (state[0] != backWhite && bIsOut) {
		if (!bIsOutRight) { // left is out
			timedDrive(500, LEFT, 1000);
		} else if (!bIsOutLeft) {
			timedDrive(500, RIGHT, 1000);
		} else {
			timedDrive(100, -1000);
		}
		state[0] = backWhite;
	} else if (state[0] == backWhite && !bIsOut) {
		timedDrive(10, 0);
		schedWakeup(0, 20, backRecover);
	} else if (bIsOut) {
		timedDrive(500, -200);
	}
}

void do_backEdgeDetect(int leftOrRight)
{
}

void do_stepInLoop()
{
// count time in current state, should not last more than 10s
	float frontMean = (radarDistances[FRONT_RIGHT_RADAR] + 
		radarDistances[FRONT_LEFT_RADAR] + 
		radarDistances[FRONT_CENTER_RADAR]) / 3;
	displayOutFront();
	if (state[0] != startedDelaying && state[0] != notStarted) {
		if (frontMean > radarDistances[RIGHT_RADAR] &&
			frontMean > radarDistances[LEFT_RADAR] &&
			frontMean > radarDistances[BACK_RADAR]) {
				if (frontMean < 60) {
					//timedDrive(500, 500);
					return;
				}
		}
		//timedDrive(5000, LEFT, 500);
	}
}
 

void do_action1(State next)
{
	state[1] = next;
	switch (next) {
	case notStarted:
		motor.drive(0);
		break;
	//default:
	// error
	}
}

void do_action2(State next)
{
}

void do_action3(State next)
{
}

void displayOutFront()
{
	bool bResultRight = (GPIO_PIN_SET == HAL_GPIO_ReadPin(CNY1_GPIO_Port, CNY1_Pin));
	bool bResultLeft = (GPIO_PIN_SET == HAL_GPIO_ReadPin(CNY1_GPIO_Port, CNY2_Pin));
	HAL_GPIO_WritePin(LED2_D5_GPIO_Port,
		LED2_D5_Pin,
		(bResultRight ? GPIO_PIN_SET : GPIO_PIN_RESET));
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, (bResultLeft ? GPIO_PIN_SET : GPIO_PIN_RESET));
}

bool isOutFront(int rightOrLeft)
{
	bool bResult;
	bool bResultRight = (GPIO_PIN_SET == HAL_GPIO_ReadPin(CNY1_GPIO_Port, CNY1_Pin));
	bool bResultLeft = (GPIO_PIN_SET == HAL_GPIO_ReadPin(CNY2_GPIO_Port, CNY2_Pin));
	HAL_GPIO_WritePin(LED2_D5_GPIO_Port,
		LED2_D5_Pin,
		(bResultRight ? GPIO_PIN_SET : GPIO_PIN_RESET));
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, (bResultLeft ? GPIO_PIN_SET : GPIO_PIN_RESET));
	if (rightOrLeft == RIGHT) {
		bResult = bResultRight;
	} else if (rightOrLeft == LEFT) {
		bResult = bResultLeft;
	} else {
		bResult = bResultRight ||
			bResultLeft;
	}
	return bResult;
}

bool isOutBack(int rightOrLeft)
{
}




void timedDrive(int ms, int speed)
{
		schedWakeup(1, ms, notStarted); 
		motor.drive(speed);
}

void timedDrive(int ms, int rightOrLeft, int speed)
{	
		schedWakeup(1, ms, notStarted); 
		motor.drive(rightOrLeft, speed);
}

TIM_OC_InitTypeDef * getOCConfig(int ms)
{
	static TIM_OC_InitTypeDef sConfigOC;

	sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
	sConfigOC.Pulse = 10*ms;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	return &sConfigOC;
}

void schedWakeup(int which, int ms, State next)
{
	if (which < 4) {
		nextState[which] = next;
	}
	switch (which) {
	case 0:
		HAL_TIM_Base_Stop_IT(&htim10);
		__HAL_TIM_SET_COUNTER(&htim10, 0);
		HAL_TIM_OC_ConfigChannel(&htim10, getOCConfig(ms), TIM_CHANNEL_1);
		HAL_TIM_OC_Start_IT(&htim10, TIM_CHANNEL_1);	
		break;
	case 1:
		HAL_TIM_Base_Stop_IT(&htim11);
		__HAL_TIM_SET_COUNTER(&htim11, 0);
		HAL_TIM_OC_ConfigChannel(&htim11, getOCConfig(ms), TIM_CHANNEL_1);
		HAL_TIM_OC_Start_IT(&htim11, TIM_CHANNEL_1);	
		break;
	case 2:
		HAL_TIM_OC_ConfigChannel(&htim13, getOCConfig(ms), TIM_CHANNEL_1);
		HAL_TIM_OC_Start_IT(&htim13, TIM_CHANNEL_1);	
		break;
	case 3:
		HAL_TIM_OC_ConfigChannel(&htim14, getOCConfig(ms), TIM_CHANNEL_1);
		HAL_TIM_OC_Start_IT(&htim14, TIM_CHANNEL_1);	
		break;
	//default:
	// error
	}
}

void setDebugState(const char * newState)
{
	int i = 0;
	while (newState[i] && i< 20) {
		dState[i] = newState[i];
		i++;
	}
	dState[i] = newState[i];
}

