#ifndef ACCESS_POINT_DHCP_H
#define ACCESS_POINT_DHCP_H

#include <zephyr/net/net_if.h>

/**
 * @brief Configure l'adresse IP et démarre le serveur DHCPv4 sur
 *        l'interface AP.
 *
 * @param iface Interface Wi-Fi AP (obtenue via net_if_get_wifi_sap()).
 * @return 0 si OK, négatif en cas d'erreur.
 */
int dhcp_server_start(struct net_if *iface);

#endif /* ACCESS_POINT_DHCP_H */
