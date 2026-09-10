#include <stdbool.h>
#include <stdint.h>

#include <soc/ethernet.h>
#include <soc/gpio.h>
#include <soc/timer.h>
#include <soc/uart.h>

#define MAX_MEASUREMENT_FRAMES    10000u
#define ETHERNET_BUSY_POLL_LIMIT 1000000u
#define TIMER_CYCLE_NS                25u
#define PAYLOAD_SEED           0x12345678u

typedef struct {
    ethernet_frame_mode_t mode;
    uint32_t payload_len;
    uint32_t ifg_cycles;
    uint32_t frame_count;
} measurement_config_t;

typedef struct {
    uint32_t all_csr_cycles;
    uint32_t data_csr_cycles;
    uint32_t full_cpu_cycles;
    uint32_t data_cpu_cycles;
    uint32_t hardware_cycles;
} measurement_results_t;

static uint32_t read_uint(const char *prompt)
{
    char buf[32];
    uint32_t value = 0u;

    uart_write(prompt);

    if (uart_read(buf, sizeof(buf)) != 0) {
        uart_write("invalid input\r\n");
        return 0u;
    }

    for (int i = 0; buf[i] != '\0' && buf[i] != '\r' && buf[i] != '\n'; ++i) {
        if (buf[i] >= '0' && buf[i] <= '9')
            value = value * 10u + (uint32_t)(buf[i] - '0');
    }

    return value;
}

static void uart_write_uint(uint32_t value)
{
    char buf[11];
    uint32_t pos = sizeof(buf) - 1u;

    buf[pos] = '\0';

    do {
        --pos;
        buf[pos] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0u);

    uart_write(&buf[pos]);
}

static void print_three_digits(uint32_t value)
{
    if (value < 100u)
        uart_write("0");

    if (value < 10u)
        uart_write("0");

    uart_write_uint(value);
}

static void print_cycles(const char *name,
                         uint32_t total_cycles,
                         uint32_t frame_count,
                         uint32_t payload_len,
                         bool print_throughput)
{
    uint32_t average_cycles = total_cycles / frame_count;

    uart_write(name);
    uart_write("\r\n  total cycles: ");
    uart_write_uint(total_cycles);
    uart_write("\r\n  cycles/frame: ");
    uart_write_uint(average_cycles);
    uart_write("\r\n  time/frame [ns]: ");
    uart_write_uint(average_cycles * TIMER_CYCLE_NS);

    if (print_throughput && average_cycles != 0u) {
        uint32_t throughput_kbps =
            (payload_len * 8u * 40000u) / average_cycles;

        uart_write("\r\n  application throughput [Mbit/s]: ");
        uart_write_uint(throughput_kbps / 1000u);
        uart_write(".");
        print_three_digits(throughput_kbps % 1000u);
    }

    uart_write("\r\n");
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
        payload[i] = (uint8_t)(rnd & 0xffu);
    }

    *seed = rnd;
}

static measurement_config_t read_measurement_config(void)
{
    measurement_config_t config;
    uint32_t mode;

    mode = read_uint("\r\nMode (0 = modular, 1 = full frame bypass): ");
    config.mode = mode == 0u
        ? ETHERNET_FRAME_MODE_MODULAR
        : ETHERNET_FRAME_MODE_BYPASS;
    config.payload_len = read_uint("Payload bytes: ");
    config.ifg_cycles = read_uint("IFG cycles: ");
    config.frame_count = read_uint("Measurement frames: ");

    return config;
}

static void normalize_measurement_config(measurement_config_t *config)
{
    if (config->payload_len == 0u)
        config->payload_len = 1u;

    if (config->mode == ETHERNET_FRAME_MODE_BYPASS &&
        config->payload_len > ETHERNET_BYPASS_MAX_PAYLOAD_BYTES) {
        config->payload_len = ETHERNET_BYPASS_MAX_PAYLOAD_BYTES;
        uart_write("Bypass payload limited to ");
        uart_write_uint(ETHERNET_BYPASS_MAX_PAYLOAD_BYTES);
        uart_write(" bytes\r\n");
    }

    if (config->mode == ETHERNET_FRAME_MODE_MODULAR &&
        config->payload_len > ETHERNET_TX_MAX_BYTES) {
        config->payload_len = ETHERNET_TX_MAX_BYTES;
    }

    if (config->frame_count == 0u)
        config->frame_count = 1u;

    if (config->frame_count > MAX_MEASUREMENT_FRAMES) {
        config->frame_count = MAX_MEASUREMENT_FRAMES;
        uart_write("Frame count limited to ");
        uart_write_uint(MAX_MEASUREMENT_FRAMES);
        uart_write("\r\n");
    }
}

