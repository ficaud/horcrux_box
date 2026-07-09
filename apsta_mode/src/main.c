/*
 * Copyright (c) 2024 Muhammad Haziq
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/dhcpv4_server.h>
#include <zephyr/net/socket.h>
#include <string.h>

LOG_MODULE_REGISTER(MAIN);

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

#define NET_EVENT_WIFI_MASK                                                                        \
	(NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT |                        \
	 NET_EVENT_WIFI_AP_ENABLE_RESULT | NET_EVENT_WIFI_AP_DISABLE_RESULT |                      \
	 NET_EVENT_WIFI_AP_STA_CONNECTED | NET_EVENT_WIFI_AP_STA_DISCONNECTED)

static struct net_if *ap_iface;

static struct wifi_connect_req_params ap_config;

static struct net_mgmt_event_callback cb;

/* Check necessary definitions */

BUILD_ASSERT(sizeof(CONFIG_WIFI_SAMPLE_AP_SSID) > 1,
	     "CONFIG_WIFI_SAMPLE_AP_SSID is empty. Please set it in conf file.");

BUILD_ASSERT(sizeof(CONFIG_WIFI_SAMPLE_SSID) > 1,
	     "CONFIG_WIFI_SAMPLE_SSID is empty. Please set it in conf file.");

#if CONFIG_WIFI_SAMPLE_DHCPV4_START
BUILD_ASSERT(sizeof(CONFIG_WIFI_SAMPLE_AP_IP_ADDRESS) > 1,
	     "CONFIG_WIFI_SAMPLE_AP_IP_ADDRESS is empty. Please set it in conf file.");

BUILD_ASSERT(sizeof(CONFIG_WIFI_SAMPLE_AP_NETMASK) > 1,
	     "CONFIG_WIFI_SAMPLE_AP_NETMASK is empty. Please set it in conf file.");

#endif

/* -------------------------------------------------------------------------
 * Synchronisation
 * -------------------------------------------------------------------------
 */

static struct k_sem ap_ready_sem;

/* -------------------------------------------------------------------------
 * Captive Portal HTTP Server
 * -------------------------------------------------------------------------
 */

#define HTTP_PORT		80
#define HTTP_BACKLOG		5
#define HTTP_SRV_STACK_SIZE	4096
#define HTTP_SRV_PRIORITY	5

static const char http_captive_page[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<html>\r\n"
	"<head><title>Captive Portal</title></head>\r\n"
	"<body>\r\n"
	"<h1>Hello World!</h1>\r\n"
	"<p>Bienvenue sur le portail captif</p>\r\n"
	"</body>\r\n"
	"</html>\r\n";

static void http_server_thread(void *arg1, void *arg2, void *arg3)
{
	int server_fd, client_fd;
	struct sockaddr_in addr;
	int opt = 1;

	server_fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_fd < 0) {
		LOG_ERR("HTTP: failed to create socket (%d)", errno);
		return;
	}

	zsock_setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(HTTP_PORT);

	if (zsock_bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LOG_ERR("HTTP: bind failed (%d)", errno);
		zsock_close(server_fd);
		return;
	}

	if (zsock_listen(server_fd, HTTP_BACKLOG) < 0) {
		LOG_ERR("HTTP: listen failed (%d)", errno);
		zsock_close(server_fd);
		return;
	}

	LOG_INF("Captive portal ready on http://%s:%d",
		 CONFIG_WIFI_SAMPLE_AP_IP_ADDRESS, HTTP_PORT);

	while (1) {
		client_fd = zsock_accept(server_fd, NULL, NULL);
		if (client_fd < 0) {
			LOG_ERR("HTTP: accept failed (%d)", errno);
			continue;
		}

		zsock_send(client_fd, http_captive_page, sizeof(http_captive_page) - 1, 0);
		zsock_close(client_fd);
	}
}

K_THREAD_DEFINE(http_srv_tid, HTTP_SRV_STACK_SIZE,
		http_server_thread, NULL, NULL, NULL,
		HTTP_SRV_PRIORITY, 0, 0);

