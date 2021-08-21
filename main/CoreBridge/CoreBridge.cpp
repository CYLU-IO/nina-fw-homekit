#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

#include <hap_platform_keystore.h>

#include <CoreBridge.h>

///// Variables Definition /////
system_status_t CoreBridgeClass::system_status;
smart_modularized_fuse_status_t CoreBridgeClass::smf_status;

CoreBridgeClass::CoreBridgeClass() {
  ///// Configuration Restoration /////
  memset(&serial_number, 0x00, sizeof(serial_number));
  memset(&device_name, 0x00, sizeof(device_name));
  hap_platform_keystore_init_partition(hap_platform_keystore_get_nvs_partition_name(), false);
  size_t size;

  //Enable POP
  size = sizeof(smf_status.enable_pop);
  if (hap_platform_keystore_get(hap_platform_keystore_get_nvs_partition_name(), "configurations", "enable_pop", (uint8_t*)&smf_status.enable_pop, &size) != HAP_SUCCESS)
    this->setEnablePOP(0);

  //Device Name
  size = sizeof(device_name);
  if (hap_platform_keystore_get_str(hap_platform_keystore_get_nvs_partition_name(), "configurations", "device_name", (char*)&device_name, &size) != HAP_SUCCESS)
    this->setDeviceName("CordBlock");

  //Serial Number (factory NVS)
  strncpy((char*)serial_number, "TW0138WJC9T", SERIAL_NUMBER_LENGTH);
}

void CoreBridgeClass::init() {
  Homekit.init();
  WifiMgr.begin();
  Warehouse.begin();
}

int CoreBridgeClass::setDeviceName(const char* name) {
  strncpy((char*)device_name, name, DEVICE_NAME_LENGTH);
  hap_platform_keystore_set_str(hap_platform_keystore_get_nvs_partition_name(), "configurations", "device_name", (const char*)&device_name, sizeof(device_name));

  return ESP_OK;
}

int CoreBridgeClass::setEnablePOP(uint8_t state) {
  smf_status.enable_pop = state;
  hap_platform_keystore_set(hap_platform_keystore_get_nvs_partition_name(), "configurations", "enable_pop", (uint8_t*)&smf_status.enable_pop, sizeof(smf_status.enable_pop));

  return ESP_OK;
}

int CoreBridgeClass::addModule(uint8_t index, const char* name, uint8_t type, uint8_t priority, uint8_t state) {
  modules[index].name = strdup(name);
  modules[index].type = type;
  modules[index].current = 0;
  modules[index].priority = priority;
  modules[index].state = state;
  system_status.num_modules++;

  return ESP_OK;
}

int CoreBridgeClass::removeModules() {
  if (Homekit.countAccessory() > 0)
    Homekit.deleteAccessory();

  system_status.num_modules = 0;

  return ESP_OK;
}

int CoreBridgeClass::digitalWrite(uint8_t pin, uint8_t state) {
  char* p = new char[2]{ pin, state };
  queue.push(254, 2, p);

  return ESP_OK;
}

int CoreBridgeClass::requestModulesData(uint8_t type) {
  char* p = new char[2]{ CMD_REQ_DATA, type };
  queue.push(3, 2, p);

  return ESP_OK;
}

int CoreBridgeClass::updateModulesData(uint8_t type, uint8_t* addrs, uint16_t* values, uint8_t length) {
  char* p = new char[length * 3 + 2]{ CMD_UPDATE_DATA, type };

  for (int i = 0; i < length; i++) {
    p[i * 3 + 2] = addrs[i];
    p[i * 3 + 3] = values[i] & 0xff;
    p[i * 3 + 4] = values[i] >> 8;
  }
  queue.push(3, length * 3 + 2, p);
  delete[] addrs;
  delete[] values;
  return ESP_OK;
}

int CoreBridgeClass::doModulesAction(uint8_t* addrs, uint8_t* actions, uint8_t length, bool malloc_ptr) {
  int l = 2 * length + 1;
  char* p = new char[l] {CMD_DO_MODULE};

  for (int i = 0; i < l; i++) {
    p[i * 2 + 1] = addrs[i];
    p[i * 2 + 2] = actions[i];
  }

  queue.push(3, l, p);
  if (malloc_ptr) {
    free(addrs);
    free(actions);
  } else {
    delete[] addrs;
    delete[] actions;
  }

  return ESP_OK;
}
int CoreBridgeClass::doModulesAction(uint8_t* addrs, uint8_t* actions, uint8_t length) {
  return this->doModulesAction(addrs, actions, length, false);
}

int CoreBridgeClass::setModuleSwitchState(uint8_t index, uint8_t state) {
  modules[index].state = state;

  Homekit.setServiceValue((hap_char_t*)modules[index].hc, state);
  MqttCtrl.moduleUpdate(index, "switch_state", state);

  return ESP_OK;
}

