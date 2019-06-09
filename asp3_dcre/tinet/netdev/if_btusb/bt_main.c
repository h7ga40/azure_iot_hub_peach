/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */

#include "btstack-config.h"

#include <stdint.h>

#include <btstack/hci_cmds.h>
#include <btstack/run_loop.h>
#include <btstack/sdp_util.h>

#include <kernel.h>
#include "kernel_cfg.h"
#include "target_syssvc.h"

#include "debug.h"
#include "hci.h"
#include "btstack_memory.h"
#include "hci_dump.h"
#include "l2cap.h"
#include "sdp_parser.h"
#include "sdp_client.h"
#include "sdp_query_util.h"
#include "sdp.h"
#include "pan.h"
#include "bt_main.h"
#include "usb_hbth.h"

//#define PACKET_DUMP

// エラー返却用定数(-1:0xffff固定)
#define FAILED_RFCOMM_ID_NOT_SET	0xffff

#define RFCOMM_SERVER_CHANNEL 1
static uint16_t  rfcomm_channel_id = FAILED_RFCOMM_ID_NOT_SET;

static int record_id = -1;
static uint16_t bnep_l2cap_psm = 0;
static uint32_t bnep_remote_uuid = 0;
static uint16_t bnep_version = 0;
static uint16_t bnep_cid = 0;
static bool_t bnep_do_connect = false;

static uint16_t sdp_bnep_l2cap_psm = 0;
static uint16_t sdp_bnep_version = 0;
static uint32_t sdp_bnep_remote_uuid = 0;

uint8_t   attribute_value[1000];
const unsigned int attribute_value_buffer_size = sizeof(attribute_value);

extern bd_addr_t remote_addr;

uint8_t   spp_service_buffer[120];
uint8_t   bnep_service_buffer[256];

char bt_localname[] = TARGET_NAME"0000";

volatile int bt_bnep_mode = 0;

enum bt_state_t {
	BT_STATE_DORMANT,
	BT_STATE_POWER_ON,
	BT_STATE_BNEP_QUERY,
	BT_STATE_BNEP_WAIT,
	BT_STATE_BNEP_QUERY_WAIT,
	BT_STATE_CONNECTING,
	BT_STATE_CONNECTED,
};

enum bt_state_t bt_state = BT_STATE_DORMANT;
int bt_timer;

// outgoing network packet
const uint8_t * network_buffer;
uint16_t network_buffer_len;
uint16_t network_buffer_cid;

/*************** PANU client routines *********************/

char * get_string_from_data_element(uint8_t * element) {
	de_size_t de_size = de_get_size_type(element);
	int pos = de_get_header_size(element);
	int len = 0;
	switch (de_size) {
	case DE_SIZE_VAR_8:
		len = element[1];
		break;
	case DE_SIZE_VAR_16:
		len = big_endian_read_16(element, 1);
		break;
	default:
		break;
	}
	char * str = (char*)malloc(len + 1);
	memcpy(str, &element[pos], len);
	str[len] = '\0';
	return str;
}

/* @section SDP parser callback
*
* @text The SDP parsers retrieves the BNEP PAN UUID as explained in
* Section [on SDP BNEP Query example](#sec:sdpbnepqueryExample}.
*/
static void handle_sdp_client_record_complete(void)
{
	log_info("SDP BNEP Record complete");

	// accept first entry or if we foudn a NAP and only have a PANU yet
	if ((bnep_remote_uuid == 0) || (sdp_bnep_remote_uuid == BLUETOOTH_SERVICE_CLASS_NAP && bnep_remote_uuid == BLUETOOTH_SERVICE_CLASS_PANU)) {
		bnep_l2cap_psm = sdp_bnep_l2cap_psm;
		bnep_remote_uuid = sdp_bnep_remote_uuid;
		bnep_version = sdp_bnep_version;
	}
}

