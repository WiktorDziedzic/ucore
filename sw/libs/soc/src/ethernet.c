#include <ethernet.h>
#include <memory_map.h>
#include <reg.h>

void ethernet_trigger(void)
{
    reg_write(ETHERNET_CTRL_ADDRESS, 0x00000001u);
}

void ethernet_set_loopback(uint32_t loopback_mask)
{
    reg_write(ETHERNET_LOOPBACK_CTRL_ADDRESS, loopback_mask & 0x00000007u);
}

void ethernet_set_loopback_levels(bool direct_lvl, bool analyzer_lvl, bool eth_ip_lvl)
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

    reg_write(ETHERNET_TX_LENGTH_ADDRESS, (uint32_t)length);
}

void ethernet_set_frame_count(uint32_t frame_count)
{
    reg_write(ETHERNET_FRAME_COUNT_ADDRESS, frame_count);
}

void ethernet_set_ifg_cycles(uint32_t cycles)
{
    reg_write(ETHERNET_IFG_CYCLES_ADDRESS, cycles);
}

void ethernet_set_dst_mac(uint64_t mac)
{
    reg_write(ETHERNET_DST_MAC_L_ADDRESS, (uint32_t)(mac & 0xffffffffu));
    reg_write(ETHERNET_DST_MAC_H_ADDRESS, (uint32_t)((mac >> 32) & 0x0000ffffu));
}

void ethernet_set_src_mac(uint64_t mac)
{
    reg_write(ETHERNET_SRC_MAC_L_ADDRESS, (uint32_t)(mac & 0xffffffffu));
    reg_write(ETHERNET_SRC_MAC_H_ADDRESS, (uint32_t)((mac >> 32) & 0x0000ffffu));
}

void ethernet_set_ethertype(uint16_t ethertype)
{
    reg_write(ETHERNET_ETHERTYPE_ADDRESS, (uint32_t)ethertype);
}

void ethernet_set_ipv4_src(uint32_t ipv4_addr)
{
    reg_write(ETHERNET_IPV4_SRC_ADDRESS, ipv4_addr);
}

void ethernet_set_ipv4_dst(uint32_t ipv4_addr)
{
    reg_write(ETHERNET_IPV4_DST_ADDRESS, ipv4_addr);
}

void ethernet_set_ipv4_cfg0(uint8_t ttl, uint8_t protocol, uint8_t dscp_ecn)
{
    uint32_t value = 0u;

    value |= ((uint32_t)ttl)      << 0;
    value |= ((uint32_t)protocol) << 8;
    value |= ((uint32_t)dscp_ecn) << 16;

    reg_write(ETHERNET_IPV4_CFG0_ADDRESS, value);
}

void ethernet_set_ipv4_cfg1(uint16_t identification, uint8_t flags, uint16_t fragment_offset)
{
    uint32_t value = 0u;

    value |= ((uint32_t)identification)               << 0;
    value |= ((uint32_t)(flags & 0x7u))               << 16;
    value |= ((uint32_t)(fragment_offset & 0x1fffu))  << 19;

    reg_write(ETHERNET_IPV4_CFG1_ADDRESS, value);
}

void ethernet_set_ipv4_checksum(uint16_t checksum)
{
    reg_write(ETHERNET_IPV4_CHECKSUM_ADDRESS, (uint32_t)checksum);
}

void ethernet_set_udp_ports(uint16_t src_port, uint16_t dst_port)
{
    uint32_t value = 0u;

    value |= ((uint32_t)src_port) << 0;
    value |= ((uint32_t)dst_port) << 16;

    reg_write(ETHERNET_UDP_PORTS_ADDRESS, value);
}

void ethernet_set_udp_length(uint16_t length)
{
    reg_write(ETHERNET_UDP_LENGTH_ADDRESS, (uint32_t)length);
}

void ethernet_set_udp_checksum(uint16_t checksum)
{
    reg_write(ETHERNET_UDP_CHECKSUM_ADDRESS, (uint32_t)checksum);
}

bool ethernet_set_tx_word(uint32_t index, uint32_t value)
{
    if (index >= ETHERNET_TX_DATA_WORDS)
        return false;

    reg_write(ETHERNET_TX_DATA_BASE_ADDRESS + (index * 4u), value);
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

            if (payload_idx < payload_len)
                value |= ((uint32_t)payload[payload_idx]) << (8u * (3u - byte_idx));
        }

        ethernet_set_tx_word(word_idx, value);
    }

    ethernet_set_tx_length((uint16_t)payload_len);
}
