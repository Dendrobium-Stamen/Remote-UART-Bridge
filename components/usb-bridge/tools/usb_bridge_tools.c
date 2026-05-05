#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#include "usb_bridge.h"

usb_bridge_error_t usb_bridge_read_ringbuffer(RingbufHandle_t ringbuffer, uint8_t *out_buffer, size_t out_buffer_size, size_t *read_data_size)
{
    size_t read_size;
    uint8_t *buffer;

    buffer = xRingbufferReceiveUpTo(ringbuffer, &read_size, pdTICKS_TO_MS(10), out_buffer_size);

    if (buffer != NULL)
    {
        memcpy(out_buffer, buffer, read_size);
        vRingbufferReturnItem(ringbuffer, (void *)buffer);
        *read_data_size = read_size;

        while (*read_data_size < out_buffer_size)
        {
            buffer = xRingbufferReceiveUpTo(ringbuffer, &read_size, 0, out_buffer_size - *read_data_size);
            if (buffer == NULL)
                break;
            memcpy(out_buffer + *read_data_size, buffer, read_size);
            vRingbufferReturnItem(ringbuffer, (void *)buffer);
            *read_data_size += read_size;
        }
    }
    else
        return USB_BRIDGE_READ_RINGBUFFER_ERROR;

    return USB_BRIDGE_OK;
}
