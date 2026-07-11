#ifndef ACCESS_POINT_ROUTER_H
#define ACCESS_POINT_ROUTER_H

#include <stddef.h>

/* Taille max des buffers de parsing */
#define ROUTER_METHOD_MAX   8
#define ROUTER_PATH_MAX     64

/**
 * @brief Requête HTTP parsée.
 *
 * Les pointeurs body / query pointent dans le buffer brut
 * (pas d'allocation dynamique).
 */
struct http_request {
	char method[ROUTER_METHOD_MAX];
	char path[ROUTER_PATH_MAX];
	const char *body;       /* pointe dans raw (NULL si pas de body) */
	size_t      body_len;
};

/**
 * @brief Parse une requête HTTP brute.
 *
 * Extrait la méthode, le chemin, et repère le début du body.
 * Le buffer @p raw est modifié sur place (ajout de \0).
 *
 * @param req    Structure à remplir.
 * @param raw    Buffer contenant la requête HTTP.
 * @param raw_len Nombre d'octets lus.
 * @return 0 si OK, -1 si parsing impossible.
 */
int router_parse(struct http_request *req, char *raw, size_t raw_len);

/**
 * @brief Dispatch la requête vers le bon handler.
 *
 * @param req Requête parsée par router_parse().
 * @return Chaîne HTTP complète prête à être envoyée (statique, ne pas libérer).
 */
const char *router_dispatch(const struct http_request *req);

#endif /* ACCESS_POINT_ROUTER_H */
