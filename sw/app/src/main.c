#include <stdint.h>

#include <soc/gpio.h>
#include <soc/uart.h>
#include <soc/ethernet.h>

#define MAX_PAYLOAD_BYTES 128

static uint32_t read_uint(const char *prompt)
{
    char buf[32];
    uint32_t value = 0;

    uart_write(prompt);

    if (uart_read(buf, sizeof(buf)) != 0) {
        uart_write("invalid input\r\n");
        return 0;
    }

    for (int i = 0; buf[i] != '\0' && buf[i] != '\r' && buf[i] != '\n'; ++i) {
        if (buf[i] >= '0' && buf[i] <= '9')
            value = value * 10 + (uint32_t)(buf[i] - '0');
    }

    return value;
}

static uint32_t prng_next(uint32_t x)
{
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static void fill_random_payload(uint8_t *payload, uint32_t len, uint32_t *seed)
{
    uint32_t rnd = *seed;

    for (uint32_t i = 0; i < len; ++i) {
        rnd = prng_next(rnd);
        payload[i] = (uint8_t)(rnd & 0xff);
    }

    *seed = rnd;
}

static void delay_cycles(uint32_t cycles)
{
    for (volatile uint32_t i = 0; i < cycles; ++i) {
        asm volatile ("nop");
    }
}

static void configure_and_send_single_frame(uint8_t *payload,
                                            uint32_t payload_len,
                                            uint32_t ifg_cycles)
{
    ethernet_disable_loopback();

    ethernet_set_dst_mac(0xffffffffffffull);
    ethernet_set_src_mac(0x123400000002ull);
    ethernet_set_ethertype(0x0800);

    ethernet_set_ipv4_src(ETHERNET_IPV4_ADDR(192, 168, 1, 2));
    ethernet_set_ipv4_dst(ETHERNET_IPV4_ADDR(192, 168, 1, 90));
    ethernet_set_ipv4_cfg0(64, 17, 0);
    ethernet_set_ipv4_cfg1(0, 2, 0);
    ethernet_set_ipv4_checksum(0);

    ethernet_set_udp_ports(50000, 50001);
    ethernet_set_udp_length(0);
    ethernet_set_udp_checksum(0);

    ethernet_set_tx_payload(payload, payload_len);

    ethernet_set_ifg_cycles(ifg_cycles);
    ethernet_set_frame_count(1);

    ethernet_trigger();

    delay_cycles(ifg_cycles);
}

int main(void)
{
    uint8_t payload[MAX_PAYLOAD_BYTES];
    uint32_t seed = 0x12345678;
    int led = 0;

    uart_init();

    uart_write("\r\nucore ethernet cpu-driven generator\r\n");

    while (1) {
        uint32_t payload_len;
        uint32_t ifg_cycles;
        uint32_t frame_count;

        gpio_set_dout(led++);

        payload_len  = read_uint("\r\nPayload bytes: ");
        ifg_cycles   = read_uint("IFG cycles: ");
        frame_count  = read_uint("Frame count: ");

        if (payload_len == 0)
            payload_len = 1;

        if (payload_len > MAX_PAYLOAD_BYTES)
            payload_len = MAX_PAYLOAD_BYTES;

        if (frame_count == 0)
            frame_count = 1;

        uart_write("TX started\r\n");

        for (uint32_t frame_idx = 0; frame_idx < frame_count; ++frame_idx) {
            fill_random_payload(payload, payload_len, &seed);
            configure_and_send_single_frame(payload, payload_len, ifg_cycles);
        }

        uart_write("TX finished\r\n");
    }
}
