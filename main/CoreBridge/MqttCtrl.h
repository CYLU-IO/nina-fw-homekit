#ifndef MQTTCTRL_H
#define MQTTCTRL_H

#include "mqtt_client.h"

#define MQTT_URL_STATUS "CYLU/CordBlock/TW0138WJC9T/12321123/Status"
#define MQTT_URL_CMD "CYLU/CordBlock/TW0138WJC9T/12321123/Cmd"

#define MQTT_CMD_REQUEST_DATA 0x41
#define MQTT_CMD_DO_ACTION 0x42
#define MQTT_CMD_CONFIGURE 0x43

#define MQTT_DATA_SERVICE_STATUS 0x61
#define MQTT_DATA_CONFIGURATIONS 0x62
#define MQTT_DATA_SYSTEM_CURRENT 0x63
#define MQTT_DATA_SUPPLY_VOLTAGE 0x64
#define MQTT_DATA_CURRENT_HISTORY 0x65
#define MQTT_DATA_MODULES_DATA 0x66
#define MQTT_DATA_SWITCH_STATE 0x67
#define MQTT_DATA_PRIORITY 0x68

#define MQTT_CONFIG_DEVICE_NAME 0x70
#define MQTT_CONFIG_ENABLE_POP 0x71
#define MQTT_CONFIG_RESET_TO_FACTORY 0x72

#define MQTT_DO_TURN_OFF 0x7a
#define MQTT_DO_TURN_ON 0x79

typedef enum {
  MQC_IDLE_STATUS = 0,
  MQC_SUBSCRIBED = 1,
  MQC_UNSUBSCRIBED = 2,
  MQC_CONNECTED = 3,
  MQC_CONNECT_FAILED = 4,
  MQC_DISCONNECTED = 6
} mqc_status_t;

class MqttCtrlClass {
private:
  static esp_mqtt_client_handle_t client;

public:
  MqttCtrlClass();

  void begin();

  int getStatus();
  int reconnect();
  int disconnect();
  int stop();

  int moduleUpdate(uint8_t index, const char* name, int value);
  int moduleUpdate(uint8_t index, const char* name, const char* value);

  int modulesUpdate();

  int configurationsUpdate();

  int warehouseDataUpdate(uint8_t type);
};

extern MqttCtrlClass MqttCtrl;

#endif