//--------------------------------------------
// my
#include "myEspNow.h"
#include <myDebug.h>

#define dbg_send	1
#define dbg_recv	1

#define NOW_SEND_TIMEOUT	2000

static volatile bool in_on_data_sent;
static volatile uint16_t send_state;
static volatile uint32_t send_time;

// debug statistics


static uint32_t num_fail;
static uint32_t num_timeout;
static uint32_t num_dup;
static uint32_t num_out_of_order;
static uint32_t last_recv;
static uint32_t show_counter;


static void init_stats()
{
	display(0,"init_stats",0);
	num_fail = 0;
	num_timeout = 0;
	num_dup = 0;
	num_out_of_order = 0;
	last_recv = 0;
	show_counter = 0;
}


static void showMacAddress()
{
	// uint64_t mac = ESP.getEfuseMac();

	unsigned char mac[6];
	esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, mac);
	if (ret == ESP_OK)
		display(0,"MAC={0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}",
			  mac[0], mac[1], mac[2],
			  mac[3], mac[4], mac[5]);
	else
	my_error("Failed to read MAC address",0);
}


static void onDataReceived(const unsigned char *mac_addr, const uint8_t *data, int len)
{
	uint32_t val = *(uint32_t *)data;

	// for testing, if we notice the other guy just started, we reset our statistics
	
	if (val == 1)
		init_stats();

	// for testing, which might evolve to a sequence and crc scheme, we check
	// for duplicate and out of order receipts

	bool dup = val == last_recv;
	bool ok = val == last_recv + 1;
	last_recv = val;
	if (dup)
		num_dup++;
	if (!ok)
		num_out_of_order++;

	// and show a message if debugging or any problems

	if (dup || !ok || dbg_recv <= 0)
		display(0,"Received (%d) bytes - data=%d  %s %s",len,val,dup?"DUP":"",ok?"":"OUT OF ORDER");

}


static void onDataSent(const unsigned char *mac_addr, esp_now_send_status_t rslt)
{
	// re-entrancy protection to prevent checkEspNowSend()
	// from timing out in brief window between when we check
	// and reset send time

	in_on_data_sent = 1;
	if (send_time)
	{
		send_time = 0;
		if (rslt==ESP_NOW_SEND_SUCCESS)
		{
			send_state = SEND_STATE_OK;
			display(dbg_send,"SEND_STATE_OK",0);
		}
		else
		{
			send_state = SEND_STATE_FAIL;
			my_error("SEND_STATE_FAIL",0);
			num_fail++;
		}
	}
	else
	{
		warning(0,"unexpected onDataSent() in SEND_STATE(%d)",send_state);
	}
	in_on_data_sent = 0;
}




bool bindEspNowPeer(const unsigned char *peer_addr)
{
	WiFi.mode(WIFI_STA);
	showMacAddress();

	if (esp_now_init() != ESP_OK)
	{
		my_error("Error initializing ESP-NOW",0);
		return false;
	}

	esp_now_register_send_cb(onDataSent);
	esp_now_register_recv_cb(onDataReceived);

	esp_now_peer_info_t peerInfo;
	memset(&peerInfo, 0, sizeof(peerInfo));
	memcpy(peerInfo.peer_addr, peer_addr, 6);
	peerInfo.channel = 1;
	peerInfo.encrypt = false;

	if (esp_now_add_peer(&peerInfo) != ESP_OK)
	{
		my_error("Failed to add peer %02x:%02x:%02x:%02x:%02x:%02x",
			peer_addr[0],
			peer_addr[1],
			peer_addr[2],
			peer_addr[3],
			peer_addr[4],
			peer_addr[5]);
		return false;
	}
	return true;
}






esp_err_t sendEspNow(const uint8_t *peer_addr, uint8_t *data, int len)
{
	if (send_time)
	{
		my_error("sendEspNow() in SEND_STATE(%d) with send_time(%ul)",send_state,send_time);
		return ESP_ERR_MYESP_BUSY;
	}
	send_time = millis();
	send_state = SEND_STATE_PENDING;
	esp_err_t rslt = esp_now_send(peer_addr,data,len);
	if (rslt != ESP_OK)
	{
		send_time = 0;
		send_state = SEND_STATE_OK;
		my_error("sendEspNow() error(%d)",rslt);
		delay(20);
	}
	
	return rslt;
}




extern int checkEspNowSend()
{
	uint32_t now = millis();
	if (send_time &&
		!in_on_data_sent &&
		(now - send_time > NOW_SEND_TIMEOUT))
	{
		my_error("NOW_SEND_TIMEOUT(%d) now(%d)",send_time,now);
		send_time = 0;
		send_state = SEND_STATE_TIMEOUT;
		num_timeout++;
	}
	if (send_state != SEND_STATE_PENDING)
	{
		show_counter++;
		if (show_counter % 100 == 0)
		{
			display(0,"COUNT(%d) NUM_FAIL(%d) NUM_TIMEOUT(%d) NUM_DUP(%d) OUT_ORDER(%d)",
				show_counter,
				num_fail,
				num_timeout,
				num_dup,
				num_out_of_order);
		}
	}
	return send_state;
}



void standardTest(const uint8_t *other_mac)
{
	static uint32_t counter = 0;

	int state = checkEspNowSend();

	if (state != SEND_STATE_PENDING)
	{
		if (state == SEND_STATE_OK)
			counter++;

		display(dbg_send,"-----> sending %d",counter);

		esp_err_t err = sendEspNow(other_mac, (uint8_t *) &counter, 4);

		if (err != ESP_OK)
			delay(1000);
	}

	delay(20);	// standard 20ms delay between sends
}
