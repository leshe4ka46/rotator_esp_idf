#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "stepper_motor_encoder.h"
#include "as5600_lib.c"
#include "sdkconfig.h"
//#include "stepper_uart.c"
#define STEPS_IN360 CONFIG_STEPPER_STEPS_PER_ROTATION*CONFIG_STEPPER_MICROSTEP/360

/*
#define STEP_MOTOR_GPIO_ENX       4
#define STEP_MOTOR_GPIO_DIRX      18
#define STEP_MOTOR_GPIO_STEPX     17

#define STEP_MOTOR_GPIO_ENY       5
#define STEP_MOTOR_GPIO_DIRY      16
#define STEP_MOTOR_GPIO_STEPY     15
*/

//board v1.0
#define STEP_MOTOR_GPIO_ENX       14
#define STEP_MOTOR_GPIO_DIRX      12
#define STEP_MOTOR_GPIO_STEPX     13

#define STEP_MOTOR_GPIO_ENY       11
#define STEP_MOTOR_GPIO_DIRY      9
#define STEP_MOTOR_GPIO_STEPY     10


/*
#define STEP_MOTOR_GPIO_ENX       11
#define STEP_MOTOR_GPIO_DIRX      9
#define STEP_MOTOR_GPIO_STEPX     10

#define STEP_MOTOR_GPIO_ENY       14
#define STEP_MOTOR_GPIO_DIRY      12
#define STEP_MOTOR_GPIO_STEPY     13
*/

/* //test uart
#define STEP_MOTOR_GPIO_ENX       5
#define STEP_MOTOR_GPIO_DIRX      16
#define STEP_MOTOR_GPIO_STEPX     15

#define STEP_MOTOR_GPIO_ENY       14
#define STEP_MOTOR_GPIO_DIRY      12
#define STEP_MOTOR_GPIO_STEPY     13*/



#define STEP_MOTOR_ENABLE_LEVEL  0 // DRV8825 is enabled on low level
#define STEP_MOTOR_SPIN_DIR_CLOCKWISE 1
#define STEP_MOTOR_SPIN_DIR_COUNTERCLOCKWISE !STEP_MOTOR_SPIN_DIR_CLOCKWISE

#define STEP_MOTOR_RESOLUTION_HZ 1000000 // 1MHz resolution

//#define DO_ROTATE_ENABLED 1
uint8_t DO_ROTATE_ENABLED;

#define DEFAULT_ACCEL_SAMPLES 3200*4
#define DEFAULT_DECEL_SAMPLES 3200*4
uint32_t accel_samplesx = DEFAULT_ACCEL_SAMPLES;
uint32_t uniform_speed_hzx = 15000;
uint32_t decel_samplesx = DEFAULT_DECEL_SAMPLES;
uint32_t do_rotate_speed_hzx = 10000;

uint32_t accel_samplesy = DEFAULT_ACCEL_SAMPLES;
uint32_t uniform_speed_hzy = 15000;
uint32_t decel_samplesy = DEFAULT_DECEL_SAMPLES;
uint32_t do_rotate_speed_hzy = 10000;

uint8_t delta_stepper_pos=10;

