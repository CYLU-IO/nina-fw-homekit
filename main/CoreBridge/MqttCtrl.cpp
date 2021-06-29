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
#include "mqtt_client.h"

#include <Arduino.h>

#include "MqttCtrl.h"

static const char *TAG = "MQTTCTRL";

static int s_mqttctrl_status = 255;

esp_mqtt_client_handle_t MqttCtrlClass::client;

static void log_error_if_nonzero(const char *message, int error_code)
{
  if (error_code != 0)
  {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    msg_id = esp_mqtt_client_subscribe(client, MQTT_URL_CMD, 2);

    esp_mqtt_client_publish(client, MQTT_URL_STATUS, "connected", 0, 2, 0);
    s_mqttctrl_status = MQC_CONNECTED;
    break;

  case MQTT_EVENT_DISCONNECTED:
    s_mqttctrl_status = MQC_DISCONNECTED;
    break;

  case MQTT_EVENT_SUBSCRIBED:
    //printf("Triggered MQTT Status Publish\n");
    //esp_mqtt_client_publish(client, MQTT_URL_STATUS, "test_msg", 0, 2, 0);
    break;

  case MQTT_EVENT_DATA:
    if (strcmp(event->topic, MQTT_URL_CMD) == 0)
    {
      char cmd = event->data[0];
      int length = (event->data[1] & 0xff) | (event->data[2] << 8);

      printf("MQTT Data Length: %i\n", length);

      switch (cmd)
      {
      case MQTT_CMD_REQUEST_DATA:
        /*switch (event->data[3]) {
          case 
        }*/
        break;

      case MQTT_CMD_DO_ACTION:
        /*switch (event->data[3]) {
          
        }*/
        break;

      case MQTT_CMD_CONFIGURE:
        printf("Configure System\n");
        break;
      }

      printf("MQTT Data Length: %i\n", length);
    }
    break;

  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;

  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

MqttCtrlClass::MqttCtrlClass()
{
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = "mqtt://10.144.1.38:1883",
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

MqttCtrlClass MqttCtrl;