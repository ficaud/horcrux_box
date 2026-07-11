#ifndef ACCESS_POINT_HANDLERS_H
#define ACCESS_POINT_HANDLERS_H

#include "router.h"

/**
 * @brief Handler racine — page d'accueil du portail captif.
 */
const char *handler_root(const struct http_request *req);

/**
 * @brief Handler portail captif — utilisé pour toutes les sondes
 *        (Android, Apple, Windows) et toute route inconnue.
 *        Renvoie la page HTML qui déclenche la popup.
 */
const char *handler_captive_portal(const struct http_request *req);

#endif /* ACCESS_POINT_HANDLERS_H */