int32_t steps_X,curr_steps_X,read_steps_X,steps_Y,curr_steps_Y,read_steps_Y;
uint8_t motorX_isReady=1,motorY_isReady=1;
void stepperX_task(void *pvParameter)
{

    gpio_config_t en_dir_gpio_configx = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 1ULL << STEP_MOTOR_GPIO_DIRX | 1ULL << STEP_MOTOR_GPIO_ENX,
    };
    ESP_ERROR_CHECK(gpio_config(&en_dir_gpio_configx));

    rmt_channel_handle_t motor_chan_x = NULL;
    rmt_tx_channel_config_t tx_chan_config_x = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select clock source
        .gpio_num = STEP_MOTOR_GPIO_STEPX,
        .mem_block_symbols = 64,
        .resolution_hz = STEP_MOTOR_RESOLUTION_HZ,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config_x, &motor_chan_x));

    gpio_set_level(STEP_MOTOR_GPIO_DIRX, STEP_MOTOR_SPIN_DIR_CLOCKWISE);
    gpio_set_level(STEP_MOTOR_GPIO_ENX, STEP_MOTOR_ENABLE_LEVEL);
    stepper_motor_curve_encoder_config_t accel_encoder_configx = {
        .resolution = STEP_MOTOR_RESOLUTION_HZ,
        .sample_points = accel_samplesx,
        .start_freq_hz = 1500,
        .end_freq_hz = uniform_speed_hzx,
    };
    rmt_encoder_handle_t accel_motor_encoderX = NULL;
    ESP_ERROR_CHECK(rmt_new_stepper_motor_curve_encoder(&accel_encoder_configx, &accel_motor_encoderX));

	stepper_motor_uniform_encoder_config_t uniform_encoder_configx = {
	        .resolution = STEP_MOTOR_RESOLUTION_HZ,
	    };
    rmt_encoder_handle_t uniform_motor_encoderX = NULL;
    ESP_ERROR_CHECK(rmt_new_stepper_motor_uniform_encoder(&uniform_encoder_configx, &uniform_motor_encoderX));

    stepper_motor_curve_encoder_config_t decel_encoder_configx = {
        .resolution = STEP_MOTOR_RESOLUTION_HZ,
        .sample_points = decel_samplesx,
        .start_freq_hz = uniform_speed_hzx,
        .end_freq_hz = 1500,
    };
    rmt_encoder_handle_t decel_motor_encoderX = NULL;
    ESP_ERROR_CHECK(rmt_new_stepper_motor_curve_encoder(&decel_encoder_configx, &decel_motor_encoderX));
	ESP_ERROR_CHECK(rmt_enable(motor_chan_x));
    rmt_transmit_config_t tx_configX = {
        .loop_count = 0,
    };
	curr_steps_X=steps_X=get_target(0);
	while(1){
		if(steps_X!=curr_steps_X){
			gpio_set_level(STEP_MOTOR_GPIO_ENX, STEP_MOTOR_ENABLE_LEVEL);
			gpio_set_level(STEP_MOTOR_GPIO_DIRX, (steps_X>curr_steps_X)?STEP_MOTOR_SPIN_DIR_CLOCKWISE:STEP_MOTOR_SPIN_DIR_COUNTERCLOCKWISE);
			ESP_LOGI("anglesX","%ld %ld",steps_X,curr_steps_X);
			if(abs(steps_X-curr_steps_X)<=DEFAULT_ACCEL_SAMPLES+DEFAULT_DECEL_SAMPLES){

				accel_samplesx=abs(steps_X-curr_steps_X)/2;
				tx_configX.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_x, accel_motor_encoderX, &accel_samplesx, sizeof(accel_samplesx), &tx_configX));
				decel_samplesx=abs(steps_X-curr_steps_X)-accel_samplesx;
				tx_configX.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_x, decel_motor_encoderX, &decel_samplesx, sizeof(decel_samplesx), &tx_configX));

				motorX_isReady=0;
			}
			else{
				accel_samplesx=DEFAULT_ACCEL_SAMPLES;
				decel_samplesx=DEFAULT_DECEL_SAMPLES;
				tx_configX.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_x, accel_motor_encoderX, &accel_samplesx, sizeof(accel_samplesx), &tx_configX));

				// uniform phase
				tx_configX.loop_count = abs(steps_X-curr_steps_X)-decel_samplesx-accel_samplesx;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_x, uniform_motor_encoderX, &uniform_speed_hzx, sizeof(uniform_speed_hzx), &tx_configX));

				// deceleration phase
				tx_configX.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_x, decel_motor_encoderX, &decel_samplesx, sizeof(decel_samplesx), &tx_configX));
			}
			curr_steps_X=steps_X;
			// wait all transactions finished
			ESP_ERROR_CHECK(rmt_tx_wait_all_done(motor_chan_x, -1));
			motorX_isReady=1;
			gpio_set_level(STEP_MOTOR_GPIO_ENX, !STEP_MOTOR_ENABLE_LEVEL);
		}
		if(motorX_isReady==1 && DO_ROTATE_ENABLED){
			read_steps_X=(as5600_getAngleXnolog(0)*STEPS_IN360);
			if(curr_steps_X<read_steps_X-delta_stepper_pos || curr_steps_X>read_steps_X+delta_stepper_pos){
				gpio_set_level(STEP_MOTOR_GPIO_ENX, STEP_MOTOR_ENABLE_LEVEL);
				tx_configX.loop_count = abs(curr_steps_X-read_steps_X);
				ESP_LOGE("X stepper error", "curr_steps_X:%ld, read_steps_X:%ld delta:%d",curr_steps_X,read_steps_X,tx_configX.loop_count);
				gpio_set_level(STEP_MOTOR_GPIO_DIRX, (read_steps_X<curr_steps_X)?STEP_MOTOR_SPIN_DIR_CLOCKWISE:STEP_MOTOR_SPIN_DIR_COUNTERCLOCKWISE);

				ESP_ERROR_CHECK(rmt_transmit(motor_chan_x, uniform_motor_encoderX, &do_rotate_speed_hzx, sizeof(do_rotate_speed_hzx), &tx_configX));
				ESP_ERROR_CHECK(rmt_tx_wait_all_done(motor_chan_x, -1));
				gpio_set_level(STEP_MOTOR_GPIO_ENX, !STEP_MOTOR_ENABLE_LEVEL);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}
void stepperY_task(void *pvParameter)
{

	gpio_config_t en_dir_gpio_configy = {
			.mode = GPIO_MODE_OUTPUT,
			.intr_type = GPIO_INTR_DISABLE,
			.pin_bit_mask = 1ULL << STEP_MOTOR_GPIO_DIRY | 1ULL << STEP_MOTOR_GPIO_ENY,
		};
	ESP_ERROR_CHECK(gpio_config(&en_dir_gpio_configy));

	rmt_channel_handle_t motor_chan_y = NULL;
	rmt_tx_channel_config_t tx_chan_config_y = {
		.clk_src = RMT_CLK_SRC_DEFAULT, // select clock source
		.gpio_num = STEP_MOTOR_GPIO_STEPY,
		.mem_block_symbols = 64,
		.resolution_hz = STEP_MOTOR_RESOLUTION_HZ,
		.trans_queue_depth = 10, // set the number of transactions that can be pending in the background
	};
	ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config_y, &motor_chan_y));
	gpio_set_level(STEP_MOTOR_GPIO_DIRY, STEP_MOTOR_SPIN_DIR_CLOCKWISE);
	gpio_set_level(STEP_MOTOR_GPIO_ENY, STEP_MOTOR_ENABLE_LEVEL);
	stepper_motor_curve_encoder_config_t accel_encoder_configy = {
		.resolution = STEP_MOTOR_RESOLUTION_HZ,
		.sample_points = accel_samplesy,
		.start_freq_hz = 1500,
		.end_freq_hz = uniform_speed_hzy,
	};
	rmt_encoder_handle_t accel_motor_encoderY = NULL;
	ESP_ERROR_CHECK(rmt_new_stepper_motor_curve_encoder(&accel_encoder_configy, &accel_motor_encoderY));

	stepper_motor_uniform_encoder_config_t uniform_encoder_configy = {
		.resolution = STEP_MOTOR_RESOLUTION_HZ,
	};
	rmt_encoder_handle_t uniform_motor_encoderY = NULL;
	ESP_ERROR_CHECK(rmt_new_stepper_motor_uniform_encoder(&uniform_encoder_configy, &uniform_motor_encoderY));

	stepper_motor_curve_encoder_config_t decel_encoder_configy = {
		.resolution = STEP_MOTOR_RESOLUTION_HZ,
		.sample_points = decel_samplesy,
		.start_freq_hz = uniform_speed_hzy,
		.end_freq_hz = 1500,
	};
	rmt_encoder_handle_t decel_motor_encoderY = NULL;
	ESP_ERROR_CHECK(rmt_new_stepper_motor_curve_encoder(&decel_encoder_configy, &decel_motor_encoderY));
	ESP_ERROR_CHECK(rmt_enable(motor_chan_y));



	rmt_transmit_config_t tx_configY = {
		.loop_count = 0,
	};
	steps_Y=curr_steps_Y=get_target(1);
	while(1){
		if(steps_Y!=curr_steps_Y){
			gpio_set_level(STEP_MOTOR_GPIO_ENY, STEP_MOTOR_ENABLE_LEVEL);
			gpio_set_level(STEP_MOTOR_GPIO_DIRY, (steps_Y>curr_steps_Y)?STEP_MOTOR_SPIN_DIR_CLOCKWISE:STEP_MOTOR_SPIN_DIR_COUNTERCLOCKWISE);
			ESP_LOGI("anglesY","%ld %ld",steps_Y,curr_steps_Y);
			if(abs(steps_Y-curr_steps_Y)<=DEFAULT_ACCEL_SAMPLES+DEFAULT_DECEL_SAMPLES){

				accel_samplesy=abs(steps_Y-curr_steps_Y)/2;
				tx_configY.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_y, accel_motor_encoderY, &accel_samplesy, sizeof(accel_samplesy), &tx_configY));
				decel_samplesy=abs(steps_Y-curr_steps_Y)-accel_samplesy;
				tx_configY.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_y, decel_motor_encoderY, &decel_samplesy, sizeof(decel_samplesy), &tx_configY));
			}
			else{
				accel_samplesy=DEFAULT_ACCEL_SAMPLES;
				decel_samplesy=DEFAULT_DECEL_SAMPLES;
				tx_configY.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_y, accel_motor_encoderY, &accel_samplesy, sizeof(accel_samplesy), &tx_configY));

				// uniform phase
				tx_configY.loop_count = abs(steps_Y-curr_steps_Y)-accel_samplesy-decel_samplesy;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_y, uniform_motor_encoderY, &uniform_speed_hzy, sizeof(uniform_speed_hzy), &tx_configY));

				// deceleration phase
				tx_configY.loop_count = 0;
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_y, decel_motor_encoderY, &decel_samplesy, sizeof(decel_samplesy), &tx_configY));
			}
			curr_steps_Y=steps_Y;
			// wait all transactions finished
			ESP_ERROR_CHECK(rmt_tx_wait_all_done(motor_chan_y, -1));
			motorY_isReady=1;
			gpio_set_level(STEP_MOTOR_GPIO_ENY, !STEP_MOTOR_ENABLE_LEVEL);
		}
		if(motorY_isReady==1 && DO_ROTATE_ENABLED){
			read_steps_Y=(as5600_getAngleYnolog(0)*STEPS_IN360);
			if(curr_steps_Y<read_steps_Y-delta_stepper_pos || curr_steps_Y>read_steps_Y+delta_stepper_pos){
				gpio_set_level(STEP_MOTOR_GPIO_ENY, STEP_MOTOR_ENABLE_LEVEL);
				tx_configY.loop_count = abs(curr_steps_Y-read_steps_Y);
				ESP_LOGE("Y stepper error", "curr_steps_Y:%ld, read_steps_Y:%ld delta:%d",curr_steps_Y,read_steps_Y,tx_configY.loop_count);
				gpio_set_level(STEP_MOTOR_GPIO_DIRY, (read_steps_Y<curr_steps_Y)?STEP_MOTOR_SPIN_DIR_CLOCKWISE:STEP_MOTOR_SPIN_DIR_COUNTERCLOCKWISE);
				ESP_ERROR_CHECK(rmt_transmit(motor_chan_y, uniform_motor_encoderY, &do_rotate_speed_hzy, sizeof(do_rotate_speed_hzy), &tx_configY));
				ESP_ERROR_CHECK(rmt_tx_wait_all_done(motor_chan_y, -1));
				gpio_set_level(STEP_MOTOR_GPIO_ENY, !STEP_MOTOR_ENABLE_LEVEL);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

esp_err_t init_steppers(){
	//init_tmc2208s(4,18,0,0);
    nvs_handle_t ws = open_nvs("settings", NVS_READONLY);
    check_err(nvs_get_u8(ws, "dorotate", &DO_ROTATE_ENABLED));
    nvs_close(ws);
    ESP_LOGI("STEPPERS", "Initing");
    xTaskCreatePinnedToCore(&stepperX_task, "stepperX_task", 1024*3, NULL, 5, NULL,1);
    xTaskCreatePinnedToCore(&stepperY_task, "stepperY_task", 1024*3, NULL, 5, NULL,1);
    return ESP_OK;
}

esp_err_t delta_stepper(uint8_t idx,int32_t delta){
	if(idx==0){
		steps_X+=delta;
	}
	else if(idx==1){
		steps_Y+=delta;
	}
	return ESP_OK;
}
esp_err_t absolute_stepper(uint8_t idx,int32_t steps){
	set_target(steps,idx);
	if(idx==0){
		steps_X=steps;

	}
	else if(idx==1){
		steps_Y=steps;
	}
	return ESP_OK;
}
uint8_t get_state(uint8_t idx){
	if(idx==0){
		return motorX_isReady;
	}
	else if(idx==1){
		return motorY_isReady;
	}
	else{
		return 0;
	}
}
int32_t get_pos(uint8_t idx){
	if(idx==0){
		return steps_X;
	}
	else if(idx==1){
		return steps_Y;
	}
	else{
		return 0;
	}
}

esp_err_t set_pos(uint8_t idx,int32_t steps){
	if(idx==0){
		steps_X=steps;
		curr_steps_X=steps;
	}
	else if(idx==1){
		steps_Y=steps;
		curr_steps_Y=steps;
	}
	else{
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t reset_all_positions(){
	steps_X=curr_steps_X=steps_Y=curr_steps_Y=0;
	set_zero_as5600();
	set_target(0,0);
	set_target(0,1);
	return ESP_OK;
}

uint8_t motor_isready(){
	return motorX_isReady&&motorY_isReady;
}

void do_rotate_set(uint8_t val){
	DO_ROTATE_ENABLED=val;
}
uint8_t do_rotate_get(){
	return DO_ROTATE_ENABLED;
}