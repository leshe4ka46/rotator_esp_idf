/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_vfs.h"
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_idf_version.h"
#include "esp_chip_info.h"
#include "stepper_logic.c"

#include "math.h"

static const char *REST_TAG = "esp-rest";
#define REST_CHECK(a, str, ...)                                                        \
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
    	 return ESP_OK;
    }
    if(strcmp(filter,"/www/gene")==0 || strcmp(filter,"/www/gen_")==0){
        httpd_resp_set_status(req, "302 Temporary Redirect");
		httpd_resp_set_hdr(req, "Location", "/");
		httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);
        /*
    	httpd_resp_set_status(req, "302 Temporary Redirect");
        // Redirect to the "/" root directory
        httpd_resp_set_hdr(req, "Location", "/");
        // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
        httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);*/
    	 return ESP_OK;
    }
    ESP_LOGI(REST_TAG, "path : %s", filepath);
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        httpd_resp_send(req, "<html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\"><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\"><title>404 Not Found</title><script>function goto_root(){window.location.href=\"/\"}</script></head><body><center><h1>404 Not Found</h1><button onclick=\"goto_root()\">Go to root</button></center><hr><center>rotator/1.0.1</center></body></html>", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
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
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);
    //ESP_LOGI(REST_TAG, "File sending complete");
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


float delta_angleX=0,delta_angleY=0;
#define STEPS_PER_ROTATION CONFIG_STEPPER_STEPS_PER_ROTATION
#define STEPPERS_MICROSTEP CONFIG_STEPPER_MICROSTEP
#define STEPPERS_GEAR_RATIO CONFIG_STEPPER_GEAR_RATIO
int32_t angle_to_steps(float angle) {
  return angle * STEPS_PER_ROTATION * STEPPERS_MICROSTEP * STEPPERS_GEAR_RATIO / 360;
}
static esp_err_t rotate_angle(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
	cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"azimut") || !cJSON_HasObjectItem(root,"elevation")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
	const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	double azimut = cJSON_GetObjectItem(root, "azimut")->valuedouble;
	double elevation = cJSON_GetObjectItem(root, "elevation")->valuedouble;

	if(is_admin(key)==77){
		absolute_stepper(0,angle_to_steps(azimut-delta_angleX));
		absolute_stepper(1,angle_to_steps(elevation-delta_angleY));
		printf("angles: %f %f \r\n",azimut,elevation);
	}

	cJSON_Delete(root);
    httpd_resp_sendstr(req, "{\"response\":1}");
    return ESP_OK;
}

double receiver[3];
double sputnic[3];
double angle[2];

double prev_sputnic[2];
double delta[2];
#define latb 1.57079633
#define longb 0
double max_diff_gps=0.1;
uint8_t gps_ok;
uint8_t count_packets=0;
double absf(double x) { return (x>0)?x:(double)x*-1;}
static esp_err_t set_gps_diff(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
	cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"diff")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
	const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	double diff = cJSON_GetObjectItem(root, "diff")->valuedouble;

	if(is_admin(key)==77){
		max_diff_gps=diff;
        count_packets=0;
	}

	cJSON_Delete(root);
    httpd_resp_sendstr(req, "{\"response\":1}");
    return ESP_OK;
}

