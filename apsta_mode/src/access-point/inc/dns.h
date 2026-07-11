#ifndef ACCESS_POINT_DNS_H
#define ACCESS_POINT_DNS_H

/**
 * @brief Démarre l'intercepteur DNS (thread séparé, port UDP 53).
 *        Toute requête DNS de type A reçoit l'IP du portail captif.
 *
 * @return 0 si OK, code d'erreur négatif sinon.
 */
int dns_interceptor_start(void);

/**
 * @brief Arrête l'intercepteur DNS et libère les ressources.
 *
 * @return 0 si OK, code d'erreur négatif sinon.
 */
int dns_interceptor_stop(void);

#endif /* ACCESS_POINT_DNS_H */
