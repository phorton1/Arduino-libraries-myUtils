#pragma once

#include <WiFi.h>
#include <esp_wifi.h>
#include "esp_now.h"

#define ESP_ERR_MYESP_BUSY	9999

#define SEND_STATE_OK			0
#define SEND_STATE_PENDING		1
#define SEND_STATE_FAIL			2
#define SEND_STATE_TIMEOUT		3


#define ACK_BIT					0x8000

#define MAX_BYTES				240




extern bool bindEspNowPeer(const unsigned char *peer_addr);


extern esp_err_t sendEspNow(const uint8_t *peer_addr, uint8_t *data, int len);
	// returns ESP_OK or an ESP error code or ESP_ERR_MYESP_BUSY
extern int checkEspNowSend();
	// returns SEND_STATE

extern void standardTest(const uint8_t *other_mac);




//