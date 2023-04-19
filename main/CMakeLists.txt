idf_component_register(SRCS
                            "rest_server.c"
                            "rotator_main.c"
                            "as5600_lib.c"
                            "nvs_logic.c"
                            "dnsServer.c"
                            "stepper_motor_encoder.c"
                            "stepper_logic.c"
                            INCLUDE_DIRS ".")

#if(CONFIG_EXAMPLE_WEB_DEPLOY_SF)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
    endif()
#endif()
