#ifndef ACCESS_POINT_HANDLERS_H
#define ACCESS_POINT_HANDLERS_H

#include "http_router.h"

// ===========================================================================
// Public function declaration
// ===========================================================================
/**
 * @brief Root handler — captive portal home page.
 */
const char *handler_root(const struct http_request *req);

/**
 * @brief Captive portal handler — used for all probes
 *        (Android, Apple, Windows) and any unknown route.
 *        Returns the HTML page that triggers the popup.
 */
const char *handler_captive_portal(const struct http_request *req);

/**
 * @brief Encrypt handler — receives a message to encrypt.
 *
 * Expects a query parameter "msg" with the message to encrypt.
 * Returns a minimal 200 OK response (the JS fetch is fire-and-forget).
 *
 * @param req[in] Request parsed by router_parse().
 *
 * @return Complete HTTP response string (static, do not free).
 */
const char *handler_encrypt(const struct http_request *req);

#endif /* ACCESS_POINT_HANDLERS_H */
