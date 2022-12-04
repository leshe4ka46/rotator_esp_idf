/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

//#include "as5600_lib.c"
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_logic.c"
#include "sdkconfig.h"
#include "esp_idf_version.h"
#include "esp_chip_info.h"
#include "stepper_logic.c"
static const char *REST_TAG = "esp-rest";

#define REST_CHECK(a, str, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)
/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    } else if (CHECK_FILE_EXTENSION(filepath, ".gz")) {
    	httpd_resp_set_hdr(req,"Content-Encoding", "gzip");
        type = "text/javascript";
    }
    return httpd_resp_set_type(req, type);
}
char filter[10];
/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/upd.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    sprintf(filter,"%.9s", filepath);
    if(strcmp(filter,"/www/mcfg")==0){
    	 httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
    	 printf(filter);
    	 printf(" -- yandex fix\r\n");
    	 return ESP_FAIL;
    }
    //printf(filepath);
    /*if (CHECK_FILE_EXTENSION(filepath,".gz") == 0) {
    	  httpd_resp_set_hdr(req,"Content-Encoding", "gzip");
    }*/
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        httpd_resp_set_status(req, "302 Temporary Redirect");
		// Redirect to the "/" root directory
		httpd_resp_set_hdr(req, "Location", "/");
		// iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
		httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

		ESP_LOGI(REST_TAG, "Redirecting to root");
        /* Respond with 500 Internal Server Error */
        //httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\"><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr><center>rotator/1.0.1</center></body></html>");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t get_buf_from_request(httpd_req_t *req,char *buf){
	int total_len = req->content_len;
	int cur_len = 0;
	int received = 0;
	if (total_len >= SCRATCH_BUFSIZE) {
		/* Respond with 500 Internal Server Error */
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
		return ESP_FAIL;
	}
	while (cur_len < total_len) {
		received = httpd_req_recv(req, buf + cur_len, total_len);
		if (received <= 0) {
			/* Respond with 500 Internal Server Error */
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
			return ESP_FAIL;
		}
		cur_len += received;
	}
	buf[total_len] = '\0';

	return ESP_OK;
}


static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", esp_get_idf_version());
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    cJSON_AddNumberToObject(root, "revision", chip_info.revision);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}
//------------------------------STEPPER----------


//int32_t angleX=0,angleY=0,newangleX=0,newangleY=0;
float delta_angleX=0,delta_angleY=0;
#define STEPS_PER_ROTATION CONFIG_STEPPER_STEPS_PER_ROTATION
#define STEPPERS_MICROSTEP CONFIG_STEPPER_MICROSTEP
#define STEPPERS_GEAR_RATIO CONFIG_STEPPER_GEAR_RATIO
//float STEPS_TO_ANGLE=STEPS_PER_ROTATION * STEPPERS_MICROSTEP * STEPPERS_GEAR_RATIO / 360;
int32_t angle_to_steps(float angle) {
  return angle * STEPS_PER_ROTATION * STEPPERS_MICROSTEP * STEPPERS_GEAR_RATIO / 360;
}

/*int32_t delta;//,local_angle;
int32_t getpos(int32_t local_angle, uint8_t motor){  //motor: x--0 y--1
	//local_angle=angle_to_steps(angle);
	if (motor==0){
		delta=local_angle-angleX;
		angleX=local_angle;
		printf("motor:%d oldpos:%ld  pos:%ld \r\n",motor,angleX,newangleX);


	}
	if (motor==1){
		delta=local_angle-angleY;
		angleY=local_angle;
		printf("motor:%d oldpos:%ld  pos:%ld \r\n",motor,angleY,newangleY);
	}
	printf("delta:%ld\r\n",delta);
	return delta;
}
*/

static esp_err_t rotate_angle(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
	cJSON *root = cJSON_Parse(buf);
	const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	int azimut = cJSON_GetObjectItem(root, "azimut")->valueint;
	int elevation = cJSON_GetObjectItem(root, "elevation")->valueint;

	if(is_admin(key)==77){
		absolute_stepper(0,angle_to_steps((float)azimut/100-delta_angleX));
		absolute_stepper(1,angle_to_steps((float)elevation/100-delta_angleY));
		printf("angles: %f %f \r\n",(float)azimut/100,(float)elevation/100);
	}

	cJSON_Delete(root);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

/* Simple handler for getting temperature data */


static esp_err_t adduser(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));

	cJSON *root = cJSON_Parse(buf);
	const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
    set_user(key,0);
    cJSON_Delete(root);
    //free(key);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t anglesdatatx(httpd_req_t *req)
{
	 httpd_resp_set_type(req, "application/json");
	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "azimut", as5600_getAngleX()+delta_angleX);
	cJSON_AddNumberToObject(root, "elevation", as5600_getAngleY()+delta_angleY);
	cJSON_AddNumberToObject(root, "voltage", 0.00);
	cJSON_AddNumberToObject(root, "setted_azimut", (float)get_pos(0)/*stepX.getPosition()*//200/16*360+delta_angleX);
	cJSON_AddNumberToObject(root, "setted_elevation", (float)get_pos(1)/*stepY.getPosition()*//200/16*360+delta_angleY);
	const char *sys_info = cJSON_Print(root);
	httpd_resp_sendstr(req, sys_info);
	free((void *)sys_info);
	cJSON_Delete(root);
    return ESP_OK;
}
static esp_err_t authuser(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
    const char * login = cJSON_GetObjectItem(root, "login")->valuestring;
    const char * password = cJSON_GetObjectItem(root, "password")->valuestring;

    if((strcmp("admin",login)+strcmp("admin",password))==0){
        	set_user(key,77);
        	httpd_resp_sendstr(req, "{\"role\":\"admin\"}");
        }
        else{
        	httpd_resp_sendstr(req, "{\"role\":\"user\"}");
        }
    cJSON_Delete(root);

    return ESP_OK;
}
static esp_err_t whoami(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;

    switch(is_admin(key)){
    case 77:
    	httpd_resp_sendstr(req, "{\"role\":\"admin\"}");
    	break;
    case 0:
        	httpd_resp_sendstr(req, "{\"role\":\"user\"}");
        	break;
    }
    cJSON_Delete(root);
    return ESP_OK;
}
static esp_err_t clearnvs(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;

    switch(is_admin(key)){
    case 77:
    	clear_nvs();
    	httpd_resp_sendstr(req, "OK");
    	break;
    default:
    	httpd_resp_sendstr(req, "NONONO");
        break;
    }
    cJSON_Delete(root);
    return ESP_OK;
}
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    // Set status
    httpd_resp_set_status(req, "302 Temporary Redirect");
    // Redirect to the "/" root directory
    httpd_resp_set_hdr(req, "Location", "/");
    // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
    httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(REST_TAG, "Redirecting to root");
    return ESP_OK;
}



