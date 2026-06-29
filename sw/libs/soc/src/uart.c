#include <uart.h>
#include <memory_map.h>
#include <reg.h>

#define UART_CR_ADDRESS                (UART_BASE_ADDRESS + 0x000)
#define UART_SR_ADDRESS                (UART_BASE_ADDRESS + 0x004)
#define UART_CCR_ADDRESS               (UART_BASE_ADDRESS + 0x008)
#define UART_WDR_ADDRESS               (UART_BASE_ADDRESS + 0x00c)
#define UART_RDR_ADDRESS               (UART_BASE_ADDRESS + 0x010)

#define UART_CR_EN_bp                  0
#define UART_CR_EN_bm                  (0x1 << UART_CR_EN_bp)

#define UART_SR_RXNE_bp                0
#define UART_SR_RXNE_bm                (0x1 << UART_SR_RXNE_bp)
#define UART_SR_TX_BUSY_bp             1
#define UART_SR_TX_BUSY_bm             (0x1 << UART_SR_TX_BUSY_bp)

#define UART_CCR_DIVIDER_bp            0
#define UART_CCR_DIVIDER_bm            (0xff << UART_CCR_DIVIDER_bp)
#define UART_CCR_EDGES_PER_BIT_bp      8
#define UART_CCR_EDGES_PER_BIT_bm      (0xff << UART_CCR_EDGES_PER_BIT_bp)

#define UART_WDR_WDATA_bp              0
#define UART_WDR_WDATA_bm              (0xff << UART_WDR_WDATA_bp)

#define UART_RDR_RDATA_bp              0
#define UART_RDR_RDATA_bm              (0xff << UART_RDR_RDATA_bp)

static uint8_t uart_is_receiver_ready(void);
static uint8_t uart_get_rdata(void);
static uint8_t uart_is_transmitter_busy(void);
static void uart_set_wdata(uint8_t val);

void uart_init(void)
{
    reg_write_bits(UART_CCR_ADDRESS, UART_CCR_DIVIDER_bp, UART_CCR_DIVIDER_bm, 26);
    reg_write_bits(UART_CCR_ADDRESS, UART_CCR_EDGES_PER_BIT_bp, UART_CCR_EDGES_PER_BIT_bm, 16);
    reg_write_bits(UART_CR_ADDRESS, UART_CR_EN_bp, UART_CR_EN_bm, 1);
}

uint8_t uart_read_byte(void)
{
    while (!uart_is_receiver_ready()) { }
    return uart_get_rdata();
}

int uart_read(char *dest, int len)
{
    int pos = 0;

    while (pos < len - 1) {
        char c = uart_read_byte();

        if (c == '\r' || c == '\n') {
            dest[pos] = '\0';
            uart_write("\r\n");
            return 0;
        }

        if (c == '\b' || c == 127) {
            if (pos > 0) {
                --pos;
                uart_write("\b \b");
            }
            continue;
        }

        dest[pos++] = c;
        uart_write_byte(c);
    }

    dest[pos] = '\0';
    return 1;
}

void uart_write_byte(uint8_t val)
{
    while (uart_is_transmitter_busy()) { }
    uart_set_wdata(val);
}

void uart_write(const char *src)
{
    while (*src)
        uart_write_byte(*src++);
}

void uart_flush(void)
{
    while (uart_is_transmitter_busy()) { }
}

uint8_t uart_is_receiver_ready(void)
{
    return reg_read_bits(UART_SR_ADDRESS, UART_SR_RXNE_bp, UART_SR_RXNE_bm);
}

uint8_t uart_get_rdata(void)
{
    return reg_read_bits(UART_RDR_ADDRESS, UART_RDR_RDATA_bp, UART_RDR_RDATA_bm);
}

uint8_t uart_is_transmitter_busy(void)
{
    return reg_read_bits(UART_SR_ADDRESS, UART_SR_TX_BUSY_bp, UART_SR_TX_BUSY_bm);
}

void uart_set_wdata(uint8_t val)
{
    reg_write_bits(UART_WDR_ADDRESS, UART_WDR_WDATA_bp, UART_WDR_WDATA_bm, val);
}
