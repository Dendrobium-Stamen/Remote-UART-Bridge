#include "stdio.h"
#include "string.h"

#include "esp_mac.h"

#include "web_tools.h"

void web_tools_mac_to_str(const uint8_t *mac, char *out)
{
    snprintf(out, WEB_TOOLS_MAC_TO_STR_LENGTH, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
