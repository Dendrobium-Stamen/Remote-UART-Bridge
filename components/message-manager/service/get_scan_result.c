#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_scan_result_t *message_manager_get_scan_result(uint64_t wait_ms)
{
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
    return &message_manager.scan_result;
}