esp_err_t check_gps(double prev[],double current[],uint8_t pck){
    esp_err_t retval=ESP_OK;
    if(pck>1){
        for(uint8_t i=0;i<2;i++){
            ESP_LOGI("gps","%d %f %f %f",i,prev[i],current[i],delta[i]);
            if(absf(absf(prev[i] - current[i])- delta[i])>max_diff_gps){
                retval = ESP_FAIL;
            }
        }
    }
    if(retval==ESP_OK){
        for(uint8_t i=0;i<2;i++){
            delta[i]=absf(prev[i]-current[i]);
        }
    }
    return retval;
}
void aiming(double lata, double longa, double altita, double latc, double longc, double altitc, double *azimut, double *elev) {
    ESP_LOGI("CALC GPS", "%f %f %f %f %f %f", lata,longa,altita,latc,longc,altitc);
    double rp = atan2(sqrt(pow(cos(latb) * sin(longb - longa), 2) + pow(cos(lata) * sin(latb) - sin(lata) * cos(latb) * cos(absf(longb - longa)), 2)), sin(lata) * sin(latb) + cos(lata) * cos(latb) * cos(absf(longb - longa)));
    double rs = atan2(sqrt(pow(cos(latc) * sin(longc - longa), 2) + pow(cos(lata) * sin(latc) - sin(lata) * cos(latc) * cos(absf(longc - longa)), 2)), sin(lata) * sin(latc) + cos(lata) * cos(latc) * cos(absf(longc - longa)));
    double ps = atan2(sqrt(pow(cos(latc) * sin(longc - longb), 2) + pow(cos(latb) * sin(latc) - sin(latb) * cos(latc) * cos(absf(longc - longb)), 2)), sin(latb) * sin(latc) + cos(latb) * cos(latc) * cos(absf(longc - longb)));
    ESP_LOGI("CALC GPS", "%f %f %f", rp, rs, ps);
    *azimut = (acos((cos(ps) - cos(rs) * cos(rp)) / (sin(rs) * sin(rp)))) * 180 / 3.141592;
    ESP_LOGI("CALC GPS", "%f", *azimut);
    if (longc < longa) *azimut *= -1;

    if (rs > 0.002 && altitc > 2000)
    {
        double s = sqrt(pow(altita / 1000.0 + 6371.0, 2) + pow(altitc / 1000.0 + 6371.0, 2) - 2.0 * (altita / 1000.0 + 6371.0) * (altitc / 1000.0 + 6371.0) * cos(rs));
        *elev = 90 - (asin(sin(rs) * (altita / 1000.0 + 6371.0) / s) + rs) * 180.0 / 3.141592;
    } else {
        double s = rs * 6371;
        *elev = atan((altitc - altita) / 1000 / s) * 180 / 3.141592;
    }
}
static esp_err_t home_set_gps(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
	cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"lat") || !cJSON_HasObjectItem(root,"lon") || !cJSON_HasObjectItem(root,"height")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
	const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	double lat = cJSON_GetObjectItem(root, "lat")->valuedouble;
	double lon = cJSON_GetObjectItem(root, "lon")->valuedouble;
    double height = cJSON_GetObjectItem(root, "height")->valuedouble;

	if(is_admin(key)==77){
		receiver[0]=lat;
        receiver[1]=lon;
        receiver[2]=height;
		ESP_LOGI("Home coords","receiver: %f %f %f\r\n",(float)lat,(float)lon,(float)height);
	}

	cJSON_Delete(root);
    httpd_resp_sendstr(req, "{\"response\":1}");
    return ESP_OK;
}
#define PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111745028410270193852110555964462294895493038196442881097566593344612847564823378678316527120190914564856692346034861045432664821339360726024914127372458700660631558817488152092096282925409171536436789259036001133053054882046652138414695194151160943305727036575959195309218611738193261179310511854807446237996274956735188575272489122793818301194912983367336244065664308602139494639522473719070217986094370277053921717629317675238467481846766940513200056812714526356082778577134275778960917363717872146844090122495343014654958537105079227968925892354201995611212902196086403441815981362977477130996051870721134999999
#define radians(x)  ( x*PI/180 )