/* SDP parser callback */
static void handle_sdp_client_query_result(sdp_query_event_t *event)
{
	sdp_query_attribute_value_event_t *value_event;
	sdp_query_complete_event_t        *complete_event;
	des_iterator_t des_list_it;
	des_iterator_t prot_it;
	char *str;

	switch (event->type) {
	case SDP_EVENT_QUERY_ATTRIBUTE_VALUE:
		value_event = (sdp_query_attribute_value_event_t*)event;

		// Handle new SDP record
		if (value_event->record_id != record_id) {
			handle_sdp_client_record_complete();
			// next record started
			record_id = value_event->record_id;
			log_info("SDP Record: Nr: %d", record_id);
		}

		if (value_event->attribute_length <= attribute_value_buffer_size) {
			attribute_value[value_event->data_offset] = value_event->data;

			if ((uint16_t)(value_event->data_offset + 1) == value_event->attribute_length) {
				//printf("SDP Attribute Data:");
				//printf_hexdump(attribute_value, value_event->attribute_length);

				switch (value_event->attribute_id) {
				case BLUETOOTH_ATTRIBUTE_SERVICE_CLASS_ID_LIST:
					if (de_get_element_type(attribute_value) != DE_DES) break;
					for (des_iterator_init(&des_list_it, attribute_value); des_iterator_has_more(&des_list_it); des_iterator_next(&des_list_it)) {
						uint8_t * element = des_iterator_get_element(&des_list_it);
						if (de_get_element_type(element) != DE_UUID) continue;
						uint32_t uuid = de_get_uuid32(element);
						switch (uuid) {
						case BLUETOOTH_SERVICE_CLASS_PANU:
						case BLUETOOTH_SERVICE_CLASS_NAP:
						case BLUETOOTH_SERVICE_CLASS_GN:
							log_info("SDP Attribute 0x%04x: BNEP PAN protocol UUID: %04x", value_event->attribute_id, uuid);
							sdp_bnep_remote_uuid = uuid;
							break;
						default:
							log_info("SDP Attribute 0x%04x: UUID: %04x", value_event->attribute_id, uuid);
							break;
						}
					}
					break;
					// 0x0100 "Service Name"
				case 0x0100:
					// 0x0101 "Service Description"
				case 0x0101:
					str = get_string_from_data_element(attribute_value);
					log_info("SDP Attribute: 0x%04x: %s", value_event->attribute_id, str);
					free(str);
					break;
				case BLUETOOTH_ATTRIBUTE_PROTOCOL_DESCRIPTOR_LIST: {
					log_info("SDP Attribute: 0x%04x", value_event->attribute_id);

					for (des_iterator_init(&des_list_it, attribute_value); des_iterator_has_more(&des_list_it); des_iterator_next(&des_list_it)) {
						uint8_t       *des_element;
						uint8_t       *element;
						uint32_t       uuid;

						if (des_iterator_get_type(&des_list_it) != DE_DES) continue;

						des_element = des_iterator_get_element(&des_list_it);
						des_iterator_init(&prot_it, des_element);
						element = des_iterator_get_element(&prot_it);

						if (de_get_element_type(element) != DE_UUID) continue;

						uuid = de_get_uuid32(element);
						switch (uuid) {
						case BLUETOOTH_PROTOCOL_L2CAP:
							if (!des_iterator_has_more(&prot_it)) continue;
							des_iterator_next(&prot_it);
							de_element_get_uint16(des_iterator_get_element(&prot_it), &sdp_bnep_l2cap_psm);
							break;
						case BLUETOOTH_PROTOCOL_BNEP:
							if (!des_iterator_has_more(&prot_it)) continue;
							des_iterator_next(&prot_it);
							de_element_get_uint16(des_iterator_get_element(&prot_it), &sdp_bnep_version);
							break;
						default:
							log_info("uuid 0x%04x", uuid);
							break;
						}
					}
					log_info("Summary: uuid 0x%04x, l2cap_psm 0x%04x, bnep_version 0x%04x", sdp_bnep_remote_uuid, sdp_bnep_l2cap_psm, sdp_bnep_version);
					break;
				}
				default:
					break;
				}
			}
		}
		else {
			log_error("SDP attribute value buffer size exceeded: available %d, required %d", attribute_value_buffer_size, value_event->attribute_length);
		}
		break;

	case SDP_EVENT_QUERY_COMPLETE:
		handle_sdp_client_record_complete();
		complete_event = (sdp_query_complete_event_t*)event;
		log_error("General query done with status %d, bnep psm %04x.", complete_event->status, bnep_l2cap_psm);
		if (bnep_l2cap_psm) {
			bnep_do_connect = complete_event->status == 0;
		}
		else {
			bnep_do_connect = false;
			log_error("No BNEP service found");
		}
		if (!bnep_do_connect) {
			// 20秒後 BNEP Query 開始
			bt_state = BT_STATE_BNEP_QUERY_WAIT;
			bt_timer = 20 * 1000;
		}
		break;
	}
}

