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
  if (error_code != 0)
    printf("Last error %s: 0x%x\n", message, error_code);
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
        switch (event->data[3])
        {
        case MQTT_DATA_MODULES_DATA:
          MqttCtrl.modulesUpdate();
          break;

        case MQTT_DATA_CONFIGURATIONS:
          MqttCtrl.configurationsUpdate();
          break;
        }
        break;

      case MQTT_CMD_DO_ACTION:

        for (int i = 0; i < (length - 1) / 2; i++)
        {
          switch (event->data[3])
          {
          case MQTT_DATA_SWITCH_STATE:
            CoreBridge.setModuleSwitchState(event->data[i * 2 + 4], event->data[i * 2 + 5]);
            break;

          case MQTT_DATA_PRIORITY:
            CoreBridge.setModulePrioirty(event->data[i * 2 + 4], event->data[i * 2 + 5]);
            break;
          }
        }
        break;

      case MQTT_CMD_CONFIGURE:
        switch (event->data[3])
        {
        case MQTT_CONFIG_DEVICE_NAME:
          char device_name[DEVICE_NAME_LENGTH + 1];
          memset(device_name, 0x00, sizeof(device_name));
          memcpy(device_name, &event->data[4], length - 1);

          printf("Configure DEVICE_NAME to %s\n", device_name);

          CoreBridge.setDeviceName(device_name);
          break;

        case MQTT_CONFIG_ENABLE_POP:
          printf("Configure ENABLE_POP to %i\n", event->data[4]);
          CoreBridge.setEnablePOP((int8_t)event->data[4]);
          break;
        }
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
    //printf("Other event id:%d\n", event->event_id);
    break;
  }
}

MqttCtrlClass::MqttCtrlClass()
{
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = "ws://10.144.1.38:1883/mqtt",
      .lwt_topic = MQTT_URL_STATUS,
      .lwt_msg = "{\"type\":\"CONNC\",\"value\":0}",
      .lwt_qos = 0,
      .lwt_retain = 1,
      .lwt_msg_len = 26,
  };

  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);

  s_mqttctrl_status = MQC_IDLE_STATUS;
  warehouse_request_addr = 0x00;
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

int MqttCtrlClass::moduleUpdate(uint8_t index, const char *name, int value)
{
  cJSON *root;
  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "MODULE_UPDATE");
  cJSON_AddNumberToObject(root, "index", index);
  cJSON_AddStringToObject(root, "name", name);
  cJSON_AddNumberToObject(root, "value", value);

  int ret = esp_mqtt_client_publish(client, MQTT_URL_STATUS, cJSON_Print(root), 0, 2, 0);
  cJSON_Delete(root);
  return ret;
}

int MqttCtrlClass::moduleUpdate(uint8_t index, const char *name, const char *value)
{
  cJSON *root;
  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "MODULE_UPDATE");
  cJSON_AddNumberToObject(root, "index", index);
  cJSON_AddStringToObject(root, "name", name);
  cJSON_AddStringToObject(root, "value", value);

  int ret = esp_mqtt_client_publish(client, MQTT_URL_STATUS, cJSON_Print(root), 0, 2, 0);
  cJSON_Delete(root);
  return ret;
}

int MqttCtrlClass::modulesUpdate()
{
  cJSON *root;
  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "MODULES_UPDATE");

  cJSON *pack = cJSON_CreateArray();
  for (int i = 0; i < CoreBridge.getModuleNum(); i++)
  {
    cJSON *m;
    module_t *module = CoreBridge.getModule(i);

    cJSON_AddItemToArray(pack, m = cJSON_CreateObject());
    cJSON_AddNumberToObject(m, "index", i);
    cJSON_AddStringToObject(m, "name", module->name);
    cJSON_AddNumberToObject(m, "type", module->type);
    cJSON_AddNumberToObject(m, "priority", module->priority);
    cJSON_AddNumberToObject(m, "current", module->current);
    cJSON_AddNumberToObject(m, "switch_state", module->state);
  }

  cJSON_AddItemToObject(root, "value", pack);

  int ret = esp_mqtt_client_publish(client, MQTT_URL_STATUS, cJSON_Print(root), 0, 2, 0);
  cJSON_Delete(root);
  return ret;
}

int MqttCtrlClass::configurationsUpdate()
{
  cJSON *root;
  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "CONFIGURATIONS_UPDATE");

  cJSON_AddStringToObject(root, "device_name", CoreBridge.device_name);
  cJSON_AddStringToObject(root, "serial_number", CoreBridge.serial_number);
  cJSON_AddNumberToObject(root, "enable_pop", CoreBridge.enable_pop);

  int ret = esp_mqtt_client_publish(client, MQTT_URL_STATUS, cJSON_Print(root), 0, 2, 0);
  cJSON_Delete(root);
  return ret;
}

void MqttCtrlClass::requestWarehouseLength()
{
  warehouse_request_addr = 0x01; //represent asking the length
}

void MqttCtrlClass::setWarehouseLength(uint16_t length)
{
  warehouse_available_length = length;
}

void MqttCtrlClass::setWarehouseRequest(uint8_t offset) //offest < 30
{
  /**
   * A day of data as one pack, so the offest unit is day
   **/

  if (offset >= 30)
    offset = 30;

  warehouse_request_addr = 2 + (144 * offset);
}

int MqttCtrlClass::warehouseRequestBufferUpdate(int *buf, uint8_t length)
{
  cJSON *root;
  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "CURRENT_HISTORY_UPDATE");
  cJSON_AddNumberToObject(root, "offset", (warehouse_request_addr - 2) / 144);
  cJSON_AddItemToObject(root, "value", cJSON_CreateIntArray(buf, length));

  int ret = esp_mqtt_client_publish(client, MQTT_URL_STATUS, cJSON_Print(root), 0, 2, 0);
  cJSON_Delete(root);
  return ret;
}

MqttCtrlClass MqttCtrl;