static esp_err_t sat_set_gps(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));
	cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"lat") || !cJSON_HasObjectItem(root,"lon") || !cJSON_HasObjectItem(root,"height")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
	const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	double lat = cJSON_GetObjectItem(root, "lat")->valuedouble;
	double lon = cJSON_GetObjectItem(root, "lon")->valuedouble;
    double height = cJSON_GetObjectItem(root, "height")->valuedouble;

	if(is_admin(key)==77){
        if(gps_ok==1){
            for(uint8_t i=0;i<2;i++){
                prev_sputnic[i]=sputnic[i];
            }
        }
		sputnic[0]=lat;
        sputnic[1]=lon;
        sputnic[2]=height;
		ESP_LOGI("SAT coords","sputnic: %f %f %f\r\n",lat,lon,height);
        if (1)//true || check_gps(prev_sputnic,sputnic,count_packets)==ESP_OK)
        {
            gps_ok=1;
            if(receiver[0]!=0){

                aiming(radians(receiver[0]), radians(receiver[1]), receiver[2], radians(sputnic[0]), radians(sputnic[1]), sputnic[2], &angle[0], &angle[1]);
                ESP_LOGI("CALC GPS","%f %f",angle[0], angle[1]);
                absolute_stepper(0,angle_to_steps(angle[0]-delta_angleX));
                absolute_stepper(1,angle_to_steps(angle[1]-delta_angleY));
            }
        }
        else{
            gps_ok=0;
            ESP_LOGE("GPS ERROR","CHECK LOGS ABOVE!");
        }
        if(count_packets<3){
            count_packets++;
        }
	}

	cJSON_Delete(root);
    httpd_resp_sendstr(req, "{\"response\":1}");
    return ESP_OK;
}

static esp_err_t set_dorotate(httpd_req_t *req)
{

    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"value")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	uint8_t dorotate = cJSON_GetObjectItem(root, "value")->valueint;


    if(is_admin(key)==77){
        do_rotate_set(dorotate);
    	nvs_handle_t ws = open_nvs("settings", NVS_READWRITE);
        check_err(nvs_set_u8(ws, "dorotate", dorotate));
        nvs_commit(ws);
        nvs_close(ws);
    	httpd_resp_sendstr(req, "{\"response\":1}");
    }
    else{
    	httpd_resp_sendstr(req, "No");
    }

    cJSON_Delete(root);
    return ESP_OK;
}


static esp_err_t adduser(httpd_req_t *req)
{
	char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
	ESP_ERROR_CHECK(get_buf_from_request(req,buf));

	cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
	const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
    set_user(key,0);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "{\"response\":1}");
    return ESP_OK;
}