static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event,
			       struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT: {
		LOG_INF("Connected to %s", CONFIG_WIFI_SAMPLE_SSID);
		break;
	}
	case NET_EVENT_WIFI_DISCONNECT_RESULT: {
		LOG_INF("Disconnected from %s", CONFIG_WIFI_SAMPLE_SSID);
		break;
	}
	case NET_EVENT_WIFI_AP_ENABLE_RESULT: {
		struct wifi_status *status = (struct wifi_status *)cb->info;

		if (status->status) {
			LOG_ERR("AP enable failed (%d)", status->status);
		} else {
			LOG_INF("AP Mode is enabled. Starting captive portal...");
			k_sem_give(&ap_ready_sem);
		}
		break;
	}
	case NET_EVENT_WIFI_AP_DISABLE_RESULT: {
		LOG_INF("AP Mode is disabled.");
		break;
	}
	case NET_EVENT_WIFI_AP_STA_CONNECTED: {
		struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

		LOG_INF("station: " MACSTR " joined ", sta_info->mac[0], sta_info->mac[1],
			sta_info->mac[2], sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
		break;
	}
	case NET_EVENT_WIFI_AP_STA_DISCONNECTED: {
		struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

		LOG_INF("station: " MACSTR " leave ", sta_info->mac[0], sta_info->mac[1],
			sta_info->mac[2], sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
		break;
	}
	default:
		break;
	}
}

#if CONFIG_WIFI_SAMPLE_DHCPV4_START
static void enable_dhcpv4_server(void)
{
	static struct net_in_addr addr;
	static struct net_in_addr netmaskAddr;

	if (net_addr_pton(NET_AF_INET, CONFIG_WIFI_SAMPLE_AP_IP_ADDRESS, &addr)) {
		LOG_ERR("Invalid address: %s", CONFIG_WIFI_SAMPLE_AP_IP_ADDRESS);
		return;
	}

	if (net_addr_pton(NET_AF_INET, CONFIG_WIFI_SAMPLE_AP_NETMASK, &netmaskAddr)) {
		LOG_ERR("Invalid netmask: %s", CONFIG_WIFI_SAMPLE_AP_NETMASK);
		return;
	}

	net_if_ipv4_set_gw(ap_iface, &addr);

	if (net_if_ipv4_addr_add(ap_iface, &addr, NET_ADDR_MANUAL, 0) == NULL) {
		LOG_ERR("unable to set IP address for AP interface");
	}

	if (!net_if_ipv4_set_netmask_by_addr(ap_iface, &addr, &netmaskAddr)) {
		LOG_ERR("Unable to set netmask for AP interface: %s",
			 CONFIG_WIFI_SAMPLE_AP_NETMASK);
	}

	addr.s4_addr[3] += 10; /* Starting IPv4 address for DHCPv4 address pool. */

	if (net_dhcpv4_server_start(ap_iface, &addr) != 0) {
		LOG_ERR("DHCP server is not started for desired IP");
		return;
	}

	LOG_INF("DHCPv4 server started...\n");
}
#endif

static int enable_ap_mode(void)
{
	if (!ap_iface) {
		LOG_INF("AP: is not initialized");
		return -EIO;
	}

	LOG_INF("Turning on AP Mode");
	ap_config.ssid = (const uint8_t *)CONFIG_WIFI_SAMPLE_AP_SSID;
	ap_config.ssid_length = sizeof(CONFIG_WIFI_SAMPLE_AP_SSID) - 1;
	ap_config.psk = (const uint8_t *)CONFIG_WIFI_SAMPLE_AP_PSK;
	ap_config.psk_length = sizeof(CONFIG_WIFI_SAMPLE_AP_PSK) - 1;
	ap_config.channel = WIFI_CHANNEL_ANY;
	ap_config.band = WIFI_FREQ_BAND_2_4_GHZ;

	if (sizeof(CONFIG_WIFI_SAMPLE_AP_PSK) == 1) {
		ap_config.security = WIFI_SECURITY_TYPE_NONE;
	} else {

		ap_config.security = WIFI_SECURITY_TYPE_PSK;
	}

#if CONFIG_WIFI_SAMPLE_DHCPV4_START
	enable_dhcpv4_server();
#endif

	int ret = net_mgmt(NET_REQUEST_WIFI_AP_ENABLE, ap_iface, &ap_config,
			   sizeof(struct wifi_connect_req_params));
	if (ret) {
		LOG_ERR("NET_REQUEST_WIFI_AP_ENABLE failed, err: %d", ret);
	}

	return ret;
}

int main(void)
{
	k_sleep(K_SECONDS(5));

	k_sem_init(&ap_ready_sem, 0, 1);

	net_mgmt_init_event_callback(&cb, wifi_event_handler, NET_EVENT_WIFI_MASK);
	net_mgmt_add_event_callback(&cb);

	/* Get AP interface in AP-STA mode. */
	ap_iface = net_if_get_wifi_sap();

	enable_ap_mode();

	/* Wait for the AP to be fully enabled before starting the HTTP server */
	k_sem_take(&ap_ready_sem, K_FOREVER);

	/* Start the captive portal HTTP server thread */
	k_thread_start(http_srv_tid);

	return 0;
}
