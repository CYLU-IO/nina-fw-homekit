#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

typedef enum {
  WL_IDLE_STATUS = 0,
  WL_START_SCAN = 1,
  WL_GOT_SSID_PWD = 2,
  WL_CONNECTED = 3,
  WL_CONNECT_FAILED = 4,
  WL_DISCONNECTED = 6
} wl_status_t;

class WifiManager {
public:
  WifiManager();

  void begin();

  int getStatus();

  void connect(wifi_config_t& config);

  void resetNetwork();
};

extern WifiManager WifiMgr;

#endif