static void packet_handler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
	uint8_t   event;
	bd_addr_t event_addr;
	uint8_t   rfcomm_channel_nr;
	uint16_t  uuid_source;
	uint16_t  uuid_dest;
	uint16_t  mtu;

	switch (packet_type) {
	case HCI_EVENT_PACKET:
		event = packet[0];
		switch (event) {
		case BTSTACK_EVENT_STATE:
			/* BT Stack activated, get started */
			if (packet[2] == HCI_STATE_WORKING) {
				hci_send_cmd(&hci_write_local_name, bt_localname);
				if (bt_bnep_mode == 0) {
					// 即時 BNEP Query 開始
					bt_state = BT_STATE_BNEP_QUERY_WAIT;
					bt_timer = 0;
				}
				else {
					bt_state = BT_STATE_BNEP_WAIT;
					bt_timer = -1;
				}
			}
			break;

		case BTSTACK_EVENT_NR_CONNECTIONS_CHANGED:
			if ((packet[2] == 0) && bnep_do_connect) {
				bnep_do_connect = false;
				/* Create BNEP connection */
				bnep_connect(NULL, &remote_addr, bnep_l2cap_psm, bnep_remote_uuid);
			}
			break;

		case HCI_EVENT_COMMAND_COMPLETE:
			if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_read_bd_addr)) {
				reverse_bd_addr(&packet[6], event_addr);
				log_info("BD-ADDR: %s", bd_addr_to_str(event_addr));

				snprintf(bt_localname, sizeof(bt_localname), TARGET_NAME"%04x", ((event_addr[4] << 8) | event_addr[5]));
				gap_set_local_name(bt_localname);
				break;
			}
			if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_write_local_name)) {
				gap_discoverable_control(1);
				break;
			}
			break;

		case HCI_EVENT_LINK_KEY_REQUEST:
			// deny link key request
			log_info("Link key request");
			reverse_bd_addr(&packet[2], event_addr);
			hci_send_cmd(&hci_link_key_request_negative_reply, &event_addr);
			break;

		case HCI_EVENT_PIN_CODE_REQUEST:
			// inform about pin code request
			log_info("Pin code request - using '0000'");
			reverse_bd_addr(&packet[2], event_addr);
			hci_send_cmd(&hci_pin_code_request_reply, &event_addr, 4, "0000");
			break;

		case HCI_EVENT_USER_CONFIRMATION_REQUEST:
			// inform about user confirmation request
			log_info("HCI User Confirmation Request with numeric value '%06u'", little_endian_read_32(packet, 8));
			log_info("HCI User Confirmation Auto accept");
			break;

		case RFCOMM_EVENT_INCOMING_CONNECTION:
			// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
			reverse_bd_addr(&packet[2], event_addr);
			rfcomm_channel_nr = packet[8];
			rfcomm_channel_id = little_endian_read_16(packet, 9);
			log_info("RFCOMM channel %u requested for %s", rfcomm_channel_nr, bd_addr_to_str(event_addr));
			rfcomm_accept_connection_internal(rfcomm_channel_id);
			break;

		case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
			// data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
			if (packet[2]) {
				log_info("RFCOMM channel open failed, status %u", packet[2]);
			}
			else {
				rfcomm_channel_id = little_endian_read_16(packet, 12);
				mtu = little_endian_read_16(packet, 14);
				log_info("RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u", rfcomm_channel_id, mtu);
			}
			break;

		case RFCOMM_EVENT_CHANNEL_CLOSED:
			log_info("RFCOMM channel closed");
			rfcomm_channel_id = 0;
			break;

		case BNEP_EVENT_OPEN_CHANNEL_COMPLETE:
			if (packet[2]) {
				log_info("BNEP channel open failed, status %02x", packet[2]);

				// 30秒後 BNEP Query 開始
				bt_state = BT_STATE_BNEP_QUERY_WAIT;
				bt_timer = 30 * 1000;
			}
			else {
				// data: event(8), len(8), status (8), bnep source uuid (16), bnep destination uuid (16), remote_address (48)
				bnep_cid = little_endian_read_16(packet, 3);
				uuid_source = little_endian_read_16(packet, 5);
				uuid_dest = little_endian_read_16(packet, 7);
				mtu = little_endian_read_16(packet, 9);
				//reverse_bd_addr(&packet[11], remote_addr); 
				memcpy(&remote_addr, &packet[11], sizeof(bd_addr_t));
				log_info("BNEP connection open succeeded to %s source UUID 0x%04x dest UUID: 0x%04x, max frame size %u", bd_addr_to_str(event_addr), uuid_source, uuid_dest, mtu);
				/* Setup network interface */
				btstack_network_up(bnep_cid, (uint8_t *)hci_local_bd_addr(), (uint8_t *)remote_addr);
				log_info("Network Interface %s activated", btstack_network_get_name(bnep_cid));

				bt_state = BT_STATE_CONNECTED;
				bt_timer = -1;
			}
			break;

		case BNEP_EVENT_CHANNEL_TIMEOUT:
			log_info("BNEP channel timeout! Channel will be closed");
			break;

		case BNEP_EVENT_CHANNEL_CLOSED:
			bnep_cid = little_endian_read_16(packet, 2);
			bt_channel_closed(bnep_cid);

			// 30秒後 BNEP Query 開始
			bt_state = BT_STATE_BNEP_QUERY_WAIT;
			bt_timer = 30 * 1000;
			break;

		case BNEP_EVENT_READY_TO_SEND:
			bnep_cid = little_endian_read_16(packet, 2);
			bt_send_packet(bnep_cid);
			break;

		default:
			break;
		}
		break;

	case RFCOMM_DATA_PACKET:
		// loopback
		if (rfcomm_channel_id) {
			int err = rfcomm_send_internal(rfcomm_channel_id, packet, size);
			if (err) {
				log_error("rfcomm_send_internal -> error %d", err);
			}
		}
		break;

	case BNEP_DATA_PACKET:
		bt_recv_packet(channel, packet, size);
		break;

	default:
		break;
	}
}