static bool wait_for_transmission(void)
{
    if (!ethernet_wait_until_busy(ETHERNET_BUSY_POLL_LIMIT))
        return false;

    if (!ethernet_wait_until_idle(ETHERNET_BUSY_POLL_LIMIT))
        return false;

    return true;
}

static bool measure_all_csr_writes(const ethernet_frame_t *frame,
                                   uint32_t ifg_cycles,
                                   uint32_t frame_count,
                                   uint32_t *cycles)
{
    if (!ethernet_wait_until_idle(ETHERNET_BUSY_POLL_LIMIT))
        return false;

    uint32_t start = timer_get_value();

    for (uint32_t i = 0; i < frame_count; ++i) {
        if (!ethernet_prepare_frame(frame, ifg_cycles))
            return false;
    }

    *cycles = timer_get_value() - start;
    return true;
}

static bool measure_data_csr_writes(const ethernet_frame_t *frame,
                                    uint32_t ifg_cycles,
                                    uint32_t frame_count,
                                    uint32_t *cycles)
{
    if (!ethernet_wait_until_idle(ETHERNET_BUSY_POLL_LIMIT))
        return false;

    if (!ethernet_configure_frame_mode(frame->mode, ifg_cycles))
        return false;

    ethernet_set_frame_count(1u);

    uint32_t start = timer_get_value();

    for (uint32_t i = 0; i < frame_count; ++i) {
        if (!ethernet_load_frame(frame))
            return false;
    }

    *cycles = timer_get_value() - start;
    return true;
}

static bool measure_full_cpu_transmission(const ethernet_frame_t *frame,
                                          uint32_t ifg_cycles,
                                          uint32_t frame_count,
                                          uint32_t *cycles)
{
    if (!ethernet_wait_until_idle(ETHERNET_BUSY_POLL_LIMIT))
        return false;

    uint32_t start = timer_get_value();

    for (uint32_t i = 0; i < frame_count; ++i) {
        if (!ethernet_prepare_frame(frame, ifg_cycles))
            return false;

        ethernet_trigger();

        if (!wait_for_transmission())
            return false;
    }

    *cycles = timer_get_value() - start;
    return true;
}

static bool measure_data_cpu_transmission(const ethernet_frame_t *frame,
                                          uint32_t ifg_cycles,
                                          uint32_t frame_count,
                                          uint32_t *cycles)
{
    if (!ethernet_wait_until_idle(ETHERNET_BUSY_POLL_LIMIT))
        return false;

    if (!ethernet_configure_frame_mode(frame->mode, ifg_cycles))
        return false;

    ethernet_set_frame_count(1u);

    uint32_t start = timer_get_value();

    for (uint32_t i = 0; i < frame_count; ++i) {
        if (!ethernet_load_frame(frame))
            return false;

        ethernet_trigger();

        if (!wait_for_transmission())
            return false;
    }

    *cycles = timer_get_value() - start;
    return true;
}

static bool measure_hardware_transmission(const ethernet_frame_t *frame,
                                          uint32_t ifg_cycles,
                                          uint32_t frame_count,
                                          uint32_t *cycles)
{
    if (!ethernet_wait_until_idle(ETHERNET_BUSY_POLL_LIMIT))
        return false;

    if (!ethernet_configure_frame_mode(frame->mode, ifg_cycles))
        return false;

    if (!ethernet_load_frame(frame))
        return false;

    ethernet_set_frame_count(frame_count);

    uint32_t start = timer_get_value();

    ethernet_trigger();

    if (!wait_for_transmission())
        return false;

    *cycles = timer_get_value() - start;
    return true;
}

