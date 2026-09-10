#include <ethernet.h>
#include <memory_map.h>
#include <reg.h>

#define ETHERNET_REG_ADDRESS(member) \
    ((uint32_t)(ETHERNET_BASE_ADDRESS + offsetof(csr_eth_t, member)))

#define ETHERNET_FIELD_PREP(value, mask, position) \
    ((((uint32_t)(value)) << (position)) & (uint32_t)(mask))

#define ETHERNET_LOOPBACK_MASK \
    (ETHERNET_LOOPBACK_DIRECT_LVL_MASK | \
     ETHERNET_LOOPBACK_ANALYZER_LVL_MASK | \
     ETHERNET_LOOPBACK_ETH_IP_LVL_MASK)

#define ETHERNET_MAC_HIGH_MASK                0x0000ffffu

#define ETHERNET_DEFAULT_DST_MAC              UINT64_C(0xffffffffffff)
#define ETHERNET_DEFAULT_SRC_MAC              UINT64_C(0x123400000002)
#define ETHERNET_ETHERTYPE_IPV4               0x0800u
#define ETHERNET_IPV4_HEADER_BYTES            20u
#define ETHERNET_UDP_HEADER_BYTES              8u
#define ETHERNET_MIN_PAYLOAD_BYTES            46u
#define ETHERNET_IPV4_VERSION_IHL             0x45u
#define ETHERNET_IPV4_TTL                     64u
#define ETHERNET_IPV4_PROTOCOL_UDP            17u
#define ETHERNET_IPV4_FLAGS_DONT_FRAGMENT      2u
#define ETHERNET_IPV4_SRC_ADDR \
    ETHERNET_IPV4_ADDR(192, 168, 1, 2)
#define ETHERNET_IPV4_DST_ADDR \
    ETHERNET_IPV4_ADDR(192, 168, 1, 90)
#define ETHERNET_UDP_SRC_PORT              50000u
#define ETHERNET_UDP_DST_PORT              50001u

void ethernet_trigger(void)
{
    reg_write(ETHERNET_REG_ADDRESS(ctrl), CSR_ETH__CTRL__START_bm);
}

uint32_t ethernet_get_status(void)
{
    return reg_read(ETHERNET_REG_ADDRESS(status));
}

bool ethernet_is_busy(void)
{
    return (ethernet_get_status() & ETHERNET_STATUS_BUSY_MASK) != 0u;
}

bool ethernet_wait_until_busy(uint32_t max_polls)
{
    while (max_polls != 0u) {
        if (ethernet_is_busy())
            return true;

        --max_polls;
    }

    return false;
}

bool ethernet_wait_until_idle(uint32_t max_polls)
{
    while (max_polls != 0u) {
        if (!ethernet_is_busy())
            return true;

        --max_polls;
    }

    return false;
}

void ethernet_set_loopback(uint32_t loopback_mask)
{
    reg_write(
        ETHERNET_REG_ADDRESS(loopback_ctrl),
        loopback_mask & ETHERNET_LOOPBACK_MASK
    );
}

void ethernet_set_loopback_levels(bool direct_lvl,
                                  bool analyzer_lvl,
                                  bool eth_ip_lvl)
{
    uint32_t value = 0u;

    if (direct_lvl)
        value |= ETHERNET_LOOPBACK_DIRECT_LVL_MASK;

    if (analyzer_lvl)
        value |= ETHERNET_LOOPBACK_ANALYZER_LVL_MASK;

    if (eth_ip_lvl)
        value |= ETHERNET_LOOPBACK_ETH_IP_LVL_MASK;

    ethernet_set_loopback(value);
}

void ethernet_disable_loopback(void)
{
    ethernet_set_loopback(0u);
}

void ethernet_set_tx_length(uint16_t length)
{
    if (length > ETHERNET_TX_MAX_BYTES)
        length = ETHERNET_TX_MAX_BYTES;

    reg_write(
        ETHERNET_REG_ADDRESS(tx_length),
        ETHERNET_FIELD_PREP(
            length,
            CSR_ETH__TX_LENGTH__VALUE_bm,
            CSR_ETH__TX_LENGTH__VALUE_bp
        )
    );
}

