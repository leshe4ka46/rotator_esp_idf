/*
 * stepper_control.cpp
 *
 *  Created on: 1 нояб. 2022 г.
 *      Author: alex9
 */
/*#include "DendoStepper.h"
#include "sdkconfig.h"
int32_t angleX=0,angleY=0,oldangleX=0,oldangleY=0;
#define STEPS_PER_ROTATION CONFIG_STEPPER_STEPS_PER_ROTATION
#define STEPPERS_MICROSTEP CONFIG_STEPPER_MICROSTEP
#define STEPPERS_GEAR_RATIO CONFIG_STEPPER_GEAR_RATIO
int32_t angle_to_steps(float angle) {
  return angle * STEPS_PER_ROTATION * STEPPERS_MICROSTEP * STEPPERS_GEAR_RATIO / 360;
}*/
int32_t getpos(float angle, uint8_t motor){  //motor: x--0 y--1
	printf("motor:%d ")
	if (motor==0){
		return angle_to_steps(angle-angleX);
		angleX=angle;
	}
	if (motor==1){
		return angle_to_steps(angle-angleY);
		angleY=angle;
	}
	return 0;
}
DendoStepper stepX;
//DendoStepper stepY;
esp_err_t init_steppers(){
	DendoStepper_config_t stepX_cfg = {
	        .stepPin = 17,
	        .dirPin = 18,
	        .enPin = 4,
	        .timer_group = TIMER_GROUP_1,
	        .timer_idx = TIMER_1,
	        .miStep = MICROSTEP_1,
	        .stepAngle = 1.8};

	    /*DendoStepper_config_t stepY_cfg = {
	        .stepPin = 15,
	        .dirPin = 16,
	        .enPin = 5,
	        .timer_group = TIMER_GROUP_1,
	        .timer_idx = TIMER_1,
	        .miStep = MICROSTEP_1,
	        .stepAngle = 1.8};*/

	    stepX.config(&stepX_cfg);
	    //stepY.config(&stepY_cfg);

	    stepX.init();
	    //stepY.init();

	    stepX.setSpeed(20000, 1000, 1000);
	    //stepY.setSpeed(20000, 1000, 1000);
	    return ESP_OK;
}
void set_angle(int32_t azim,int32_t elev){
	angleX=azim;
	angleY=elev;
}
void steppers_task(void *pvParameter)
{
	init_steppers();
	while(1){
		if(oldangleX!=angleX){
			stepX.runPos(getpos((float)angleX/100,0));
			oldangleX=angleX;
		}
		/*if(oldangleY!=angleY){
				stepY.runPos(getpos((float)angleY/100,0));
				ldangleY=angleY;
			}*/
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


void steppers_loop(){
	xTaskCreatePinnedToCore(&steppers_task, "steppers_task", 1024*3, NULL, 5, NULL,1);
	//steppers_task();
}
