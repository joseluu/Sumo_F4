#include "Sensor.h"
#include "Motor.h"
#include "tim.h"
#include "Action.h"

// module variables

volatile bool OutDetect[4];

volatile float radarDistances[NUM_RADAR];
const int medianSize = 11;
volatile char dState[20] = "";


// forward definitions

void timedDrive(int ms, int speed);
void timedDrive(int ms, int rightOrLeft, int speed);
bool isOutFront(int rightOrLeft = 0);
bool isOutBack(int rightOrLeft = 0);
void setDebugState(const char* state);
void displayOutFront(void);




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
	OutDetect[FRONT_RIGHT] = bIsOutRight;
	OutDetect[FRONT_LEFT] = bIsOutLeft;

	HAL_GPIO_WritePin(LED2_D5_GPIO_Port,
		LED2_D5_Pin,
		bIsOutRight ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 
		bIsOutLeft ? GPIO_PIN_SET : GPIO_PIN_RESET);

	onFrontEdgeDetect(bIsOutRight, bIsOutLeft);
}

void do_backEdgeDetect(int leftOrRight)
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
	bool bResult=false;
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
	return false;
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Check proper line */
	if (GPIO_Pin == B1_Pin) { // Blue push button: start
		do_startButton();
	} else if (GPIO_Pin == ECHO_11_A1_EXTI1_Pin) {
		do_radarDetect(FRONT_LEFT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO_10_A2_EXTI4_Pin) {
		do_radarDetect(FRONT_CENTER_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO_12_A3_EXTI0_Pin) {
		do_radarDetect(FRONT_RIGHT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO1_PC10_EXTI10_Pin) {
		do_radarDetect(RIGHT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO2_PA15_EXTI15_Pin) {
		do_radarDetect(LEFT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO3_PC12_EXTI12_Pin) {
		do_radarDetect(BACK_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == CNY1_Pin) {
		do_frontEdgeDetect(RIGHT);

	} else if (GPIO_Pin == CNY2_Pin) {
		do_frontEdgeDetect(LEFT); // 

	} else if (0 && (GPIO_Pin == ECHO_11_A1_EXTI1_Pin)) {
		do_backEdgeDetect(LEFT);
	}
}