void ethernet_set_frame_count(uint32_t frame_count)
{
    reg_write(
        ETHERNET_REG_ADDRESS(frame_count),
        ETHERNET_FIELD_PREP(
            frame_count,
            CSR_ETH__FRAME_COUNT__VALUE_bm,
            CSR_ETH__FRAME_COUNT__VALUE_bp
        )
    );
}

void ethernet_set_ifg_cycles(uint32_t cycles)
{
    reg_write(
        ETHERNET_REG_ADDRESS(ifg_cycles),
        ETHERNET_FIELD_PREP(
            cycles,
            CSR_ETH__IFG_CYCLES__VALUE_bm,
            CSR_ETH__IFG_CYCLES__VALUE_bp
        )
    );
}

void ethernet_set_dst_mac(uint64_t mac)
{
    reg_write(
        ETHERNET_REG_ADDRESS(dst_mac_l),
        ETHERNET_FIELD_PREP(
            (uint32_t)mac,
            CSR_ETH__DST_MAC_L__VALUE_bm,
            CSR_ETH__DST_MAC_L__VALUE_bp
        )
    );
    reg_write(
        ETHERNET_REG_ADDRESS(dst_mac_h),
        ETHERNET_FIELD_PREP(
            (uint32_t)(mac >> 32) & ETHERNET_MAC_HIGH_MASK,
            CSR_ETH__DST_MAC_H__VALUE_bm,
            CSR_ETH__DST_MAC_H__VALUE_bp
        )
    );
}

void ethernet_set_src_mac(uint64_t mac)
{
    reg_write(
        ETHERNET_REG_ADDRESS(src_mac_l),
        ETHERNET_FIELD_PREP(
            (uint32_t)mac,
            CSR_ETH__SRC_MAC_L__VALUE_bm,
            CSR_ETH__SRC_MAC_L__VALUE_bp
        )
    );
    reg_write(
        ETHERNET_REG_ADDRESS(src_mac_h),
        ETHERNET_FIELD_PREP(
            (uint32_t)(mac >> 32) & ETHERNET_MAC_HIGH_MASK,
            CSR_ETH__SRC_MAC_H__VALUE_bm,
            CSR_ETH__SRC_MAC_H__VALUE_bp
        )
    );
}

void ethernet_set_ethertype(uint16_t ethertype)
{
    reg_write(
        ETHERNET_REG_ADDRESS(ethertype),
        ETHERNET_FIELD_PREP(
            ethertype,
            CSR_ETH__ETHERTYPE__VALUE_bm,
            CSR_ETH__ETHERTYPE__VALUE_bp
        )
    );
}

void ethernet_set_ipv4_src(uint32_t ipv4_addr)
{
    reg_write(
        ETHERNET_REG_ADDRESS(ipv4_src),
        ETHERNET_FIELD_PREP(
            ipv4_addr,
            CSR_ETH__IPV4_SRC__VALUE_bm,
            CSR_ETH__IPV4_SRC__VALUE_bp
        )
    );
}

void ethernet_set_ipv4_dst(uint32_t ipv4_addr)
{
    reg_write(
        ETHERNET_REG_ADDRESS(ipv4_dst),
        ETHERNET_FIELD_PREP(
            ipv4_addr,
            CSR_ETH__IPV4_DST__VALUE_bm,
            CSR_ETH__IPV4_DST__VALUE_bp
        )
    );
}

