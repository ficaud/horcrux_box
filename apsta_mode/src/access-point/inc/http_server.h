#ifndef ACCESS_POINT_HTTP_SERVER_H
#define ACCESS_POINT_HTTP_SERVER_H

/**
 * @brief Démarre le serveur HTTP du portail captif (thread séparé, port TCP 80).
 *
 * @return 0 si OK, code d'erreur négatif sinon.
 */
int http_server_start(void);

/**
 * @brief Arrête le serveur HTTP et libère les ressources.
 *
 * @return 0 si OK, code d'erreur négatif sinon.
 */
int http_server_stop(void);

#endif /* ACCESS_POINT_HTTP_SERVER_H */
