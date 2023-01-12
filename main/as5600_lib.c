/*
 * as5600_lib.cpp
 *
 *  Created on: 2 нояб. 2022 г.
 *      Author: alex9
 */
#ifndef __nvs_logic_inc_
#include "nvs_logic.c"
#endif

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *AS_TAG = "AS5600";
static const char *AS_TAGX = "AS5600 X";
static const char *AS_TAGY = "AS5600 Y";
#define I2C_X              I2C_NUM_0
#define I2C_Y              I2C_NUM_1
uint8_t As5600_Addr=0x36;
#define RawAngle_Addr 0x0C
#define I2C_WRITE_MODE      0
#define I2C_READ_MODE       1
#define ACK                 I2C_MASTER_ACK
#define NACK                I2C_MASTER_NACK
#define ACK_CHECK_ENABLE    1
#define ACK_CHECK_DISABLE   0

void init_i2c(uint8_t i2c_sdaX_pin,uint8_t i2c_sclX_pin,uint8_t i2c_sdaY_pin,uint8_t i2c_sclY_pin){
	i2c_config_t confX = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = i2c_sdaX_pin,
		.scl_io_num = i2c_sclX_pin,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master={
				.clk_speed = 100000
		}
	};
	ESP_ERROR_CHECK(i2c_param_config(I2C_X, &confX));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_X, confX.mode, 0, 0, 0));


	i2c_config_t confY = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = i2c_sdaY_pin,
		.scl_io_num = i2c_sclY_pin,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master={
				.clk_speed = 100000
		}
	};
	ESP_ERROR_CHECK(i2c_param_config(I2C_Y, &confY));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_Y, confY.mode, 0, 0, 0));
	ESP_LOGI(AS_TAG, "I2C initialized");
}

float as5600_get_currentAngle(uint8_t device_addr,i2c_port_t i2c_port)
{
	uint8_t angle_high = 0;
	uint8_t angle_low = 0;
	uint16_t result = 0;
	float angle=0;
	i2c_cmd_handle_t cmd;

	// Read HIGHADDR
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, device_addr << 1 | I2C_WRITE_MODE, ACK_CHECK_ENABLE);
	i2c_master_write_byte(cmd, RawAngle_Addr, ACK_CHECK_ENABLE);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	vTaskDelay(pdMS_TO_TICKS(1));

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, device_addr << 1 | I2C_READ_MODE, ACK_CHECK_ENABLE);
	i2c_master_read_byte(cmd, &angle_high, ACK); // 0x0c is a high level
	i2c_master_read_byte(cmd, &angle_low, NACK);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);

	result=(uint16_t)(angle_high<<8|angle_low); // A total of 11 points
	angle=((int) result & 0b0000111111111111)*360.0/4096.0;
	//ESP_LOGI(AS_TAG,"%.2f",angle);
	return angle;
}


float as5600_angleX=0,as5600_prevangleX=0,as5600_delta_angleX=0,as5600_temp_angleX=0;
int32_t as5600_turnsX=0;

float as5600_angleY=0,as5600_prevangleY=0,as5600_delta_angleY=0,as5600_temp_angleY=0;
int32_t as5600_turnsY=0;
uint8_t upd_turns=0;
void tickAS5600(void *pvParameter)
{
	get_turns();
	as5600_turnsX=get_turns_X();
	as5600_turnsY=get_turns_Y();

	get_delta();
	as5600_prevangleX=as5600_get_currentAngle(As5600_Addr,I2C_X);
	as5600_angleX=as5600_prevangleX;
	as5600_delta_angleX=get_delta_X();//=as5600_angleX;

	as5600_prevangleY=as5600_get_currentAngle(As5600_Addr,I2C_Y);
	as5600_angleY=as5600_prevangleY;
	as5600_delta_angleY=get_delta_Y();//=as5600_angleY;
	while(1)
	  {
		  	as5600_temp_angleX = as5600_get_currentAngle(As5600_Addr,I2C_X);
			if (as5600_temp_angleX - as5600_prevangleX > 180 && as5600_prevangleX < 180)
			{
				as5600_turnsX -= 1;
				upd_turns=1;
			}
			if (as5600_temp_angleX - as5600_prevangleX < -180 && as5600_prevangleX > 180)
			{
				as5600_turnsX += 1;
				upd_turns=1;
			}
			as5600_angleX = as5600_temp_angleX + 360 * as5600_turnsX - as5600_delta_angleX;
			as5600_prevangleX = as5600_temp_angleX;



			as5600_temp_angleY = as5600_get_currentAngle(As5600_Addr,I2C_Y);
			if (as5600_temp_angleY - as5600_prevangleY > 180 && as5600_prevangleY < 180)
			{
				as5600_turnsY -= 1;
				upd_turns=1;
			}
			if (as5600_temp_angleY - as5600_prevangleY < -180 && as5600_prevangleY > 180)
			{
				as5600_turnsY += 1;
				upd_turns=1;
			}
			as5600_angleY = as5600_temp_angleY + 360 * as5600_turnsY - as5600_delta_angleY;
			as5600_prevangleY = as5600_temp_angleY;

			if(upd_turns==1){
				set_turns(as5600_turnsX,as5600_turnsY);
				upd_turns=0;
			}
			vTaskDelay(pdMS_TO_TICKS(5));
	  }
}
void start_monitoring_AS5600(){
	xTaskCreatePinnedToCore(tickAS5600 , "tickAS5600", 1024*2, NULL, 10, NULL,1);
}


void set_zero_as5600(){
	set_turns(0,0);
	as5600_turnsX=as5600_turnsY=0;
	set_delta(as5600_temp_angleX,as5600_temp_angleY);
	as5600_delta_angleX=as5600_temp_angleX;
	as5600_delta_angleY=as5600_temp_angleY;


}


float as5600_getAngleX(){
		ESP_LOGI(AS_TAGX,"%f  temp:%f  delta:%f",as5600_angleX,as5600_temp_angleX,as5600_delta_angleX);
		return as5600_angleX;
}
float as5600_getAngleXnolog(){
		return as5600_angleX;
}
float as5600_getAngleY(){
		ESP_LOGI(AS_TAGY,"%f  temp:%f  delta:%f",as5600_angleY,as5600_temp_angleY,as5600_delta_angleY);
		return as5600_angleY;
}
float as5600_getAngleYnolog(){
		return as5600_angleY;
}
