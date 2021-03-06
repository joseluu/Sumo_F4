#include "Action.h"
#include "Motor.h"
#include "Sensor.h"
#include "tim.h"
#include "Mutex.h"

using namespace std;

std::queue <Action> todos;
volatile int moveCount[Last];
volatile Move lastMove = Last;

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


void cancelAllActions(){
	scopedWithoutInterrupts noIT;

	while (!todos.empty()){
		todos.pop();
	}
}

void queueAction(Action action)
{ // push to queue, will see after if need to do something
	scopedWithoutInterrupts noIT;

	todos.push(action);
}

#define UTURN_MS 600
#define QTURN_MS 520

#define UTURN UTURN_MS +(side==Right?450:0)
#define QTURN QTURN_MS + (side == Right ? 80 : 0)

void doUTurn(Move side, bool keep)
{
	queueAction(Action(side, UTURN, keep)); 
}

void doUTurn(Move side)
{
	queueAction(Action(side, UTURN)); 
}

void doQuarterTurn(Move side,bool keep)
{
	queueAction(Action(side, QTURN, keep)); 
}

void doQuarterTurn(Move side)
{
	queueAction(Action(side, QTURN)); 
}

int findMin(){
	float min_dist = 999;
	int min_index = 0;
	for (int i = 0; i < NUM_RADAR; i++) {
		if (i != FRONT_RIGHT_RADAR) {
			if ((radarDistances[i] < min_dist) && (radarDistances[i] > 1)) {
				min_dist = radarDistances[i];
				min_index = i;
			}
		}
	}
	return min_index;
}


volatile bool started;

volatile bool cancel = false;
volatile bool keepPlan = false;

void doSeek(void);

void onRadarDetect(){
	static int prevIndex = -1;
	static int count;
	count++;
	if ((count % 30) == 0){
		if (started && !keepPlan){
			int min_index = findMin();
			if (min_index != prevIndex){
				prevIndex = min_index;
				cancel = true;
				cancelAllActions();
				doSeek();
			}
		}
	}
}

void doSeek(){
	int min_index = findMin();
	switch (min_index) {
	case FRONT_RIGHT_RADAR:
		queueAction(Action(Right, 50,true));
		queueAction(Action(Fwd, 1000));
		break;
	case FRONT_CENTER_RADAR:
		queueAction(Action(Fwd, 1000));
		break;
	case FRONT_LEFT_RADAR:
		queueAction(Action(Left, 50,true));
		queueAction(Action(Fwd, 1000));
		break;
	case RIGHT_RADAR:
		doQuarterTurn(Right,false);
		queueAction(Action(Fwd, 1000));
		break;
	case LEFT_RADAR:
		doQuarterTurn(Left,false);
		queueAction(Action(Fwd, 1000));
		break;
	case BACK_RADAR:
		static bool bLeft;
		bLeft = !bLeft;
		doUTurn((bLeft?Left:Right),false);
		queueAction(Action(Seek, 0));
		break;
	}
}

void recordAction(Action action){
	if (lastMove != action.move){
	}
}


int executeAction(Action action){
	keepPlan = action.keepPlan;
	switch (action.move) {
	case Stop:
		motor.drive(0);
		break;
	case Fwd:
		recordAction(action);
		motor.drive(256);
		break;
	case Back:
		motor.drive(-256);
		recordAction(action);
		break;
	case Left:
		recordAction(action);
		motor.drive(LEFT, 256);
		break;
	case Right:
		recordAction(action);
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
	CpuInterruptMask();
	CpuWithoutInterrupts();
	//lock_mutex(&m_Action);
	if (todos.empty()) {
		CpuRestoreInterrupts();
		if (started) {
			queueAction(Action(Seek, 0));
		}
		//unlock_mutex(&m_Action);
		return 0; // execute immediately
	}
	Action action;
	action = todos.front();
	todos.pop();
	CpuRestoreInterrupts();
	//unlock_mutex;

	return executeAction(action);

}


void doWakeup0()
{
	checkTodo();
}

void do_startButton(void)
{
#if 1
	queueAction(Action(Stop, 5000));
	queueAction(Action(Start, 0));
#else
// calibration cycle
	queueAction(Action(Stop, 1000));
	queueAction(Action(Fwd, 500));
	doQuarterTurn(Left);
	queueAction(Action(Fwd, 500));
	doUTurn(Right);
	queueAction(Action(Fwd, 1000));
	doUTurn(Left);
	queueAction(Action(Fwd, 500));
	doQuarterTurn(Left);
	queueAction(Action(Fwd, 500));
	queueAction(Action(Stop, 5000));
#endif
}


void mainLoop(){
	int time_ms;
	while(1) {
		time_ms= checkTodo();
		cancel = false;
		HAL_Delay(1);
		while ((time_ms > 0) && !cancel){
			HAL_Delay(10);
			time_ms -= 10;
		}
	}
}

void timedDrive(int ms, int speed)
{
	schedWakeup(1, ms); 
	motor.drive(speed);
}

void onFrontEdgeDetect(bool bIsOutRight, bool bIsOutLeft){
	if (started) {
		if (!bIsOutRight && !bIsOutLeft) {
			return;
		}
		cancel = true;
		cancelAllActions();
		if (bIsOutRight && bIsOutLeft) {
			queueAction(Action(Back, 150,true));
			doUTurn(Left,true);
		} else if (bIsOutRight) {
			queueAction(Action(Back, 100,true));
			doQuarterTurn(Left,true);
		} else if (bIsOutLeft) {
			queueAction(Action(Back, 100,true));
			doQuarterTurn(Right,true);
		} else {
			// bug ?
		}
	}
}