void ethernet_set_ipv4_cfg0(uint8_t ttl,
                            uint8_t protocol,
                            uint8_t dscp_ecn)
{
    uint32_t value = 0u;

    value |= ETHERNET_FIELD_PREP(
        ttl,
        CSR_ETH__IPV4_CFG0__TTL_bm,
        CSR_ETH__IPV4_CFG0__TTL_bp
    );
    value |= ETHERNET_FIELD_PREP(
        protocol,
        CSR_ETH__IPV4_CFG0__PROTOCOL_bm,
        CSR_ETH__IPV4_CFG0__PROTOCOL_bp
    );
    value |= ETHERNET_FIELD_PREP(
        dscp_ecn,
        CSR_ETH__IPV4_CFG0__DSCP_ECN_bm,
        CSR_ETH__IPV4_CFG0__DSCP_ECN_bp
    );

    reg_write(ETHERNET_REG_ADDRESS(ipv4_cfg0), value);
}

void ethernet_set_ipv4_cfg1(uint16_t identification,
                            uint8_t flags,
                            uint16_t fragment_offset)
{
    uint32_t value = 0u;

    value |= ETHERNET_FIELD_PREP(
        identification,
        CSR_ETH__IPV4_CFG1__IDENTIFICATION_bm,
        CSR_ETH__IPV4_CFG1__IDENTIFICATION_bp
    );
    value |= ETHERNET_FIELD_PREP(
        flags,
        CSR_ETH__IPV4_CFG1__FLAGS_bm,
        CSR_ETH__IPV4_CFG1__FLAGS_bp
    );
    value |= ETHERNET_FIELD_PREP(
        fragment_offset,
        CSR_ETH__IPV4_CFG1__FRAGMENT_OFFSET_bm,
        CSR_ETH__IPV4_CFG1__FRAGMENT_OFFSET_bp
    );

    reg_write(ETHERNET_REG_ADDRESS(ipv4_cfg1), value);
}

void ethernet_set_ipv4_checksum(uint16_t checksum)
{
    reg_write(
        ETHERNET_REG_ADDRESS(ipv4_checksum),
        ETHERNET_FIELD_PREP(
            checksum,
            CSR_ETH__IPV4_CHECKSUM__VALUE_bm,
            CSR_ETH__IPV4_CHECKSUM__VALUE_bp
        )
    );
}

void ethernet_set_udp_ports(uint16_t src_port, uint16_t dst_port)
{
    uint32_t value = 0u;

    value |= ETHERNET_FIELD_PREP(
        src_port,
        CSR_ETH__UDP_PORTS__SRC_PORT_bm,
        CSR_ETH__UDP_PORTS__SRC_PORT_bp
    );
    value |= ETHERNET_FIELD_PREP(
        dst_port,
        CSR_ETH__UDP_PORTS__DST_PORT_bm,
        CSR_ETH__UDP_PORTS__DST_PORT_bp
    );

    reg_write(ETHERNET_REG_ADDRESS(udp_ports), value);
}

void ethernet_set_udp_length(uint16_t length)
{
    reg_write(
        ETHERNET_REG_ADDRESS(udp_length),
        ETHERNET_FIELD_PREP(
            length,
            CSR_ETH__UDP_LENGTH__VALUE_bm,
            CSR_ETH__UDP_LENGTH__VALUE_bp
        )
    );
}

void ethernet_set_udp_checksum(uint16_t checksum)
{
    reg_write(
        ETHERNET_REG_ADDRESS(udp_checksum),
        ETHERNET_FIELD_PREP(
            checksum,
            CSR_ETH__UDP_CHECKSUM__VALUE_bm,
            CSR_ETH__UDP_CHECKSUM__VALUE_bp
        )
    );
}

bool ethernet_set_tx_word(uint32_t index, uint32_t value)
{
    if (index >= ETHERNET_TX_DATA_WORDS)
        return false;

    reg_write(
        ETHERNET_REG_ADDRESS(tx_data_0) + index * sizeof(uint32_t),
        value
    );
    return true;
}

void ethernet_clear_tx_data(void)
{
    for (uint32_t i = 0; i < ETHERNET_TX_DATA_WORDS; ++i)
        ethernet_set_tx_word(i, 0u);
}