static esp_err_t anglesdatatx(httpd_req_t *req)
{
	 httpd_resp_set_type(req, "application/json");
	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "azimut", as5600_getAngleX()+delta_angleX);
	cJSON_AddNumberToObject(root, "elevation", as5600_getAngleY()+delta_angleY);
	cJSON_AddNumberToObject(root, "voltage", 0.00);
	cJSON_AddNumberToObject(root, "setted_azimut", (float)get_pos(0)/STEPS_PER_ROTATION/STEPPERS_MICROSTEP/STEPPERS_GEAR_RATIO*360+delta_angleX);
	cJSON_AddNumberToObject(root, "setted_elevation", (float)get_pos(1)/STEPS_PER_ROTATION/STEPPERS_MICROSTEP/STEPPERS_GEAR_RATIO*360+delta_angleY);
    cJSON_AddNumberToObject(root, "is_ready", motor_isready());
    cJSON_AddNumberToObject(root, "delta_enabled", (delta_angleX==0&&delta_angleY==0)?0:1);
    cJSON_AddNumberToObject(root, "dorotate_enabled", do_rotate_get());
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
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"login") || !cJSON_HasObjectItem(root,"password")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
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
    if (!cJSON_HasObjectItem(root,"key")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
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
    if (!cJSON_HasObjectItem(root,"key")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;

    switch(is_admin(key)){
    case 77:
    	clear_nvs();
    	httpd_resp_sendstr(req, "{\"response\":1}");
    	break;
    default:
    	httpd_resp_sendstr(req, "NONONO");
        break;
    }
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t reset_as5600(httpd_req_t *req)
{
    if(!motor_isready()){
        httpd_resp_sendstr(req, "MOTOR IS NOT READY!");
        return ESP_OK;
    }
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
    if(is_admin(key)==77){
    	reset_all_positions();
    	httpd_resp_sendstr(req, "{\"response\":1}");
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
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"azimut") || !cJSON_HasObjectItem(root,"elevation")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	double azimut = cJSON_GetObjectItem(root, "azimut")->valuedouble;
	double elevation = cJSON_GetObjectItem(root, "elevation")->valuedouble;


    if(is_admin(key)==77){
    	delta_angleX=azimut;
    	delta_angleY=elevation;
    	printf("newdeltaX:%f %f\r\n",delta_angleX,delta_angleY);
    	httpd_resp_sendstr(req, "{\"response\":1}");
    }
    else{
    	httpd_resp_sendstr(req, "No");
    }

    cJSON_Delete(root);
    return ESP_OK;
}
static esp_err_t wifi_mode_set(httpd_req_t *req)
{

    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key") || !cJSON_HasObjectItem(root,"mode")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
	int8_t mode = cJSON_GetObjectItem(root, "mode")->valueint;


    if(is_admin(key)==77){
        set_wifi(mode);
        //set_pr_state(0);
    	httpd_resp_sendstr(req, "{\"response\":1}");
    }
    else{
    	httpd_resp_sendstr(req, "No");
    }

    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t restart_esp(httpd_req_t *req)
{

    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    ESP_ERROR_CHECK(get_buf_from_request(req,buf));
    cJSON *root = cJSON_Parse(buf);
    if (!cJSON_HasObjectItem(root,"key")){
        httpd_resp_sendstr(req, "{\"error\":\"args\"}");
        return ESP_FAIL;
    }
    const char * key = cJSON_GetObjectItem(root, "key")->valuestring;
    if(is_admin(key)==77){
        esp_restart();
    	httpd_resp_sendstr(req, "{\"response\":1}");
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
    config.max_uri_handlers = 16;
    //config.max_open_sockets=13;
    config.lru_purge_enable = true;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed");
    //ESP_ERROR_CHECK(httpd_start(&server, &config));

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

    httpd_uri_t data_set_homegps_uri = {
				.uri = "/api/v1/data/set/homegps",
				.method = HTTP_POST,
				.handler = home_set_gps,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &data_set_homegps_uri);

    httpd_uri_t data_set_satgps_uri = {
				.uri = "/api/v1/data/set/satgps",
				.method = HTTP_POST,
				.handler = sat_set_gps,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &data_set_satgps_uri);

    httpd_uri_t data_set_diffgps_uri = {
				.uri = "/api/v1/data/set/diffgps",
				.method = HTTP_POST,
				.handler = set_gps_diff,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &data_set_diffgps_uri);

    httpd_uri_t data_set_dorotate_uri = {
				.uri = "/api/v1/data/set/dorotate",
				.method = HTTP_POST,
				.handler = set_dorotate,
				.user_ctx = rest_context
			};
    httpd_register_uri_handler(server, &data_set_dorotate_uri);

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

    httpd_uri_t data_reset_as_uri = {
				.uri = "/api/v1/reset/as5600/",
				.method = HTTP_POST,
				.handler = reset_as5600,
				.user_ctx = rest_context
			};
	httpd_register_uri_handler(server, &data_reset_as_uri);

    httpd_uri_t data_set_wifi_mode_uri = {
				.uri = "/api/v1/data/set/wifi_mode",
				.method = HTTP_POST,
				.handler = wifi_mode_set,
				.user_ctx = rest_context
			};
	httpd_register_uri_handler(server, &data_set_wifi_mode_uri);


    httpd_uri_t data_set_system_restart_uri = {
				.uri = "/api/v1/system/restart",
				.method = HTTP_POST,
				.handler = restart_esp,
				.user_ctx = rest_context
			};
	httpd_register_uri_handler(server, &data_set_system_restart_uri);

    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &common_get_uri);


    init_steppers();
    //init_i2c(21,45,48,47); //new
    init_i2c(7,6,19,8); //test board
    start_monitoring_AS5600();
    ESP_LOGI("RAD","%.51f",radians(180));
    return ESP_OK;

}
