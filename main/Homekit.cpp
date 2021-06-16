#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

#include <hap.h>
#include "Homekit.h"

#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

static const char *TAG = "Homekit";

hap_acc_t *HomekitClass::_accessory;
module_t HomekitClass::modules[MAX_MODULE_NUM];

static int hk_identify(hap_acc_t *ha)
{
  ESP_LOGI(TAG, "Accessory identified");
  return HAP_SUCCESS;
}

HomekitClass::HomekitClass()
{
  module_t *modules = (module_t *)malloc(MAX_MODULE_NUM * sizeof(module_t));
}

int HomekitClass::init()
{
  memset(&_acc_serial, 0x00, sizeof(_acc_serial));
  memset(&_acc_name, 0x00, sizeof(_acc_name));

  hap_set_debug_level(HAP_DEBUG_LEVEL_ERR);

  return hap_init(HAP_TRANSPORT_WIFI);
}

/* Initialize the HAP core */
int HomekitClass::create(const char *serial, const char *name)
{
  int ret = 0;

  strncpy((char *)_acc_serial, serial, HK_ACC_SERIAL_MAX_LENGTH);
  strncpy((char *)_acc_name, name, HK_ACC_NAME_MAX_LENGTH);

  hap_acc_cfg_t cfg = {
      .name = (char *)_acc_name,
      .model = "CDBKV01TWA",
      .manufacturer = "Cylu Design",
      .serial_num = (char *)_acc_serial,
      .fw_rev = "1.0.0",
      .hw_rev = "1.0",
      .pv = "1.1.0",
      .cid = HAP_CID_OUTLET,
      .identify_routine = hk_identify,
  };
  /* Create accessory object */
  _accessory = hap_acc_create(&cfg);

  if (!_accessory)
    return HAP_FAIL;

/* Set hardcoded accessory code, later shoule be define in factory_nvs */
#ifdef CONFIG_EXAMPLE_USE_HARDCODED_SETUP_CODE
  hap_set_setup_code(CONFIG_EXAMPLE_SETUP_CODE);
  ret = hap_set_setup_id(CONFIG_EXAMPLE_SETUP_ID);
#else
  char setup_code[11] = {0};
  size_t setup_code_size = sizeof(setup_code);
  ret = hap_factory_keystore_get("hap_setup", "setup_code", (uint8_t *)setup_code, &setup_code_size);
#endif

  return ret;
}

/* Create the Switch Service */
int HomekitClass::addService(uint8_t index, uint8_t id, uint8_t state, const char *name)
{
  hap_serv_t *service;

  service = hap_serv_switch_create(state);
  hap_serv_add_char(service, hap_char_name_create((char *)name));

  /* Get pointer to the outlet in use characteristic which we need to monitor for state changes */
  hap_char_t *characteristic = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);

  /* Set the write callback for the service */
  hap_serv_set_write_cb(service, switchWrite);

  /* Add the Switch Service to the Accessory Object */
  int ret = hap_acc_add_serv(_accessory, service);

  module_t module = {
      .id = id,
      .name = (char *)name,
      .hs = service,
      .hc = characteristic,
      .event_triggered = false,
  };
  /* Create module db */
  modules[index] = module;

  return ret;
}

int HomekitClass::begin()
{
  static bool first = true;
  int ret = HAP_SUCCESS;
  //hap_enable_mfi_auth(HAP_MFI_AUTH_HW);

  /* Add the Accessory to the HomeKit Database */
  hap_add_accessory(_accessory);

  if (first)
  {
    ret = hap_start();
    first = false;
  }

  return ret;
}

int HomekitClass::getServiceValue(uint8_t index, uint8_t id)
{
  const hap_val_t *cur_val = hap_char_get_val((hap_char_t *)modules[index].hc);

  return cur_val->i;
}

int HomekitClass::setServiceValue(uint8_t index, uint8_t id, uint8_t state)
{
  hap_char_t *hc = modules[index].hc;
  hap_val_t appliance_value = {
      .b = (bool)state,
  };

  return hap_char_update_val(hc, &appliance_value);
}

int HomekitClass::readTriggered(uint8_t index, uint8_t id)
{
  bool b = modules[index].event_triggered;
  modules[index].event_triggered = false; //clean after response

  return (int)b;
}

int HomekitClass::switchWrite(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv)
{
  int i, ret = HAP_SUCCESS;
  hap_write_data_t *write;

  for (i = 0; i < count; i++)
  {
    write = &write_data[i];

    if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON))
    {
      ESP_LOGI(TAG, "Received Write. Switch %s", write->val.b ? "On" : "Off");

      for (int i = 0; i < MAX_MODULE_NUM; i++)
      {
        hap_char_t *m_hc = HomekitClass::modules[i].hc;

        if (hap_char_get_iid(m_hc) == hap_char_get_iid(write->hc))
        {
          HomekitClass::modules[i].event_triggered = true;
          break;
        }
      }

      hap_char_update_val(write->hc, &(write->val));
      *(write->status) = HAP_STATUS_SUCCESS;
    }
    else
    {
      *(write->status) = HAP_STATUS_RES_ABSENT;
    }
  }

  return ret;
}

void HomekitClass::deleateAccessory()
{
  hap_acc_delete(_accessory);
}

int HomekitClass::resetToFactory()
{
  hap_reset_homekit_data();
  return hap_reset_to_factory();
}

HomekitClass Homekit;