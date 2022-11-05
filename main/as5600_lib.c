/*
 * as5600_lib.cpp
 *
 *  Created on: 2 нояб. 2022 г.
 *      Author: alex9
 */
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *AS_TAG = "AS5600";
#define I2C_MASTER_NUM              I2C_NUM_0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
uint8_t As5600_AddrX=0x36;
#define RawAngle_Addr 0x0C
#define I2C_WRITE_MODE      0
#define I2C_READ_MODE       1
#define ACK                 I2C_MASTER_ACK                       /*!< I2C ack value */
#define NACK                I2C_MASTER_NACK                       /*!< I2C nack value */
#define ACK_CHECK_ENABLE    1
#define ACK_CHECK_DISABLE   0


void init_i2c(uint8_t i2c_sda_pin,uint8_t i2c_scl_pin){
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = i2c_sda_pin,
		.scl_io_num = i2c_scl_pin,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master={
				.clk_speed = 100000
		}
	};

	ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));

	ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
	ESP_LOGI(AS_TAG, "I2C initialized");
}
float as5600_get_currentAngle(uint8_t device_addr)
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
	i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	vTaskDelay(pdMS_TO_TICKS(1));

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, device_addr << 1 | I2C_READ_MODE, ACK_CHECK_ENABLE);
	i2c_master_read_byte(cmd, &angle_high, ACK); // 0x0c is a high level
	i2c_master_read_byte(cmd, &angle_low, NACK);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);

	result=(uint16_t)(angle_high<<8|angle_low); // A total of 11 points
	angle=((int) result & 0b0000111111111111)*360.0/4096.0;
	//ESP_LOGI(AS_TAG,"%.2f",angle);
	return angle;
}


float as5600_angleX=0,as5600_prevangleX=0,as5600_deltaangleX=0,as5600_start_deltaangleX=0,as5600_temp_angleX=0;
int32_t as5600_turnsX=0;

void tickAS5600(void *pvParameter)
{
	as5600_prevangleX=as5600_get_currentAngle(As5600_AddrX);
	as5600_angleX=as5600_prevangleX;
	as5600_deltaangleX=as5600_angleX;
	as5600_start_deltaangleX=as5600_deltaangleX;
	while(1)
	  {
		  	as5600_temp_angleX = as5600_get_currentAngle(As5600_AddrX);
			if (as5600_temp_angleX - as5600_prevangleX > 180 && as5600_prevangleX < 180)
			{
				as5600_turnsX -= 1;
			}
			if (as5600_temp_angleX - as5600_prevangleX < -180 && as5600_prevangleX > 180)
			{
				as5600_turnsX += 1;
			}
			as5600_angleX = as5600_temp_angleX + 360 * as5600_turnsX - as5600_deltaangleX;
			as5600_prevangleX = as5600_temp_angleX;

			vTaskDelay(pdMS_TO_TICKS(5));
	  }
}
void start_monitoring_AS5600(){
	xTaskCreatePinnedToCore(tickAS5600 , "tickAS5600", 1024, NULL, 10, NULL,1);
}
int count_zerosX=0;
void set_zero_X(){
	//as5600_angleX+=as5600_deltaangleX;
	if(count_zerosX==0){as5600_deltaangleX=as5600_angleX+as5600_deltaangleX;}
	else{as5600_deltaangleX=as5600_start_deltaangleX;}
	count_zerosX+=1;
	if(count_zerosX==2){
		count_zerosX=0;
	}
}
float as5600_getAngleX(){
		ESP_LOGI(AS_TAG,"%f  temp:%f  delta:%f",as5600_angleX,as5600_temp_angleX,as5600_deltaangleX);
		return as5600_angleX;
}



