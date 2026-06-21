#include <soc/gpio.h>
#include <soc/uart.h>
#include <soc/ethernet.h>

int main(void)
{
    uart_init();

    while (1) {
        for (int i = 0; i < 16; ++i) {
            gpio_set_dout(i);

            ethernet_trigger();

            uart_write((i & 0x1) ? "pong\n" : "ping\n");

            for (int i = 0; i < 1000000; ++i)
                asm volatile ("nop");
        }
    }
}