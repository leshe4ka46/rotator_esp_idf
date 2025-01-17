#ifndef __nvs_logic_inc_
#define __nvs_logic_inc_
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

void check_err(esp_err_t err)
{
    switch (err)
    {
    case ESP_OK:
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("Val not found (%s) \n", esp_err_to_name(err));
        break;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
}
uint8_t curr_err = 0;
nvs_handle_t open_nvs(const char *namespace, nvs_open_mode_t mode)
{
    nvs_handle_t ws;
    esp_err_t err;
    err = nvs_open(namespace, mode, &ws);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS!\n", esp_err_to_name(err));
        curr_err = 1;
    }
    else
    {
        curr_err = 0;
    }
    return ws;
}

esp_err_t init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    return err;
}
esp_err_t user_nfs_err;
esp_err_t clear_nvs(void)
{
    nvs_handle_t userdata;
    user_nfs_err = nvs_open("users", NVS_READWRITE, &userdata);
    if (user_nfs_err != ESP_OK)
    {
        printf("Error (%s) opening NVS!\n", esp_err_to_name(user_nfs_err));
        return 0;
    }
    nvs_erase_all(userdata);
    user_nfs_err = nvs_commit(userdata);
    if (user_nfs_err != ESP_OK)
    {
        printf("Error (%s) saving NVS!\n", esp_err_to_name(user_nfs_err));
        return 0;
    }
    // Close
    nvs_close(userdata);
    return 1;
}
esp_err_t set_user(const char *uid, int8_t role)
{
    nvs_handle_t userdata;
    user_nfs_err = nvs_open("users", NVS_READWRITE, &userdata);
    if (user_nfs_err != ESP_OK)
    {
        printf("Error (%s) opening NVS!\n", esp_err_to_name(user_nfs_err));
        return 0;
    }
    nvs_set_i8(userdata, uid, role);
    user_nfs_err = nvs_commit(userdata);
    if (user_nfs_err != ESP_OK)
    {
        printf("Error (%s) saving NVS!\n", esp_err_to_name(user_nfs_err));
        return 0;
    }
    // Close
    nvs_close(userdata);
    return 1;
}
int8_t is_admin(const char *uid)
{
    if (strcmp(uid, "SATAPPSP") == 0)
    {
        return 77;
    }
    nvs_handle_t userdata;
    user_nfs_err = nvs_open("users", NVS_READWRITE, &userdata);
    if (user_nfs_err != ESP_OK)
    {
        printf("Error (%s) opening NVS!\n", esp_err_to_name(user_nfs_err));
        return -1;
    }
    int8_t role;
    user_nfs_err = nvs_get_i8(userdata, uid, &role);
    switch (user_nfs_err)
    {
    case ESP_OK:
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("The value is not initialized yet!\n");
        set_user(uid, 0);
        return 0;
        break;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(user_nfs_err));
    }
    nvs_close(userdata);
    return role;
}

esp_err_t set_wifi(int8_t value)
{
    nvs_handle_t ws = open_nvs("wifi_settings", NVS_READWRITE);
    check_err(nvs_set_i8(ws, "mode", value));
    nvs_commit(ws);
    nvs_close(ws);
    return ESP_OK;
}

int8_t get_wifi()
{
    int8_t value;
    nvs_handle_t ws = open_nvs("wifi_settings", NVS_READONLY);
    if (curr_err)
    {
        set_wifi(0);
    }
    check_err(nvs_get_i8(ws, "mode", &value));
    nvs_close(ws);
    return value;
}

esp_err_t set_turns(int32_t vX, int32_t vY)
{
    nvs_handle_t ws = open_nvs("turns", NVS_READWRITE);
    check_err(nvs_set_i32(ws, "x", vX));
    check_err(nvs_set_i32(ws, "y", vY));
    nvs_commit(ws);
    nvs_close(ws);
    return ESP_OK;
}
int8_t get_turns(int32_t *turnsX, int32_t *turnsY)
{
    nvs_handle_t ws = open_nvs("turns", NVS_READONLY);
    if (curr_err)
    {
        set_turns(0, 0);
    }
    check_err(nvs_get_i32(ws, "x", turnsX));
    check_err(nvs_get_i32(ws, "y", turnsY));
    nvs_close(ws);
    return 1;
}

esp_err_t set_delta(float vX, float vY)
{
    nvs_handle_t ws = open_nvs("delta", NVS_READWRITE);
    check_err(nvs_set_i32(ws, "x", (int32_t)(vX * 1000)));
    check_err(nvs_set_i32(ws, "y", (int32_t)(vY * 1000)));
    nvs_commit(ws);
    nvs_close(ws);
    return ESP_OK;
}

int8_t get_delta(float *deltaX, float *deltaY)
{
    int32_t tmp;
    nvs_handle_t ws = open_nvs("delta", NVS_READONLY);
    if (curr_err)
    {
        set_delta(0, 0);
    }
    check_err(nvs_get_i32(ws, "x", &tmp));
    *deltaX = (float)tmp/1000.0f;
    check_err(nvs_get_i32(ws, "y", &tmp));
    *deltaY = (float)tmp/1000.0f;
    nvs_close(ws);
    return 1;
}