/*
 * @section Network packet handler
 *
 * @text A pointer to the network packet is stored and a BNEP_EVENT_CAN_SEND_NOW requested
 */

 /* LISTING_START(networkPacketHandler): Network Packet Handler */
static void network_send_packet_callback(uint16_t bnep_cid, const uint8_t * packet, uint16_t size) {
	if ((bnep_cid == 0) || (network_buffer_len > 0)) {
#ifdef PACKET_DUMP
		int len = size;
		printf("DROP %04x %d ", bnep_cid, size);
		if (len > 128)
			len = 128;
		printf_hexdump(packet, len);
#endif
		if (network_buffer_len > 0) {
			network_buffer_len = 0;
			btstack_network_packet_sent(network_buffer_cid);
		}
	}

	network_buffer_cid = bnep_cid;
	network_buffer = packet;
	network_buffer_len = size;
}

void bt_init(void)
{
	const hci_transport_t *transport;
	bt_control_t       * control;
	hci_uart_config_t  * config;
	remote_device_db_t * remote_db;
	service_record_item_t *spp_service_record_item;
	service_record_item_t *bnep_service_record_item;

	btstack_network_init(&network_send_packet_callback);

	//hci_dump_open("", HCI_DUMP_PACKETLOGGER);

	// start with BTstack init - especially configure HCI Transport
	btstack_memory_init();
	run_loop_init(RUN_LOOP_EMBEDDED);

	/* Initialize HCI */
	transport = hci_transport_usb_instance();
	control = NULL;
	config = NULL;
	remote_db = (remote_device_db_t *)&remote_device_db_memory;
	hci_init(transport, config, control, remote_db);
	hci_set_class_of_device(0x820300/*0x820810*/);

	/* Initialize L2CAP */
	l2cap_init();
	l2cap_register_packet_handler(packet_handler);

	/* Initialise SDP, create record for SPP and register with SDP */
	sdp_init();

	/* Initialize RFCOMM */
	rfcomm_init();
	rfcomm_register_packet_handler(packet_handler);
	rfcomm_register_service_internal(NULL, RFCOMM_SERVER_CHANNEL, 0xffff);  // reserved channel, mtu=100

	/* Initialise BNEP */
	bnep_init();
	bnep_register_packet_handler(packet_handler);
	if (bt_bnep_mode == 0) {
		bnep_register_service(NULL, BLUETOOTH_SERVICE_CLASS_PANU, BNEP_MTU_MIN);  /* Minimum L2CAP MTU for bnep is 1691 bytes */
	}
	else {
		bnep_register_service(NULL, BLUETOOTH_SERVICE_CLASS_NAP, BNEP_MTU_MIN);  /* Minimum L2CAP MTU for bnep is 1691 bytes */
	}

	memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
	memset(bnep_service_buffer, 0, sizeof(bnep_service_buffer));

	// サービス定義
	spp_service_record_item = (service_record_item_t *)spp_service_buffer;
	bnep_service_record_item = (service_record_item_t *)bnep_service_buffer;

	sdp_create_spp_service((uint8_t*)&spp_service_record_item->service_record, 1, "RX SPP");
	de_dump_data_element((uint8_t*)spp_service_record_item->service_record);
	log_info("SSP SDP service buffer size: %u", (uint16_t)(sizeof(service_record_item_t) + de_get_len((uint8_t*)spp_service_record_item->service_record)));
	sdp_register_service_internal(NULL, spp_service_record_item);

	if (bt_bnep_mode == 0) {
		pan_create_panu_service((uint8_t*)&bnep_service_record_item->service_record, "RX PANU", "RX PANU", BNEP_SECURITY_NONE);
		de_dump_data_element((uint8_t*)bnep_service_record_item->service_record);
		log_info("PANU SDP service buffer size: %u", (uint16_t)(sizeof(service_record_item_t) + de_get_len((uint8_t*)bnep_service_record_item->service_record)));
	}
	else {
		pan_create_nap_service((uint8_t*)&bnep_service_record_item->service_record, "RX NAP", "RX NAP", BNEP_SECURITY_NONE,
			PAN_NET_ACCESS_TYPE_100MB_ETHERNET, 0x0001312D0/*10Mb/sec*/, NULL, NULL);
		de_dump_data_element((uint8_t*)bnep_service_record_item->service_record);
		log_info("NAP SDP service buffer size: %u", (uint16_t)(sizeof(service_record_item_t) + de_get_len((uint8_t*)bnep_service_record_item->service_record)));
	}
	sdp_register_service_internal(NULL, bnep_service_record_item);

	// Secure Simple Pairing configuration -> just works
	hci_ssp_set_enable(1);
	hci_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
	hci_ssp_set_auto_accept(1);

	/* Turn on the device */
	hci_power_control(HCI_POWER_ON);

	/* Initialise SDP */
	sdp_parser_init();
	sdp_parser_register_callback(handle_sdp_client_query_result);

	bt_state = BT_STATE_POWER_ON;
	bt_timer = -1;
}