void ethernet_set_tx_words(const uint32_t *words, size_t word_count)
{
    if (words == NULL)
        return;

    if (word_count > ETHERNET_TX_DATA_WORDS)
        word_count = ETHERNET_TX_DATA_WORDS;

    for (size_t i = 0; i < word_count; ++i)
        ethernet_set_tx_word((uint32_t)i, words[i]);

    for (size_t i = word_count; i < ETHERNET_TX_DATA_WORDS; ++i)
        ethernet_set_tx_word((uint32_t)i, 0u);
}

void ethernet_set_tx_payload(const uint8_t *payload, size_t payload_len)
{
    if (payload == NULL)
        return;

    if (payload_len > ETHERNET_TX_MAX_BYTES)
        payload_len = ETHERNET_TX_MAX_BYTES;

    for (uint32_t word_idx = 0; word_idx < ETHERNET_TX_DATA_WORDS; ++word_idx) {
        uint32_t value = 0u;

        for (uint32_t byte_idx = 0; byte_idx < 4u; ++byte_idx) {
            size_t payload_idx = ((size_t)word_idx * 4u) + byte_idx;

            if (payload_idx < payload_len) {
                value |= ((uint32_t)payload[payload_idx])
                         << (8u * (3u - byte_idx));
            }
        }

        ethernet_set_tx_word(word_idx, value);
    }

    ethernet_set_tx_length((uint16_t)payload_len);
}

static void frame_append_u8(uint8_t *data, uint32_t *index, uint8_t value)
{
    data[*index] = value;
    ++(*index);
}

static void frame_append_u16_be(uint8_t *data, uint32_t *index, uint16_t value)
{
    frame_append_u8(data, index, (uint8_t)(value >> 8));
    frame_append_u8(data, index, (uint8_t)value);
}

static void frame_append_u32_be(uint8_t *data, uint32_t *index, uint32_t value)
{
    frame_append_u8(data, index, (uint8_t)(value >> 24));
    frame_append_u8(data, index, (uint8_t)(value >> 16));
    frame_append_u8(data, index, (uint8_t)(value >> 8));
    frame_append_u8(data, index, (uint8_t)value);
}

static void frame_append_u32_le(uint8_t *data, uint32_t *index, uint32_t value)
{
    frame_append_u8(data, index, (uint8_t)value);
    frame_append_u8(data, index, (uint8_t)(value >> 8));
    frame_append_u8(data, index, (uint8_t)(value >> 16));
    frame_append_u8(data, index, (uint8_t)(value >> 24));
}

static void frame_append_mac(uint8_t *data, uint32_t *index, uint64_t mac)
{
    for (uint32_t byte_index = 0; byte_index < 6u; ++byte_index) {
        uint32_t shift = 8u * (5u - byte_index);

        frame_append_u8(data, index, (uint8_t)(mac >> shift));
    }
}

static uint16_t calculate_ipv4_checksum(const uint8_t *header)
{
    uint32_t sum = 0u;

    for (uint32_t i = 0; i < ETHERNET_IPV4_HEADER_BYTES; i += 2u)
        sum += ((uint32_t)header[i] << 8) | header[i + 1u];

    while ((sum >> 16) != 0u)
        sum = (sum & 0xffffu) + (sum >> 16);

    return (uint16_t)(~sum);
}

