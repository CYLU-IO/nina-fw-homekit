#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>
#include <hap.h>
#include <hap_platform_keystore.h>
#include "CoreBridge.h"

#include "Homekit.h"
#include "MqttCtrl.h"
#include "WifiManager.h"

module_t CoreBridgeClass::modules[MAX_MODULE_NUM];
int CoreBridgeClass::num_modules;

CoreBridgeClass::CoreBridgeClass()
{
  /*** Modules DB Initialization ***/
  module_t *modules = (module_t *)malloc(MAX_MODULE_NUM * sizeof(module_t));
  num_modules = 0;

  /*** Configuration Restoration ***/
  memset(&serial_number, 0x00, sizeof(serial_number));
  memset(&device_name, 0x00, sizeof(device_name));

  hap_platform_keystore_init_partition(hap_platform_keystore_get_nvs_partition_name(), false);
  size_t size;

  //Enable POP
  size = sizeof(enable_pop);
  if (hap_platform_keystore_get(hap_platform_keystore_get_nvs_partition_name(), "configurations", "enable_pop", (uint8_t *)&enable_pop, &size) != HAP_SUCCESS)
  {
    this->setEnablePOP(0);
  }

  //Device Name
  size = sizeof(device_name);
  if (hap_platform_keystore_get_str(hap_platform_keystore_get_nvs_partition_name(), "configurations", "device_name", (char *)&device_name, &size) != HAP_SUCCESS)
  {
    this->setDeviceName("CordBlock");
  }

  //Serial Number (factory NVS)
  strncpy((char *)serial_number, "TW0138WJC9T", SERIAL_NUMBER_LENGTH);
}

void CoreBridgeClass::init()
{
  Homekit.init();
  WifiMgr.begin();
  Warehouse.begin();
}

int CoreBridgeClass::setDeviceName(const char *name)
{
  strncpy((char *)device_name, name, DEVICE_NAME_LENGTH);
  hap_platform_keystore_set_str(hap_platform_keystore_get_nvs_partition_name(), "configurations", "device_name", (const char *)&device_name, sizeof(device_name));

  return ESP_OK;
}

int CoreBridgeClass::setEnablePOP(uint8_t state)
{
  enable_pop = state;
  hap_platform_keystore_set(hap_platform_keystore_get_nvs_partition_name(), "configurations", "enable_pop", (uint8_t *)&enable_pop, sizeof(enable_pop));

  return ESP_OK;
}

int CoreBridgeClass::createAccessory()
{
  this->deleteAccessory();

  return Homekit.createAccessory(serial_number, device_name);
}

int CoreBridgeClass::countAccessory()
{
  return Homekit.countAccessory();
}

int CoreBridgeClass::deleteAccessory()
{
  if (this->countAccessory() > 0)
    Homekit.deleteAccessory();

  num_modules = 0;

  return ESP_OK;
}

int CoreBridgeClass::beginHomekit()
{
  return Homekit.beginAccessory();
}

int CoreBridgeClass::addModule(uint8_t index, const char *name, uint8_t type, uint8_t priority, uint8_t state)
{
  modules[index].name = strdup(name);
  modules[index].type = type;
  modules[index].current = 0;
  modules[index].priority = priority;
  modules[index].state = state;
  modules[index].event_triggered = false;
  num_modules++;

  return ESP_OK;
}

int CoreBridgeClass::setModuleSwitchState(uint8_t index, uint8_t state, bool trigger)
{
  modules[index].state = state;

  if (trigger)
  {
    modules[index].event_triggered = true;
  }
  else
  {
    Homekit.setServiceValue((hap_char_t *)modules[index].hc, state);
    MqttCtrl.moduleUpdate(index, "switch_state", state);
  }

  return ESP_OK;
}

int CoreBridgeClass::setModuleSwitchState(uint8_t index, uint8_t state)
{
  return this->setModuleSwitchState(index, state, true);
}

int CoreBridgeClass::getModuleSwitchState(uint8_t index)
{
  return modules[index].state;
}

int CoreBridgeClass::setModuleCurrent(uint8_t index, uint16_t value)
{
  modules[index].current = value;

  MqttCtrl.moduleUpdate(index, "current", value);

  return ESP_OK;
}

int CoreBridgeClass::setModulePrioirty(uint8_t index, uint8_t value)
{
  modules[index].priority = value;

  MqttCtrl.moduleUpdate(index, "priority", value);

  return ESP_OK;
}

int CoreBridgeClass::getModulePrioirty(uint8_t index)
{
  return modules[index].priority;
}

int CoreBridgeClass::readModuleTriggered(uint8_t index)
{
  bool b = modules[index].event_triggered;
  modules[index].event_triggered = false; //clean after response

  return (int)b;
}

int CoreBridgeClass::getModuleNum()
{
  return num_modules;
}

module_t *CoreBridgeClass::getModule(uint8_t index)
{
  return &modules[index];
}

CoreBridgeClass CoreBridge;