void bt_process(void)
{
	if (bt_state != BT_STATE_DORMANT) {
		run_loop_execute();
		bt_send_packet(0xffff);
	}

	if (bt_timer != 0)
		return;

	switch (bt_state) {
	case BT_STATE_BNEP_QUERY_WAIT:
		/* Send a general query for BNEP Protocol ID */
		log_info("Start SDP BNEP query.");
		sdp_general_query_for_uuid(remote_addr, BLUETOOTH_PROTOCOL_BNEP);

		bt_state = BT_STATE_BNEP_QUERY;
		bt_timer = -1;
		break;
	default:
		bt_timer = -1;
		break;
	}
}

int bt_can_send_packet(void)
{
	return network_buffer_len == 0;
}

void bt_send_packet(uint16_t bnep_cid)
{
	if (network_buffer_len > 0) {
		if (network_buffer_cid == 0) {
			network_buffer_len = 0;
			btstack_network_packet_sent(network_buffer_cid);
			return;
		}
		/* Check for parked network packets and send it out now */
		if (bnep_can_send_packet_now(network_buffer_cid)) {
			uint16_t len = network_buffer_len;
#ifdef PACKET_DUMP
			printf("ETH %u => ", len);
			if (len > 128)
				len = 128;
			printf_hexdump(network_buffer, len);
#endif
			// この関数はbnep_sendから再帰呼び出しされるので、
			// bnep_sendを呼び出すより前に送信パケットなし状態にします。
			network_buffer_len = 0;
			bnep_send(network_buffer_cid, (uint8_t*)network_buffer, len);
			btstack_network_packet_sent(network_buffer_cid);
		}
	}
}

