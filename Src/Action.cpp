#include "Action.h"
#include "Motor.h"
#include "Sensor.h"
#include "tim.h"
#include "Mutex.h"

using namespace std;

std::queue <Action> todos;
static volatile mutex_t m_Action; // mutex

void doWakeup1()
{
}
void doWakeup2()
{
}
void doWakeup3()
{
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{

	if (htim == &htim10) {
		doWakeup0();
	} else if (htim == &htim11) {
		doWakeup1();
	} else if (htim == &htim13) {
		doWakeup2();
	} else if (htim == &htim14) {
		doWakeup3();
	} else {
		// error
	}
// signal something
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); // sync pulse on PA_0  (arduino A0)
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
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

void schedWakeup(int which, int ms)
{

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


void queueAction(Action action)
{ // push to queue, will see after if need to do something
	CpuCriticalVar();
	CpuEnterCritical();
	todos.push(action);
	CpuExitCritical();
}

void doSeek(){
	float min_dist = 999;
	int min_index = 0;
	for (int i = 0; i < NUM_RADAR;i++){
		if (radarDistances[i]< min_dist) {
			min_dist = radarDistances[i];
			min_index = i;
		}
	}
	switch (min_index) {
	case FRONT_RIGHT_RADAR:
		queueAction(Action(Right, 50));
		queueAction(Action(Fwd, 1000));
		break;
	case FRONT_CENTER_RADAR:
		queueAction(Action(Fwd, 1000));
		break;
	case FRONT_LEFT_RADAR:
		queueAction(Action(Left, 50));
		queueAction(Action(Fwd, 1000));
		break;
	case RIGHT_RADAR:
		queueAction(Action(Right, 200));
		queueAction(Action(Fwd, 1000));
		break;
	case LEFT_RADAR:
		queueAction(Action(Left, 200));
		queueAction(Action(Fwd, 1000));
		break;
	case BACK_RADAR:
		queueAction(Action(Left, 500));
		queueAction(Action(Seek, 0));
		break;
	}
}

volatile bool started;

int executeAction(Action action){
	switch (action.move) {
	case Stop:
		motor.drive(0);
		break;
	case Fwd:
		motor.drive(256);
		break;
	case Back:
		motor.drive(-256);
		break;
	case Left:
		motor.drive(LEFT, 256);
		break;
	case Right:
		motor.drive(RIGHT, 256);
		break;
	case Seek:
		doSeek();
	case Start:
		started=true;
	//default:
		// bug !
	}
	return action.time_ms; 
}

int checkTodo()
{
	CpuCriticalVar();
	CpuEnterCritical();
	//lock_mutex(&m_Action);
	if (todos.empty()) {
		CpuExitCritical();
		if (started) {
			queueAction(Action(Seek, 0));
		}
		//unlock_mutex(&m_Action);
		return 0; // execute immediately
	}
	Action action;
	action = todos.front();
	todos.pop();
	CpuExitCritical();
	//unlock_mutex;

	return executeAction(action);

}


void doWakeup0()
{
	checkTodo();
}

void do_startButton(void)
{
	queueAction(Action(Stop, 5000));
	queueAction(Action(Start, 0));
}

void mainLoop(){
	int time_ms;
	while(1) {
		time_ms= checkTodo();
		HAL_Delay(time_ms+1);
	}
}

void timedDrive(int ms, int speed)
{
	schedWakeup(1, ms); 
	motor.drive(speed);
}

