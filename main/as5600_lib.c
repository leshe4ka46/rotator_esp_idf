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

#define I2C_X              I2C_NUM_0
#define I2C_Y              I2C_NUM_1
#define AS5600_ADDR   0x36
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
				.clk_speed = 400000
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
				.clk_speed = 400000
		}
	};
	ESP_ERROR_CHECK(i2c_param_config(I2C_Y, &confY));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_Y, confY.mode, 0, 0, 0));

	ESP_LOGI("AS5600", "I2C initialized");
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
	i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(100));
	i2c_cmd_link_delete(cmd);
	vTaskDelay(pdMS_TO_TICKS(1));

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, device_addr << 1 | I2C_READ_MODE, ACK_CHECK_ENABLE);
	i2c_master_read_byte(cmd, &angle_high, ACK);
	i2c_master_read_byte(cmd, &angle_low, NACK);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(100));
	i2c_cmd_link_delete(cmd);

	result=(uint16_t)(angle_high<<8|angle_low);
	angle=((int) result & 0xFFF)*360.0/4096.0;
	return 360.0f-angle;
}


float as_angleX=0,as_prevangleX=0,as_delta_angleX=0,as_temp_angleX=0;
int32_t turnsX=0;

float as_angleY=0,as_prevangleY=0,as_delta_angleY=0,as_temp_angleY=0;
int32_t turnsY=0;
uint8_t upd_turns=0;
void tickAS5600(void *pvParameter)
{
	get_turns();
	turnsX=get_turns_X();
	turnsY=get_turns_Y();

	get_delta();
	as_prevangleX=as5600_get_currentAngle(AS5600_ADDR,I2C_X);
	as_angleX=as_prevangleX;
	as_delta_angleX=get_delta_X();//=as_angleX;

	as_prevangleY=as5600_get_currentAngle(AS5600_ADDR,I2C_Y);
	as_angleY=as_prevangleY;
	as_delta_angleY=get_delta_Y();//=as_angleY;
	while(1)
	  {
		  	as_temp_angleX = as5600_get_currentAngle(AS5600_ADDR,I2C_X);
			if (as_temp_angleX - as_prevangleX > 180 && as_prevangleX < 180)
			{
				turnsX -= 1;
				upd_turns=1;
			}
			if (as_temp_angleX - as_prevangleX < -180 && as_prevangleX > 180)
			{
				turnsX += 1;
				upd_turns=1;
			}
			as_angleX = as_temp_angleX + 360 * turnsX - as_delta_angleX;
			as_prevangleX = as_temp_angleX;


			as_temp_angleY = as5600_get_currentAngle(AS5600_ADDR,I2C_Y);
			if (as_temp_angleY - as_prevangleY > 180 && as_prevangleY < 180)
			{
				turnsY -= 1;
				upd_turns=1;
			}
			if (as_temp_angleY - as_prevangleY < -180 && as_prevangleY > 180)
			{
				turnsY += 1;
				upd_turns=1;
			}
			as_angleY = as_temp_angleY + 360 * turnsY - as_delta_angleY;
			as_prevangleY = as_temp_angleY;

			if(upd_turns==1){
				set_turns(turnsX,turnsY);
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
	turnsX=turnsY=0;
	set_delta(as_temp_angleX,as_temp_angleY);
	as_delta_angleX=as_temp_angleX;
	as_delta_angleY=as_temp_angleY;
	upd_turns=1;
}





float as5600_getAngleX(){
		ESP_LOGI("AS5600 X","%f  temp:%f  delta:%f",as_angleX,as_temp_angleX,as_delta_angleX);
		return as_angleX;
}
float as5600_getAngleXnolog(){
		return as_angleX;
}
float as5600_getAngleY(){
		ESP_LOGI("AS5600 Y","%f  temp:%f  delta:%f",as_angleY,as_temp_angleY,as_delta_angleY);
		return as_angleY;
}
float as5600_getAngleYnolog(){
		return as_angleY;
}
