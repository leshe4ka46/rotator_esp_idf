#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
esp_err_t init_nvs(void)
{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	   // NVS partition was truncated and needs to be erased
	   // Retry nvs_flash_init
	   ESP_ERROR_CHECK(nvs_flash_erase());
	   err = nvs_flash_init();
	 }
	 ESP_ERROR_CHECK( err );
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
    /*if (role == 0)
    {
        int8_t temp;
        user_nfs_err = nvs_get_i8(userdata, uid, &temp);
        if (user_nfs_err == ESP_ERR_NVS_NOT_FOUND)
        {
            printf("The value is not initialized yet!\n");
            nvs_set_i8(userdata, uid, 0);
        }
    }
    else
    {
        nvs_set_i8(userdata, uid, 200);
    }*/
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
        set_user(uid,0);
        return 0;
        break;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(user_nfs_err));
    }
    /*user_nfs_err = nvs_commit(userdata);
    if (user_nfs_err != ESP_OK)
    {
        printf("Error (%s) saving NVS!\n", esp_err_to_name(user_nfs_err));
        return -1;
    }*/
    // Close

    nvs_close(userdata);
    return role;
}
