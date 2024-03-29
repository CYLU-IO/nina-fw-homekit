#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"

#include "CoreBridge.h"

static EventGroupHandle_t s_wifi_event_group;

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const int WIFI_FAIL_BIT = BIT2;

static int s_retry_num = 0;
static int s_wifi_status = 255;

void smarconfig_task(void* parm);

void wifimgr_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    wifi_config_t wifi_config;
    esp_wifi_get_config((wifi_interface_t)WIFI_IF_STA, &wifi_config);

    ///// Connect to Previous Connected Network /////
    if (strlen((const char*)wifi_config.sta.ssid)) {
      WifiMgr.connect(wifi_config);
    } else {
      xTaskCreate(smarconfig_task, "smartconfig_task", 2048, NULL, 3, NULL);
    }
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    s_wifi_status = WL_DISCONNECTED;
    CoreBridge.digitalWrite(WIFI_STATE_PIN, 0);

    esp_wifi_connect();

    if (s_retry_num < 3) {
      esp_wifi_connect();
      s_retry_num++;
    } else {
      s_wifi_status = WL_CONNECT_FAILED;
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }

    xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    printf("WiFi Connected\n");
    s_retry_num = 0;
    s_wifi_status = WL_CONNECTED;

    ///// Remote Control Begins /////
    CoreBridge.initTime();

    if (MqttCtrl.getStatus() == MQC_IDLE_STATUS) MqttCtrl.connect();
    else MqttCtrl.reconnect();
    
    ///// Start Periodic Tasks /////
    xTaskCreate(onlinePeriodicTask, "custom_opt", 3072, NULL, 1, NULL);

    xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
  } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
    smartconfig_event_got_ssid_pswd_t* evt = (smartconfig_event_got_ssid_pswd_t*)event_data;
    wifi_config_t wifi_config;

    uint8_t ssid[33] = { 0 };
    uint8_t password[65] = { 0 };
    uint8_t rvd_data[33] = { 0 };

    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
    wifi_config.sta.bssid_set = evt->bssid_set;

    if (wifi_config.sta.bssid_set == true)
      memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));

    memcpy(ssid, evt->ssid, sizeof(evt->ssid));
    memcpy(password, evt->password, sizeof(evt->password));

    if (evt->type == SC_TYPE_ESPTOUCH_V2)
      esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data));

    s_wifi_status = WL_GOT_SSID_PWD;

    WifiMgr.connect(wifi_config);
  } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
    xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
  }
}

void smarconfig_task(void* parm) {
  EventBits_t uxBits;
  esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);

  smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
  esp_smartconfig_start(&cfg);

  s_wifi_status = WL_START_SCAN;

  while (1) {
    uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);

    if (uxBits & ESPTOUCH_DONE_BIT) {
      esp_smartconfig_stop();
      vTaskDelete(NULL);
    }

    taskYIELD();
  }
}

WifiManager::WifiManager() {}

void WifiManager::begin() {
  esp_netif_init();

  s_wifi_event_group = xEventGroupCreate();
  esp_event_loop_create_default();

  esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifimgr_event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifimgr_event_handler, NULL);
  esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &wifimgr_event_handler, NULL);

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();

  s_wifi_status = WL_IDLE_STATUS;
}

int WifiManager::getStatus() {
  return s_wifi_status;
}

void WifiManager::connect(wifi_config_t& config) {
  esp_wifi_disconnect();
  esp_wifi_set_config(WIFI_IF_STA, &config);
  esp_wifi_connect();
}

void WifiManager::resetNetwork() {
  esp_wifi_restore();
  esp_restart();
}

WifiManager WifiMgr;