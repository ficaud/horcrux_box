/*
 * Router — parse une requête HTTP brute et la dispatche vers
 * le handler correspondant.
 */

#include "router.h"
#include "handlers.h"

#include <string.h>

/* ---------------------------------------------------------------------------
 * Parsing
 * ------------------------------------------------------------------------- */

int router_parse(struct http_request *req, char *raw, size_t raw_len)
{
	char *ptr = raw;
	char *end = raw + raw_len;

	/* ── Méthode ── */
	/* Avance jusqu'au premier espace */
	while (ptr < end && *ptr != ' ') ptr++;
	if (ptr >= end) return -1;

	size_t method_len = ptr - raw;
	if (method_len >= ROUTER_METHOD_MAX) method_len = ROUTER_METHOD_MAX - 1;
	memcpy(req->method, raw, method_len);
	req->method[method_len] = '\0';

	/* ── Chemin ── */
	ptr++; /* saute l'espace */
	char *path_start = ptr;
	while (ptr < end && *ptr != ' ' && *ptr != '?') ptr++;

	size_t path_len = ptr - path_start;
	if (path_len >= ROUTER_PATH_MAX) path_len = ROUTER_PATH_MAX - 1;
	memcpy(req->path, path_start, path_len);
	req->path[path_len] = '\0';

	/* ── Body (après \r\n\r\n) ── */
	char *body_start = strstr(raw, "\r\n\r\n");
	if (body_start) {
		body_start += 4;
		req->body = body_start;
		req->body_len = raw_len - (body_start - raw);
	} else {
		req->body = NULL;
		req->body_len = 0;
	}

	return 0;
}

/* ---------------------------------------------------------------------------
 * Table de routage
 * ------------------------------------------------------------------------- */

typedef const char *(*handler_fn)(const struct http_request *);

static const struct {
	const char *path;
	handler_fn  handler;
} routes[] = {
	{ "/",                    handler_root },
	/* Sondes de portail captif — toutes → page captive */
	{ "/generate_204",        handler_captive_portal },
	{ "/gen_204",             handler_captive_portal },
	{ "/hotspot-detect.html", handler_captive_portal },
	{ "/success.txt",         handler_captive_portal },
	{ "/ncsi.txt",            handler_captive_portal },
	{ "/connecttest.txt",     handler_captive_portal },
	{ "/redirect",            handler_captive_portal },
	{ "/canonical.html",      handler_captive_portal },
	{ "/kindle-wifi/wifistub.html", handler_captive_portal },
};

#define ROUTE_COUNT (sizeof(routes) / sizeof(routes[0]))

/* ---------------------------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------------------------- */

const char *router_dispatch(const struct http_request *req)
{
	for (size_t i = 0; i < ROUTE_COUNT; i++) {
		if (strcmp(req->path, routes[i].path) == 0) {
			return routes[i].handler(req);
		}
	}

	return handler_captive_portal(req);
}
