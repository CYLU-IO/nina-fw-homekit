#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <hap.h>

#include "CoreBridge.h"

#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

static const char *TAG = "Homekit";

hap_acc_t *HomekitClass::_accessory;

static int hk_identify(hap_acc_t *ha)
{
  return HAP_SUCCESS;
}

HomekitClass::HomekitClass()
{
}

int HomekitClass::init()
{
  hap_set_debug_level(HAP_DEBUG_LEVEL_INFO);

  return hap_init(HAP_TRANSPORT_WIFI);
}

/* Initialize the HAP core */
int HomekitClass::createAccessory(const char *serial, const char *name)
{
  int ret = 0;

  hap_acc_cfg_t cfg = {
      .name = (char *)name,
      .model = "CDBKV01TWA",
      .manufacturer = "CYLU.IO",
      .serial_num = (char *)serial,
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

  hap_update_config_number();

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

int HomekitClass::countAccessory()
{
  return hap_count_accessories();
}

int HomekitClass::beginAccessory()
{
  static bool first = true;
  int ret = HAP_SUCCESS;

  for (int i = 0; i < CoreBridge.getModuleNum(); i++)
  {
    module_t *module = CoreBridge.getModule(i);
    hap_serv_t *service;

    service = hap_serv_switch_create(module->state);
    hap_serv_add_char(service, hap_char_name_create((char *)module->name));

    hap_char_t *characteristic = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);

    hap_serv_set_write_cb(service, switchWrite);

    module->hs = service;
    module->hc = characteristic;
    hap_acc_add_serv(_accessory, service);
  }

  hap_add_accessory(_accessory);

  if (first)
  {
    ret = hap_start();
    first = false;
  }

  return ret;
}

int HomekitClass::deleteAccessory()
{
  hap_acc_delete(_accessory);

  return 0;
}

int HomekitClass::setServiceValue(hap_char_t *hc, uint8_t state)
{
  hap_val_t appliance_value = {
      .b = (bool)state,
  };

  return hap_char_update_val(hc, &appliance_value);
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
      for (int i = 0; i < CoreBridge.getModuleNum(); i++)
      {
        module_t *module = CoreBridge.getModule(i);

        if (hap_char_get_iid((hap_char_t *)module->hc) == hap_char_get_iid(write->hc))
        {
          uint8_t *addrs = new uint8_t[1]{(uint8_t)(i + 1)};
          uint8_t *acts = new uint8_t[1]{(uint8_t)(write->val.b ? DO_TURN_ON : DO_TURN_OFF)};

          CoreBridge.doModulesAction(addrs, acts, 1);
          //CoreBridge.setModuleSwitchState(i, (uint8_t)write->val.b);
          *(write->status) = HAP_STATUS_SUCCESS;
          break;
        }
      }
    }
    else
    {
      *(write->status) = HAP_STATUS_RES_ABSENT;
    }
  }

  return ret;
}

HomekitClass Homekit;