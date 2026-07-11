/*
 * DNS Interceptor — intercepte toutes les requêtes DNS pour les rediriger
 * vers l'adresse IP du portail captif.
 */

#include "dns.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>

#include <string.h>
#include <errno.h>

LOG_MODULE_REGISTER(dns, LOG_LEVEL_INF);

/* ---------------------------------------------------------------------------
 * Configuration
 * ------------------------------------------------------------------------- */

#define DNS_PORT        53
#define DNS_STACK_SIZE  4096
#define DNS_PRIORITY    5
#define DNS_BUF_SIZE    256

/* Adresse du portail (doit correspondre à l'IP de l'AP) */
static const uint8_t portal_ip[4] = {192, 168, 4, 1};

/* ---------------------------------------------------------------------------
 * Construction de la réponse DNS
 * ------------------------------------------------------------------------- */

static void build_dns_response(uint8_t *buf, const uint8_t *query, int query_len)
{
	memcpy(buf, query, query_len);

	/* QR = 1 (response), RA = 1 (recursion available), pas d'erreur */
	buf[2] = 0x81;
	buf[3] = 0x80;

	/* ANCOUNT = 1 réponse */
	buf[6] = 0x00;
	buf[7] = 0x01;

	/* Réponse compressée : pointe vers le nom à l'offset 0x0C */
	int answer_pos = query_len;

	buf[answer_pos++] = 0xC0;
	buf[answer_pos++] = 0x0C;

	/* TYPE = A (0x0001) */
	buf[answer_pos++] = 0x00;
	buf[answer_pos++] = 0x01;

	/* CLASS = IN (0x0001) */
	buf[answer_pos++] = 0x00;
	buf[answer_pos++] = 0x01;

	/* TTL = 300 secondes */
	buf[answer_pos++] = 0x00;
	buf[answer_pos++] = 0x00;
	buf[answer_pos++] = 0x01;
	buf[answer_pos++] = 0x2C;

	/* RDLENGTH = 4 */
	buf[answer_pos++] = 0x00;
	buf[answer_pos++] = 0x04;

	/* IP du portail */
	memcpy(&buf[answer_pos], portal_ip, 4);
}

/* ---------------------------------------------------------------------------
 * Thread DNS
 * ------------------------------------------------------------------------- */

static void dns_thread_fn(void *arg1, void *arg2, void *arg3)
{
	int sock;
	struct sockaddr_in addr;
	uint8_t rx_buf[DNS_BUF_SIZE];
	uint8_t tx_buf[DNS_BUF_SIZE];
	int rx_len;

	sock = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		LOG_ERR("socket failed (%d)", errno);
		return;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(DNS_PORT);

	if (zsock_bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LOG_ERR("bind failed (%d)", errno);
		zsock_close(sock);
		return;
	}

	LOG_INF("DNS interceptor ready on port %d", DNS_PORT);

	while (1) {
		struct sockaddr_in client;
		socklen_t client_len = sizeof(client);

		rx_len = zsock_recvfrom(sock, rx_buf, sizeof(rx_buf), 0,
					(struct sockaddr *)&client, &client_len);
		if (rx_len < 12) {
			continue;
		}

		/* Répondre uniquement aux requêtes A standards (QR=0, OPCODE=0) */
		if (rx_buf[2] != 0x01 || rx_buf[3] != 0x00) {
			continue;
		}

		build_dns_response(tx_buf, rx_buf, rx_len);

		int tx_len = rx_len + 16;
		zsock_sendto(sock, tx_buf, tx_len, 0,
			     (struct sockaddr *)&client, client_len);
	}

	zsock_close(sock);
}

/* ---------------------------------------------------------------------------
 * Ressources du thread
 * ------------------------------------------------------------------------- */

K_THREAD_STACK_DEFINE(dns_stack, DNS_STACK_SIZE);
static struct k_thread dns_thread;

/* ---------------------------------------------------------------------------
 * API publique
 * ------------------------------------------------------------------------- */

int dns_interceptor_start(void)
{
	k_thread_create(&dns_thread, dns_stack, DNS_STACK_SIZE,
			dns_thread_fn, NULL, NULL, NULL,
			DNS_PRIORITY, 0, K_NO_WAIT);
	return 0;
}

int dns_interceptor_stop(void)
{
	/* TODO : implémenter un flag d'arrêt + k_thread_abort() */
	return 0;
}
