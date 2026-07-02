#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ETHERNET_CTRL_ADDRESS             (ETHERNET_BASE_ADDRESS + 0x000)
#define ETHERNET_LOOPBACK_CTRL_ADDRESS    (ETHERNET_BASE_ADDRESS + 0x008)
#define ETHERNET_TX_LENGTH_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x00c)
#define ETHERNET_FRAME_COUNT_ADDRESS      (ETHERNET_BASE_ADDRESS + 0x010)
#define ETHERNET_IFG_CYCLES_ADDRESS       (ETHERNET_BASE_ADDRESS + 0x018)

#define ETHERNET_DST_MAC_L_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x020)
#define ETHERNET_DST_MAC_H_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x024)
#define ETHERNET_SRC_MAC_L_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x028)
#define ETHERNET_SRC_MAC_H_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x02c)

#define ETHERNET_ETHERTYPE_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x030)

#define ETHERNET_IPV4_SRC_ADDRESS         (ETHERNET_BASE_ADDRESS + 0x040)
#define ETHERNET_IPV4_DST_ADDRESS         (ETHERNET_BASE_ADDRESS + 0x044)
#define ETHERNET_IPV4_CFG0_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x048)
#define ETHERNET_IPV4_CFG1_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x04c)
#define ETHERNET_IPV4_CHECKSUM_ADDRESS    (ETHERNET_BASE_ADDRESS + 0x050)

#define ETHERNET_UDP_PORTS_ADDRESS        (ETHERNET_BASE_ADDRESS + 0x060)
#define ETHERNET_UDP_LENGTH_ADDRESS       (ETHERNET_BASE_ADDRESS + 0x064)
#define ETHERNET_UDP_CHECKSUM_ADDRESS     (ETHERNET_BASE_ADDRESS + 0x068)

#define ETHERNET_TX_DATA_BASE_ADDRESS     (ETHERNET_BASE_ADDRESS + 0x100)
#define ETHERNET_TX_DATA_WORDS            32u
#define ETHERNET_TX_MAX_BYTES             128u

#define ETHERNET_LOOPBACK_DIRECT_LVL_MASK     (1u << 0)
#define ETHERNET_LOOPBACK_ANALYZER_LVL_MASK   (1u << 1)
#define ETHERNET_LOOPBACK_ETH_IP_LVL_MASK     (1u << 2)

#define ETHERNET_IPV4_ADDR(a, b, c, d) \
    ((((uint32_t)(a) & 0xffu) << 24) | \
     (((uint32_t)(b) & 0xffu) << 16) | \
     (((uint32_t)(c) & 0xffu) << 8)  | \
     (((uint32_t)(d) & 0xffu) << 0))

void ethernet_trigger(void);

void ethernet_set_loopback(uint32_t loopback_mask);
void ethernet_set_loopback_levels(bool direct_lvl, bool analyzer_lvl, bool eth_ip_lvl);
void ethernet_disable_loopback(void);

void ethernet_set_tx_length(uint16_t length);
void ethernet_set_frame_count(uint32_t frame_count);
void ethernet_set_ifg_cycles(uint32_t cycles);

void ethernet_set_dst_mac(uint64_t mac);
void ethernet_set_src_mac(uint64_t mac);

void ethernet_set_ethertype(uint16_t ethertype);

void ethernet_set_ipv4_src(uint32_t ipv4_addr);
void ethernet_set_ipv4_dst(uint32_t ipv4_addr);
void ethernet_set_ipv4_cfg0(uint8_t ttl, uint8_t protocol, uint8_t dscp_ecn);
void ethernet_set_ipv4_cfg1(uint16_t identification, uint8_t flags, uint16_t fragment_offset);
void ethernet_set_ipv4_checksum(uint16_t checksum);

void ethernet_set_udp_ports(uint16_t src_port, uint16_t dst_port);
void ethernet_set_udp_length(uint16_t length);
void ethernet_set_udp_checksum(uint16_t checksum);

bool ethernet_set_tx_word(uint32_t index, uint32_t value);
void ethernet_clear_tx_data(void);
void ethernet_set_tx_words(const uint32_t *words, size_t word_count);
void ethernet_set_tx_payload(const uint8_t *payload, size_t payload_len);

#endif
