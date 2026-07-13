/**
 * @file handlers.c
 *
 * @brief HTTP request handlers for the captive portal.
 *
 * @author Julien F.
 * @date 2026-07-12
 *
 * @details This module implements HTTP request handlers for the captive portal.
 *          It serves the appropriate HTML pages based on the request path.
 */

#include "handlers.h"

#include <zephyr/logging/log.h>

#include "page_captive.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LOG_MODULE_REGISTER(handlers, LOG_LEVEL_DBG);

// ===========================================================================
// Variables definition
// ===========================================================================
const char *handler_root(const struct http_request *req)
{
    (void)req;
    return PAGE_CAPTIVE;
}

const char *handler_captive_portal(const struct http_request *req)
{
    (void)req;
    /* Any unrecognised request = captive portal page.
     * This covers Android (/generate_204),
     * Apple (/hotspot-detect.html), Windows (/ncsi.txt), etc.
     * Receiving a 200 + HTML instead of the expected
     * response is what triggers the captive portal popup. */
    return PAGE_CAPTIVE;
}

// ===========================================================================
// Static functions
// ===========================================================================

/**
 * @brief URL-decode a string in-place.
 *
 * Decodes %XX sequences and converts '+' to space.
 */
static void url_decode(char *dst, const char *src, size_t dst_size)
{
    size_t i = 0;
    while (*src && i < dst_size - 1)
    {
        if (*src == '%' && *(src + 1) && *(src + 2))
        {
            char hex[3] = {src[1], src[2], '\0'};
            dst[i++] = (char)strtol(hex, NULL, 16);
            src += 3;
        }
        else if (*src == '+')
        {
            dst[i++] = ' ';
            src++;
        }
        else
        {
            dst[i++] = *src++;
        }
    }
    dst[i] = '\0';
}

// ===========================================================================
// Public functions
// ===========================================================================
const char *handler_test(const struct http_request *req)
{
    /* Parse the 'msg' query parameter */
    const char *query = req->query;
    const char *prefix = "msg=";
    const char *msg_start = strstr(query, prefix);

    if (msg_start)
    {
        msg_start += strlen(prefix); /* skip "msg=" */

        /* Find the end of the value (next '&' or end of string) */
        const char *msg_end = msg_start;
        while (*msg_end && *msg_end != '&')
            msg_end++;

        size_t msg_len = msg_end - msg_start;
        if (msg_len > 0)
        {
            /* Copy to temporary buffer and URL-decode */
            char msg_buf[128];
            size_t copy_len = msg_len;
            if (copy_len >= sizeof(msg_buf))
                copy_len = sizeof(msg_buf) - 1;

            char raw[sizeof(msg_buf)];
            memcpy(raw, msg_start, copy_len);
            raw[copy_len] = '\0';

            url_decode(msg_buf, raw, sizeof(msg_buf));

            LOG_INF("Secret to encrypt: %s", msg_buf);
        }
    }

    /* Return a minimal 200 OK (the JS fetch is fire-and-forget) */
    return "HTTP/1.1 200 OK\r\n"
           "Content-Length: 2\r\n"
           "Connection: close\r\n"
           "\r\n"
           "OK";
}
