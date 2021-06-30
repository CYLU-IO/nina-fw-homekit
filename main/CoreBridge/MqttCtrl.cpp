#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"


#include "esp_log.h"
#include "cJSON.h"
#include "mqtt_client.h"

#include <Arduino.h>

#include "CoreBridge.h"
#include "MqttCtrl.h"

static int s_mqttctrl_status = 255;

esp_mqtt_client_handle_t MqttCtrlClass::client;

static void log_error_if_nonzero(const char *message, int error_code)
{
  if (error_code != 0) printf("Last error %s: 0x%x\n", message, error_code);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    msg_id = esp_mqtt_client_subscribe(client, MQTT_URL_CMD, 2);
    s_mqttctrl_status = MQC_CONNECTED;
    break;

  case MQTT_EVENT_DISCONNECTED:
    s_mqttctrl_status = MQC_DISCONNECTED;
    esp_mqtt_client_reconnect(client);
    break;

  case MQTT_EVENT_SUBSCRIBED:
    printf("Triggered MQTT Status Publish\n");
    esp_mqtt_client_publish(client, MQTT_URL_STATUS, "{\"type\":\"CONNC\",\"value\":1}", 0, 2, 1);
    break;

  case MQTT_EVENT_DATA:
    if (strcmp(event->topic, MQTT_URL_CMD) == 0)
    {
      char cmd = event->data[0];
      int length = (event->data[1] & 0xff) | (event->data[2] << 8);

      switch (cmd)
      {
      case MQTT_CMD_REQUEST_DATA:
        /*switch (event->data[3]) {
          case 
        }*/
        break;

      case MQTT_CMD_DO_ACTION:
        for (int i = 0; i < length / 2; i++) {
          CoreBridge.setModuleValue(event->data[i * 2 + 3], event->data[i * 2 + 4]);
        }
        break;

      case MQTT_CMD_CONFIGURE:
        printf("Configure System\n");
        break;
      }
    }
    break;

  case MQTT_EVENT_ERROR:
    printf("MQTT_EVENT_ERROR\n");
    
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
    }
    break;

  default:
    printf("Other event id:%d\n", event->event_id);
    break;
  }
}

MqttCtrlClass::MqttCtrlClass()
{
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = "mqtt://10.144.1.38:1883",
      .lwt_topic = MQTT_URL_STATUS,
      .lwt_msg = "{\"type\":\"CONNC\",\"value\":0}",
      .lwt_qos = 0,
      .lwt_retain = 1,
      .lwt_msg_len = 12,
  };

  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);

  s_mqttctrl_status = MQC_IDLE_STATUS;
}

void MqttCtrlClass::begin()
{
  esp_mqtt_client_start(client);
}

int MqttCtrlClass::getStatus()
{
  return s_mqttctrl_status;
}

int MqttCtrlClass::reconnect()
{
  return esp_mqtt_client_reconnect(client);
}

int MqttCtrlClass::disconnect()
{
  return esp_mqtt_client_disconnect(client);
}

int MqttCtrlClass::stop()
{
  return esp_mqtt_client_stop(client);
}

int MqttCtrlClass::moduleUpdate(uint8_t index, const char *name, uint8_t value)
{
  cJSON *root;
	root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "MODULE_UPDATE");
  cJSON_AddStringToObject(root, "name", name);
  cJSON_AddNumberToObject(root, "index", index);
  cJSON_AddNumberToObject(root, "value", value);

  int ret = esp_mqtt_client_publish(client, MQTT_URL_STATUS, cJSON_Print(root), 0, 2, 0);
  cJSON_Delete(root);
  return ret;
}

MqttCtrlClass MqttCtrl;