int CoreBridgeClass::setModuleCurrent(uint8_t index, uint16_t value) {
  modules[index].current = value;

  MqttCtrl.moduleUpdate(index, "current", value);

  return ESP_OK;
}

int CoreBridgeClass::setModulePrioirty(uint8_t index, uint8_t value) {
  modules[index].priority = value;

  MqttCtrl.moduleUpdate(index, "priority", value);

  return ESP_OK;
}

int CoreBridgeClass::getModuleNum() {
  return system_status.num_modules;
}

module_t* CoreBridgeClass::getModule(uint8_t index) {
  return &modules[index];
}

void CoreBridgeClass::overloadProtectionCheck() {
  static bool emerg_triggered = false;

  if (system_status.sum_current > MAX_CURRENT) {
    if (!emerg_triggered) {
      ///// POP Protection Logic /////
      if (smf_status.enable_pop) {
        int highest_priority = 0;

        for (int i = 0; i < this->getModuleNum(); i++) {
          module_t* module = this->getModule(i);
          if (module->state && module->priority >= highest_priority) {
            smf_status.overload_triggered_addr = i + 1;
            highest_priority = module->priority;
          }
        }
      }

      ///// Execute Turn Off Action /////
      printf("[SMF] Overloaded. Turning OFF: %i\n", smf_status.overload_triggered_addr);
      uint8_t* addrs = new uint8_t[1]{ (uint8_t)smf_status.overload_triggered_addr };
      uint8_t* acts = new uint8_t[1]{ DO_TURN_OFF };

      this->doModulesAction(addrs, acts, 1);
      emerg_triggered = true;

      return;
    }

    ///// Request Current per Second /////
    static unsigned long t = millis();
    if (millis() - t > 1000) {
      this->requestModulesData(MODULE_CURRENT);
      t = millis();
    }

    return;
  }

  emerg_triggered = false;
}

void moduleLiveCheck(void*) {
  unsigned long t = millis();
  bool sent = false;

  while (1) {
    if (!sent && CoreBridge.system_status.module_initialized) {
      ///// Send HI Signal /////
      char* p = new char[1]{ CMD_HI };
      queue.push(1, 1, p);

      CoreBridge.system_status.module_connected = false;

      sent = true;
      t = millis();
    }

    if (sent && millis() - t > LIVE_DETECT_INTERVAL) {
      static bool modules_removed = false;

      if (!CoreBridge.system_status.module_connected) {
        if (!modules_removed) {
          CoreBridge.digitalWrite(MODULES_STATE_PIN, 0);

          ///// Create an Empty Accessory /////
          CoreBridge.removeModules();
          Homekit.createAccessory(CoreBridge.serial_number, CoreBridge.device_name);
          Homekit.beginAccessory();

          modules_removed = true;
        }

        ///// Attempting Reconnection /////
        char* p = new char[2]{ CMD_LOAD_MODULE, 0x00 };
        queue.push(1, 2, p);
      } else {
        modules_removed = false;
      }

      sent = false;
    }

    taskYIELD();
  }
}

void recordSumCurrent(void*) {
  ///// Setup SNTP Service /////
  time_t now;
  tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);

  if (timeinfo.tm_year < (2016 - 1900)) {
    printf("Time is not set yet. Setting SNTP.\n");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "time.google.com");
    sntp_setservername(2, "time.asia.apple.com");
    sntp_init();

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
      printf("Waiting for system time to be set...\n");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    time(&now);
  }

  setenv("TZ", "CST-8", 1);
  tzset();
  localtime_r(&now, &timeinfo);

  char strftime_buf[64];
  int previousHr = timeinfo.tm_hour - 1; //TODO: get recorded hour from Warehouse

  while (1) {
    if (true) { //CoreBridge.system_status.module_initialized
      time(&now);
      localtime_r(&now, &timeinfo);

      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      printf("The current date/time in Taipei is: %s\n", strftime_buf);

      if (timeinfo.tm_hour != previousHr) {


        ///// 10 Seconds Before Recording, NINA Sends Request First /////
        printf("Taking recordSumCurrent... S1\n");
        //CoreBridge.requestModulesData(MODULE_CURRENT);
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        //int* buffer = new int[1]{ CoreBridge.system_status.sum_current };

        //Warehouse.appendData(CoreBridge.system_status.sum_current);
        printf("Taking recordSumCurrent... S2\n");
        //MqttCtrl.warehouseRequestBufferUpdate(buffer, 1);
        //delete[] buffer;
      }

      printf("Waiting %i minutes...\n", (59 - timeinfo.tm_min));
      vTaskDelay((60 - timeinfo.tm_min) * 60 * 1000 / portTICK_PERIOD_MS);
      previousHr = timeinfo.tm_hour;
    }

    //taskYIELD();
  }
}

CoreBridgeClass CoreBridge;