#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "csr_eth.h"

#define ETHERNET_TX_DATA_WORDS              32u
#define ETHERNET_TX_MAX_BYTES              128u
#define ETHERNET_BYPASS_MAX_PAYLOAD_BYTES   74u

#define ETHERNET_STATUS_BUSY_MASK \
    CSR_ETH__STATUS__BUSY_bm

#define ETHERNET_LOOPBACK_DIRECT_LVL_MASK \
    CSR_ETH__LOOPBACK_CTRL__LOOPBACK_DIRECT_LVL_bm

#define ETHERNET_LOOPBACK_ANALYZER_LVL_MASK \
    CSR_ETH__LOOPBACK_CTRL__LOOPBACK_ANALYZER_LVL_bm

#define ETHERNET_LOOPBACK_ETH_IP_LVL_MASK \
    CSR_ETH__LOOPBACK_CTRL__LOOPBACK_ETH_IP_LVL_bm

#define ETHERNET_IPV4_ADDR(a, b, c, d) \
    ((((uint32_t)(a) & 0xffu) << 24) | \
     (((uint32_t)(b) & 0xffu) << 16) | \
     (((uint32_t)(c) & 0xffu) << 8)  | \
     (((uint32_t)(d) & 0xffu) << 0))

typedef enum {
    ETHERNET_FRAME_MODE_MODULAR = 0,
    ETHERNET_FRAME_MODE_BYPASS = 1
} ethernet_frame_mode_t;

typedef struct {
    ethernet_frame_mode_t mode;
    uint16_t length;
    uint8_t data[ETHERNET_TX_MAX_BYTES];
} ethernet_frame_t;

void ethernet_trigger(void);

uint32_t ethernet_get_status(void);

bool ethernet_is_busy(void);

bool ethernet_wait_until_busy(uint32_t max_polls);

bool ethernet_wait_until_idle(uint32_t max_polls);

void ethernet_set_loopback(uint32_t loopback_mask);

void ethernet_set_loopback_levels(bool direct_lvl,
                                  bool analyzer_lvl,
                                  bool eth_ip_lvl);

void ethernet_disable_loopback(void);

void ethernet_set_tx_length(uint16_t length);

void ethernet_set_frame_count(uint32_t frame_count);

void ethernet_set_ifg_cycles(uint32_t cycles);

void ethernet_set_dst_mac(uint64_t mac);

void ethernet_set_src_mac(uint64_t mac);

void ethernet_set_ethertype(uint16_t ethertype);

void ethernet_set_ipv4_src(uint32_t ipv4_addr);

void ethernet_set_ipv4_dst(uint32_t ipv4_addr);

void ethernet_set_ipv4_cfg0(uint8_t ttl,
                            uint8_t protocol,
                            uint8_t dscp_ecn);

void ethernet_set_ipv4_cfg1(uint16_t identification,
                            uint8_t flags,
                            uint16_t fragment_offset);

void ethernet_set_ipv4_checksum(uint16_t checksum);

void ethernet_set_udp_ports(uint16_t src_port, uint16_t dst_port);

void ethernet_set_udp_length(uint16_t length);

void ethernet_set_udp_checksum(uint16_t checksum);

bool ethernet_set_tx_word(uint32_t index, uint32_t value);

void ethernet_clear_tx_data(void);

void ethernet_set_tx_words(const uint32_t *words, size_t word_count);

void ethernet_set_tx_payload(const uint8_t *payload, size_t payload_len);

bool ethernet_frame_build(ethernet_frame_t *frame,
                          ethernet_frame_mode_t mode,
                          const uint8_t *payload,
                          size_t payload_len);

bool ethernet_configure_frame_mode(ethernet_frame_mode_t mode,
                                   uint32_t ifg_cycles);

bool ethernet_load_frame(const ethernet_frame_t *frame);

bool ethernet_prepare_frame(const ethernet_frame_t *frame,
                            uint32_t ifg_cycles);

#endif
