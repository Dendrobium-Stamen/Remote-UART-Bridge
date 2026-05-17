#include "stdio.h"

#include "esp_log.h"
#include "esp_err.h"
#include "driver/temperature_sensor.h"

static const char *TAG = "TEMP_SENSOR";

temperature_sensor_handle_t temp_handle = NULL;

void espstate_monitor_chip_temperature_init()
{
    temperature_sensor_config_t temp_sensor_config = {
        .range_min = -10,
        .range_max = 80,
        .clk_src = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT};

    esp_err_t ret = temperature_sensor_install(&temp_sensor_config, &temp_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "温度传感器安装失败");
        return;
    }

    ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
}

float espstate_monitor_chip_temperature_read()
{
    float tsens_out;
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));

    return tsens_out;
}
