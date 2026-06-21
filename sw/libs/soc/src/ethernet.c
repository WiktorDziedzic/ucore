#include <ethernet.h>
#include <memory_map.h>
#include <reg.h>

#define ETHERNET_CTRL_ADDRESS (ETHERNET_BASE_ADDRESS + 0x000)

void ethernet_trigger(void)
{
    reg_write(ETHERNET_CTRL_ADDRESS, 0x00000001);
}