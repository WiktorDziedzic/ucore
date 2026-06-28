#include <soc/gpio.h>
#include <soc/uart.h>
#include <soc/ethernet.h>

int main(void)
{
    uart_init();

    while (1) {
        for (int i = 0; i < 16; ++i) {
            gpio_set_dout(i);

            uint8_t payload[] = {
                0xde, 0xad, 0xbe, 0xef,
                0x01, 0x02, 0x03, 0x04
            };

            ethernet_disable_loopback();

            ethernet_set_dst_mac(0xffffffffffffull);
            ethernet_set_src_mac(0x123400000002ull);
            ethernet_set_ethertype(0x0800);

            ethernet_set_ipv4_src(ETHERNET_IPV4_ADDR(192, 168, 1, 2));
            ethernet_set_ipv4_dst(ETHERNET_IPV4_ADDR(192, 168, 1, 90));
            ethernet_set_ipv4_cfg0(64, 17, 0);
            ethernet_set_ipv4_cfg1(0, 2, 0);
            ethernet_set_ipv4_checksum(0);

            ethernet_set_udp_ports(1234, 1235);
            ethernet_set_udp_length(0);
            ethernet_set_udp_checksum(0);

            ethernet_set_tx_payload(payload, sizeof(payload));
            ethernet_set_ifg_cycles(100);
            ethernet_set_frame_count(1);

            ethernet_trigger();

            uart_write((i & 0x1) ? "pong\r\n" : "ping\r\n");

            for (int i = 0; i < 1000000; ++i)
                asm volatile ("nop");
        }
    }
}
