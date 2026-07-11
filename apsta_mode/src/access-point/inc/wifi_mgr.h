#ifndef ACCESS_POINT_WIFI_MGR_H
#define ACCESS_POINT_WIFI_MGR_H

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

// Event mask for Wi-Fi events we are interested in
#define NET_EVENT_WIFI_MASK                                                                    \
	(NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT |                        \
	 NET_EVENT_WIFI_AP_ENABLE_RESULT | NET_EVENT_WIFI_AP_DISABLE_RESULT |                      \
	 NET_EVENT_WIFI_AP_STA_CONNECTED | NET_EVENT_WIFI_AP_STA_DISCONNECTED)

// ===========================================================================
// Public function declaration
// ===========================================================================
/**
 * @brief Initialise le Wi-Fi en mode Point d'Accès (AP).
 *
 * Étapes :
 *  1. Attend 5s que le driver Wi-Fi s'initialise
 *  2. Enregistre le callback d'événements Wi-Fi
 *  3. Récupère l'interface AP (net_if_get_wifi_sap)
 *  4. Configure SSID/PSK/canal et active l'AP
 *  5. Démarre le serveur DHCPv4
 *  6. Bloque jusqu'à réception de NET_EVENT_WIFI_AP_ENABLE_RESULT
 *
 * @return 0 si l'AP est prêt, négatif en cas d'erreur.
 */
int wifi_mgr_init(void);

#endif /* ACCESS_POINT_WIFI_MGR_H */