void bt_recv_packet(uint16_t bnep_cid, const uint8_t * packet, uint16_t size)
{
#ifdef PACKET_DUMP
	int len = size;
	printf("ETH %d <= ", size);
	if (len > 128)
		len = 128;
	printf_hexdump(packet, len);
#endif
	btstack_network_process_packet(bnep_cid, packet, size);
}

void bt_channel_closed(uint16_t bnep_cid)
{
	log_info("BNEP channel closed");
	btstack_network_down(bnep_cid);
	if ((network_buffer_len > 0) && (network_buffer_cid == bnep_cid)) {
		network_buffer_len = 0;
		btstack_network_packet_sent(network_buffer_cid);
	}
}

void bt_terminate(void)
{
	hci_close();

	bt_state = BT_STATE_DORMANT;
	bt_timer = -1;
}

void bt_task_on_start(struct task_if_t *task, ID tskid)
{
}

void bt_task_on_end(struct task_if_t *task)
{
}

int bt_task_get_timer(struct task_if_t *task)
{
	return bt_timer;
}

void bt_task_progress(struct task_if_t *task, int elapse)
{
	if (bt_timer == -1)
		return;

	bt_timer -= elapse;
	if (bt_timer < 0)
		bt_timer = 0;
}

void bt_task_process(struct task_if_t *task, enum task_event_t event)
{
	extern void hal_tick_handler();

	switch (event & ~TASK_EVENT_TICK) {
	case TASK_EVENT_START:
		bt_init();
		break;
	case TASK_EVENT_TERMINATE:
		bt_terminate();
		break;
	case TASK_EVENT_WAKEUP:
	case TASK_EVENT_TIMEOUT:
		if (event & TASK_EVENT_TICK)
			hal_tick_handler();
		bt_process();
		break;
	}
}

struct task_if_t bt_task = {
	bt_task_on_start,
	bt_task_on_end,
	bt_task_get_timer,
	bt_task_progress,
	bt_task_process,
};
