#ifndef usb_bridge_tools_h
#define usb_bridge_tools_h

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#include "usb_bridge.h"

usb_bridge_error_t usb_bridge_read_ringbuffer(RingbufHandle_t ringbuffer, uint8_t *out_buffer, size_t out_buffer_size, size_t *read_data_size);

#endif
