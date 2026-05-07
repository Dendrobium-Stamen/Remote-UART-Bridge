
if(IDF_TARGET STREQUAL "esp32s2" OR IDF_TARGET STREQUAL "esp32s3" OR
   IDF_TARGET STREQUAL "esp32p4"  OR IDF_TARGET STREQUAL "esp32h4")
idf_component_register(SRC_DIRS "."
                                "./tools"
                    INCLUDE_DIRS "./include"
                                 "./tools"
                    PRIV_REQUIRES espressif__esp_tinyusb espressif__tinyusb freertos esp_ringbuf)
endif()
