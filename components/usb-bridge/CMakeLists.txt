if(CONFIG_IDF_TARGET_ESP32S2 OR CONFIG_IDF_TARGET_ESP32S3 OR
   CONFIG_IDF_TARGET_ESP32P4 OR CONFIG_IDF_TARGET_ESP32H4)

idf_component_register(SRC_DIRS "."
                                "./tools"
                    INCLUDE_DIRS "./include"
                                 "./tools"
                    REQUIRES espressif__esp_tinyusb espressif__tinyusb freertos esp_ringbuf)


endif()
