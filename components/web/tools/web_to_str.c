#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

bool web_tools_str_to_mac(const char *str, uint8_t *out)
{
    if (!str)
        return false;
    unsigned int tmp[6];
    if (sscanf(str, "%2X:%2X:%2X:%2X:%2X:%2X",
               &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]) != 6)
        return false;
    for (int i = 0; i < 6; i++)
    {
        if (tmp[i] > 0xFF)
            return false;
        out[i] = (uint8_t)tmp[i];
    }
    return true;
}