static esp_err_t setXnull(httpd_req_t *req)
{
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
    if(is_admin(key)==77){
    	set_zero_X();
    	httpd_resp_sendstr(req, "OK");
    }
    else{
    	httpd_resp_sendstr(req, "No");
    }
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t setYnull(httpd_req_t *req)
{
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
    if(is_admin(key)==77){
    	set_zero_Y();
    	httpd_resp_sendstr(req, "OK");
    }
    else{
    	httpd_resp_sendstr(req, "No");
    }
    cJSON_Delete(root);
    return ESP_OK;
}
static esp_err_t delta_angle(httpd_req_t *req)
{

    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	int azimut = cJSON_GetObjectItem(root, "azimut")->valueint;
	int elevation = cJSON_GetObjectItem(root, "elevation")->valueint;


    if(is_admin(key)==77){
    	delta_angleX=(float)azimut/100;
    	delta_angleY=(float)elevation/100;
    	printf("newdeltaX:%f %f\r\n",delta_angleX,delta_angleY);
    	httpd_resp_sendstr(req, "OK");
    }
    else{
    	httpd_resp_sendstr(req, "No");
    }

    cJSON_Delete(root);
    return ESP_OK;
}
esp_err_t start_rest_server(const char *base_path)
{
	init_nvs();
    REST_CHECK(base_path, "wrong base path");
	rest_server_context_t *rest_context = (rest_server_context_t*)calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context");
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 12;
    config.lru_purge_enable = true;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed");
    //ESP_ERROR_CHECK(httpd_start(&server, &config));
    /* URI handler for fetching system info */
    httpd_uri_t system_info_get_uri = {
				.uri = "/api/v1/system/info",
				.method = HTTP_GET,
				.handler = system_info_get_handler,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &system_info_get_uri);

    httpd_uri_t users_add_post_uri = {
				.uri = "/api/v1/users/add",
				.method = HTTP_POST,
				.handler = adduser,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &users_add_post_uri);


    httpd_uri_t users_auth_post_uri = {
                .uri = "/api/v1/users/auth",
                .method = HTTP_POST,
                .handler = authuser,
                .user_ctx = rest_context
            };
    httpd_register_uri_handler(server, &users_auth_post_uri);

    httpd_uri_t users_whoami_post_uri = {
				.uri = "/api/v1/users/whoami",
				.method = HTTP_POST,
				.handler = whoami,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &users_whoami_post_uri);

    httpd_uri_t users_clear_post_uri = {
				.uri = "/api/v1/users/clear",
				.method = HTTP_POST,
				.handler = clearnvs,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &users_clear_post_uri);


    httpd_uri_t data_angles_post_uri = {
				.uri = "/api/v1/data/get/angles",
				.method = HTTP_GET,
				.handler = anglesdatatx,
				.user_ctx = rest_context
            };
    httpd_register_uri_handler(server, &data_angles_post_uri);





    httpd_uri_t data_set_angles_uri = {
				.uri = "/api/v1/data/set/angles",
				.method = HTTP_POST,
				.handler = rotate_angle,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &data_set_angles_uri);

    httpd_uri_t data_set_currentangles_uri = {
				.uri = "/api/v1/data/set/currentangles",
				.method = HTTP_POST,
				.handler = delta_angle,
				.user_ctx = rest_context
			};
	httpd_register_uri_handler(server, &data_set_currentangles_uri);

    httpd_uri_t data_set_nullX_uri = {
				.uri = "/api/v1/data/set/nullX",
				.method = HTTP_POST,
				.handler = setXnull,
				.user_ctx = rest_context
			};
	httpd_register_uri_handler(server, &data_set_nullX_uri);

	httpd_uri_t data_set_nullY_uri = {
				.uri = "/api/v1/data/set/nullY",
				.method = HTTP_POST,
				.handler = setYnull,
				.user_ctx = rest_context
			};
	httpd_register_uri_handler(server, &data_set_nullY_uri);

    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);


    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &common_get_uri);


    init_steppers();
    init_i2c(7,6,19,8);
    start_monitoring_AS5600();
    return ESP_OK;

}