static uint32_t calculate_ethernet_fcs(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xffffffffu;

    for (uint32_t i = 0; i < len; ++i) {
        crc ^= data[i];

        for (uint32_t bit = 0; bit < 8u; ++bit) {
            if ((crc & 1u) != 0u)
                crc = (crc >> 1) ^ 0xedb88320u;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}

static uint16_t build_bypass_frame(uint8_t *data,
                                   const uint8_t *payload,
                                   uint32_t payload_len)
{
    uint32_t index = 0u;
    uint32_t ipv4_offset;
    uint16_t ipv4_total_len =
        (uint16_t)(ETHERNET_IPV4_HEADER_BYTES +
                   ETHERNET_UDP_HEADER_BYTES +
                   payload_len);
    uint16_t udp_len = (uint16_t)(ETHERNET_UDP_HEADER_BYTES + payload_len);
    uint32_t ethernet_payload_len;
    uint16_t ipv4_checksum;
    uint32_t fcs;

    for (uint32_t i = 0; i < 7u; ++i)
        frame_append_u8(data, &index, 0x55u);

    frame_append_u8(data, &index, 0xd5u);
    frame_append_mac(data, &index, ETHERNET_DEFAULT_DST_MAC);
    frame_append_mac(data, &index, ETHERNET_DEFAULT_SRC_MAC);
    frame_append_u16_be(data, &index, ETHERNET_ETHERTYPE_IPV4);

    ipv4_offset = index;

    frame_append_u8(data, &index, ETHERNET_IPV4_VERSION_IHL);
    frame_append_u8(data, &index, 0u);
    frame_append_u16_be(data, &index, ipv4_total_len);
    frame_append_u16_be(data, &index, 0u);
    frame_append_u16_be(
        data,
        &index,
        (uint16_t)(ETHERNET_IPV4_FLAGS_DONT_FRAGMENT << 13)
    );
    frame_append_u8(data, &index, ETHERNET_IPV4_TTL);
    frame_append_u8(data, &index, ETHERNET_IPV4_PROTOCOL_UDP);
    frame_append_u16_be(data, &index, 0u);
    frame_append_u32_be(data, &index, ETHERNET_IPV4_SRC_ADDR);
    frame_append_u32_be(data, &index, ETHERNET_IPV4_DST_ADDR);

    ipv4_checksum = calculate_ipv4_checksum(&data[ipv4_offset]);
    data[ipv4_offset + 10u] = (uint8_t)(ipv4_checksum >> 8);
    data[ipv4_offset + 11u] = (uint8_t)ipv4_checksum;

    frame_append_u16_be(data, &index, ETHERNET_UDP_SRC_PORT);
    frame_append_u16_be(data, &index, ETHERNET_UDP_DST_PORT);
    frame_append_u16_be(data, &index, udp_len);
    frame_append_u16_be(data, &index, 0u);

    for (uint32_t i = 0; i < payload_len; ++i)
        frame_append_u8(data, &index, payload[i]);

    ethernet_payload_len = ipv4_total_len;

    while (ethernet_payload_len < ETHERNET_MIN_PAYLOAD_BYTES) {
        frame_append_u8(data, &index, 0u);
        ++ethernet_payload_len;
    }

    fcs = calculate_ethernet_fcs(&data[8], index - 8u);
    frame_append_u32_le(data, &index, fcs);

    return (uint16_t)index;
}

bool ethernet_frame_build(ethernet_frame_t *frame,
                          ethernet_frame_mode_t mode,
                          const uint8_t *payload,
                          size_t payload_len)
{
    if (frame == NULL || (payload == NULL && payload_len != 0u))
        return false;

    frame->mode = mode;
    frame->length = 0u;

    for (size_t i = 0; i < ETHERNET_TX_MAX_BYTES; ++i)
        frame->data[i] = 0u;

    switch (mode) {
    case ETHERNET_FRAME_MODE_MODULAR:
        if (payload_len > ETHERNET_TX_MAX_BYTES)
            return false;

        for (size_t i = 0; i < payload_len; ++i)
            frame->data[i] = payload[i];

        frame->length = (uint16_t)payload_len;
        return true;

    case ETHERNET_FRAME_MODE_BYPASS:
        if (payload_len > ETHERNET_BYPASS_MAX_PAYLOAD_BYTES)
            return false;

        frame->length = build_bypass_frame(
            frame->data,
            payload,
            (uint32_t)payload_len
        );
        return true;

    default:
        return false;
    }
}

static void configure_modular_mode(uint32_t ifg_cycles)
{
    ethernet_disable_loopback();

    ethernet_set_dst_mac(ETHERNET_DEFAULT_DST_MAC);
    ethernet_set_src_mac(ETHERNET_DEFAULT_SRC_MAC);
    ethernet_set_ethertype(ETHERNET_ETHERTYPE_IPV4);

    ethernet_set_ipv4_src(ETHERNET_IPV4_SRC_ADDR);
    ethernet_set_ipv4_dst(ETHERNET_IPV4_DST_ADDR);
    ethernet_set_ipv4_cfg0(
        ETHERNET_IPV4_TTL,
        ETHERNET_IPV4_PROTOCOL_UDP,
        0u
    );
    ethernet_set_ipv4_cfg1(
        0u,
        ETHERNET_IPV4_FLAGS_DONT_FRAGMENT,
        0u
    );
    ethernet_set_ipv4_checksum(0u);

    ethernet_set_udp_ports(
        ETHERNET_UDP_SRC_PORT,
        ETHERNET_UDP_DST_PORT
    );
    ethernet_set_udp_length(0u);
    ethernet_set_udp_checksum(0u);

    ethernet_set_ifg_cycles(ifg_cycles);
}

static void configure_bypass_mode(uint32_t ifg_cycles)
{
    ethernet_set_loopback_levels(false, false, true);
    ethernet_set_ifg_cycles(ifg_cycles);
}

bool ethernet_configure_frame_mode(ethernet_frame_mode_t mode,
                                   uint32_t ifg_cycles)
{
    switch (mode) {
    case ETHERNET_FRAME_MODE_MODULAR:
        configure_modular_mode(ifg_cycles);
        return true;

    case ETHERNET_FRAME_MODE_BYPASS:
        configure_bypass_mode(ifg_cycles);
        return true;

    default:
        return false;
    }
}

static bool write_modular_data(const uint8_t *data, uint32_t data_len)
{
    uint32_t word_count = (data_len + 3u) / 4u;

    for (uint32_t word_index = 0; word_index < word_count; ++word_index) {
        uint32_t value = 0u;

        for (uint32_t byte_index = 0; byte_index < 4u; ++byte_index) {
            uint32_t data_index = word_index * 4u + byte_index;

            if (data_index < data_len) {
                value |= ((uint32_t)data[data_index])
                         << (8u * (3u - byte_index));
            }
        }

        if (!ethernet_set_tx_word(word_index, value))
            return false;
    }

    ethernet_set_tx_length((uint16_t)data_len);
    return true;
}

static bool write_bypass_data(const uint8_t *data, uint32_t data_len)
{
    uint32_t word_count = (data_len + 3u) / 4u;

    for (uint32_t word_index = 0; word_index < word_count; ++word_index) {
        uint32_t value = 0u;

        for (uint32_t byte_index = 0; byte_index < 4u; ++byte_index) {
            uint32_t data_index = word_index * 4u + byte_index;

            if (data_index < data_len)
                value |= ((uint32_t)data[data_index]) << (8u * byte_index);
        }

        if (!ethernet_set_tx_word(word_index, value))
            return false;
    }

    ethernet_set_tx_length((uint16_t)data_len);
    return true;
}

bool ethernet_load_frame(const ethernet_frame_t *frame)
{
    if (frame == NULL || frame->length > ETHERNET_TX_MAX_BYTES)
        return false;

    switch (frame->mode) {
    case ETHERNET_FRAME_MODE_MODULAR:
        return write_modular_data(frame->data, frame->length);

    case ETHERNET_FRAME_MODE_BYPASS:
        return write_bypass_data(frame->data, frame->length);

    default:
        return false;
    }
}

bool ethernet_prepare_frame(const ethernet_frame_t *frame,
                            uint32_t ifg_cycles)
{
    if (frame == NULL)
        return false;

    if (!ethernet_configure_frame_mode(frame->mode, ifg_cycles))
        return false;

    if (!ethernet_load_frame(frame))
        return false;

    ethernet_set_frame_count(1u);
    return true;
}
