/*
 * bt_main.h
 *
 */
#ifndef __BT_MAIN_
#define __BT_MAIN_

#if defined __cplusplus
extern "C" {
#endif

// エラー返却用定数(-1:0xffff固定)
#define FAILED_RFCOMM_ID_NOT_SET	0xffff

#define RFCOMM_SERVER_CHANNEL 1

void bt_init(void);
int bt_can_send_packet(void);
void bt_send_packet(uint16_t bnep_cid);
void bt_recv_packet(uint16_t bnep_cid, const uint8_t * packet, uint16_t size);
void bt_channel_closed(uint16_t bnep_cid);
void bt_terminate(void);

void btstack_network_init(void (*send_packet_callback)(uint16_t bnep_cid, const uint8_t * packet, uint16_t size));
void btstack_network_up(uint16_t bnep_cid, bd_addr_t local_addr, bd_addr_t remote_addr);
const char * btstack_network_get_name(uint16_t bnep_cid);
int btstack_network_down(uint16_t bnep_cid);
void btstack_network_process_packet(uint16_t bnep_cid, const uint8_t * packet, uint16_t size);
void btstack_network_packet_sent(uint16_t bnep_cid);

#if defined __cplusplus
}
#endif
#endif
