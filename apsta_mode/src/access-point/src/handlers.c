/*
 * Handlers — génèrent les réponses HTTP pour chaque route.
 */

#include "handlers.h"

/* ---------------------------------------------------------------------------
 * Pages
 * ------------------------------------------------------------------------- */

static const char page_captive[] =
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

/* ---------------------------------------------------------------------------
 * Handlers
 * ------------------------------------------------------------------------- */

const char *handler_root(const struct http_request *req)
{
	(void)req;
	return page_captive;
}

const char *handler_captive_portal(const struct http_request *req)
{
	(void)req;
	/* Toute requête non reconnue = page du portail captif.
	 * Cela couvre les sondes Android (/generate_204),
	 * Apple (/hotspot-detect.html), Windows (/ncsi.txt), etc.
	 * C'est le fait de recevoir un 200 + HTML au lieu de la
	 * réponse attendue qui déclenche la popup de portail captif. */
	return page_captive;
}
