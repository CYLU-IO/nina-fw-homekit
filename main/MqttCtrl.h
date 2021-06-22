#ifndef MQTTCTRL_H
#define MQTTCTRL_H

#include "mqtt_client.h"

typedef enum
{
  MQC_IDLE_STATUS = 0,
  MQC_SUBSCRIBED = 1,
  MQC_UNSUBSCRIBED = 2,
  MQC_CONNECTED = 3,
  MQC_CONNECT_FAILED = 4,
  MQC_DISCONNECTED = 6
} mqc_status_t;

class MqttCtrlClass
{
private:
  static esp_mqtt_client_handle_t client;

public:
  MqttCtrlClass();

  void begin();

  int getStatus();

  int reconnect();

  int disconnect();

  int stop();
};

extern MqttCtrlClass MqttCtrl;

#endif