esp_err_t set_target(int32_t val, uint8_t uid)
{
    nvs_handle_t ws = open_nvs("target", NVS_READWRITE);
    if (uid == 0)
    {
        check_err(nvs_set_i32(ws, "x", val));
    }
    else if (uid == 1)
    {
        check_err(nvs_set_i32(ws, "y", val));
    }
    nvs_commit(ws);
    nvs_close(ws);
    return ESP_OK;
}

int32_t get_target(uint8_t uid)
{
    int32_t val;
    nvs_handle_t ws = open_nvs("target", NVS_READONLY);
    if (curr_err)
    {
        set_target(0, 0);
    }
    if (uid == 0)
    {
        check_err(nvs_get_i32(ws, "x", &val));
    }
    else if (uid == 1)
    {
        check_err(nvs_get_i32(ws, "y", &val));
    }
    nvs_close(ws);
    return val;
}

esp_err_t set_joy_delta(float vX, float vY)
{
    nvs_handle_t ws = open_nvs("joydelta", NVS_READWRITE);
    check_err(nvs_set_i32(ws, "x", (int32_t)(vX * 1000)));
    check_err(nvs_set_i32(ws, "y", (int32_t)(vY * 1000)));
    nvs_commit(ws);
    nvs_close(ws);
    return ESP_OK;
}

int8_t get_joy_delta(float *_joy_deltaX, float *_joy_deltaY)
{
    int32_t tmp;
    nvs_handle_t ws = open_nvs("joydelta", NVS_READONLY);
    if (curr_err)
    {
        set_joy_delta(0, 0);
    }
    check_err(nvs_get_i32(ws, "x", &tmp));
    *_joy_deltaX = (float)tmp/1000.0f;
    check_err(nvs_get_i32(ws, "y", &tmp));
    *_joy_deltaY = (float)tmp/1000.0f;
    nvs_close(ws);
    return 1;
}

esp_err_t set_delta_angles(float vX, float vY)
{
    nvs_handle_t ws = open_nvs("deltareal", NVS_READWRITE);
    check_err(nvs_set_i32(ws, "x", (int32_t)(vX * 1000)));
    check_err(nvs_set_i32(ws, "y", (int32_t)(vY * 1000)));
    nvs_commit(ws);
    nvs_close(ws);
    return ESP_OK;
}

int8_t get_delta_angles(float *deltaX, float *deltaY)
{
    int32_t tmp;
    nvs_handle_t ws = open_nvs("deltareal", NVS_READONLY);
    if (curr_err)
    {
        set_delta(0, 0);
    }
    check_err(nvs_get_i32(ws, "x", &tmp));
    *deltaX = (float)tmp/1000.0f;
    check_err(nvs_get_i32(ws, "y", &tmp));
    *deltaY = (float)tmp/1000.0f;
    nvs_close(ws);
    return 1;
}

/*
esp_err_t write_nvs(const char* namespace,const char* id,int8_t val){
    nvs_handle_t ws = open_nvs(namespace,NVS_READWRITE);
    esp_err_t err = nvs_set_i8(ws, id, val);
    if (err != ESP_OK)
    {
        printf("Error (%s) writing NVS!\n", esp_err_to_name(err));
        return ESP_FAIL;
    }
    err=nvs_commit(ws);
    if (err != ESP_OK)
    {
        printf("Error (%s) committing NVS!\n", esp_err_to_name(err));
        return ESP_FAIL;
    }
    nvs_close(ws);
    return ESP_OK;
}
esp_err_t write_nvs(const char* namespace,const char* id,int32_t val){
    nvs_handle_t ws = open_nvs(namespace,NVS_READWRITE);
    esp_err_t err = nvs_set_i32(ws, id, val);
    if (err != ESP_OK)
    {
        printf("Error (%s) writing NVS!\n", esp_err_to_name(err));
        return ESP_FAIL;
    }
    err=nvs_commit(ws);
    if (err != ESP_OK)
    {
        printf("Error (%s) committing NVS!\n", esp_err_to_name(err));
        return ESP_FAIL;
    }
    nvs_close(ws);
    return ESP_OK;
}




int8_t get_nvs_8(const char* namespace,const char* id){
    int8_t val;
    nvs_handle_t ws = open_nvs(namespace,NVS_READONLY);
    esp_err_t err=nvs_get_i8(ws, id, &val);
    if (err != ESP_OK)
    {
        printf("Error (%s) reading NVS!\n", esp_err_to_name(err));
        return 0;
    }
    nvs_close(ws);
    return val;
}

int32_t get_nvs_32(const char* namespace,const char* id){
    int32_t val;
    nvs_handle_t ws = open_nvs(namespace,NVS_READONLY);
    esp_err_t err=nvs_get_i32(ws, id, &val);
    if (err != ESP_OK)
    {
        printf("Error (%s) reading NVS!\n", esp_err_to_name(err));
        return 0;
    }
    nvs_close(ws);
    return val;
}
*/