static bool run_measurements(const measurement_config_t *config,
                             const ethernet_frame_t *frame,
                             measurement_results_t *results)
{
    if (!ethernet_wait_until_idle(ETHERNET_BUSY_POLL_LIMIT)) {
        uart_write("ERROR: busy active before measurement\r\n");
        return false;
    }

    uart_write("Measurement started\r\n");

    if (!measure_all_csr_writes(
            frame,
            config->ifg_cycles,
            config->frame_count,
            &results->all_csr_cycles)) {
        uart_write("ERROR: all CSR write measurement failed\r\n");
        return false;
    }

    if (!measure_data_csr_writes(
            frame,
            config->ifg_cycles,
            config->frame_count,
            &results->data_csr_cycles)) {
        uart_write("ERROR: data CSR write measurement failed\r\n");
        return false;
    }

    if (!measure_full_cpu_transmission(
            frame,
            config->ifg_cycles,
            config->frame_count,
            &results->full_cpu_cycles)) {
        uart_write("ERROR: full CPU transmission measurement failed\r\n");
        return false;
    }

    if (!measure_data_cpu_transmission(
            frame,
            config->ifg_cycles,
            config->frame_count,
            &results->data_cpu_cycles)) {
        uart_write("ERROR: data CPU transmission measurement failed\r\n");
        return false;
    }

    if (!measure_hardware_transmission(
            frame,
            config->ifg_cycles,
            config->frame_count,
            &results->hardware_cycles)) {
        uart_write("ERROR: hardware transmission measurement failed\r\n");
        return false;
    }

    return true;
}

static void print_measurement_results(const measurement_config_t *config,
                                      const ethernet_frame_t *frame,
                                      const measurement_results_t *results)
{
    uint32_t transferred_bytes = frame->length;
    uint32_t transferred_words = (transferred_bytes + 3u) / 4u;

    uart_write("\r\n=== CONFIGURATION ===\r\n");
    uart_write("Mode: ");
    uart_write(config->mode == ETHERNET_FRAME_MODE_BYPASS
        ? "full frame bypass\r\n"
        : "modular\r\n");
    uart_write("Application payload [bytes]: ");
    uart_write_uint(config->payload_len);
    uart_write("\r\nData written to CSR [bytes]: ");
    uart_write_uint(transferred_bytes);
    uart_write("\r\nData words written per frame: ");
    uart_write_uint(transferred_words);
    uart_write("\r\nIFG [cycles]: ");
    uart_write_uint(config->ifg_cycles);
    uart_write("\r\nMeasurement frames: ");
    uart_write_uint(config->frame_count);
    uart_write("\r\n");

    uart_write("\r\n=== CSR WRITES ===\r\n");
    print_cycles(
        "All fields required by selected mode",
        results->all_csr_cycles,
        config->frame_count,
        config->payload_len,
        false
    );
    print_cycles(
        "Frame data and length only",
        results->data_csr_cycles,
        config->frame_count,
        config->payload_len,
        false
    );

    uart_write("\r\n=== TRANSMISSION ===\r\n");
    print_cycles(
        "CPU: full configuration before every frame",
        results->full_cpu_cycles,
        config->frame_count,
        config->payload_len,
        true
    );
    print_cycles(
        "CPU: frame data written before every frame",
        results->data_cpu_cycles,
        config->frame_count,
        config->payload_len,
        true
    );
    print_cycles(
        "Hardware: frame data written once",
        results->hardware_cycles,
        config->frame_count,
        config->payload_len,
        true
    );

    uart_write("Measurement finished\r\n");
}

int main(void)
{
    static uint8_t payload[ETHERNET_TX_MAX_BYTES];
    static ethernet_frame_t frame;
    uint32_t led_value = 0u;

    uart_init();
    timer_set_enabled(1u);
    (void)timer_get_value();

    uart_write("\r\nucore ethernet modular/bypass measurement\r\n");

    while (1) {
        measurement_config_t config;
        measurement_results_t results;
        uint32_t payload_seed = PAYLOAD_SEED;

        gpio_set_dout(led_value++);
        config = read_measurement_config();
        normalize_measurement_config(&config);
        fill_random_payload(payload, config.payload_len, &payload_seed);

        if (!ethernet_frame_build(
                &frame,
                config.mode,
                payload,
                config.payload_len)) {
            uart_write("ERROR: frame preparation failed\r\n");
            continue;
        }

        if (!run_measurements(&config, &frame, &results))
            continue;

        print_measurement_results(&config, &frame, &results);
    }
}
