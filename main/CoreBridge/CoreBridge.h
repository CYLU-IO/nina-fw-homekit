#ifndef COREBRIDGE_H
#define COREBRIDGE_H

#include <Arduino.h>
#include <hap.h>

#include <definitions.h>
#include <Homekit.h>
#include <MqttCtrl.h>
#include <WifiManager.h>
#include <Warehouse.h>
#include <Perioder.h>
#include <CommandHandler.h>

typedef struct {
  char* name;
  uint8_t type;
  uint8_t priority;
  uint16_t current;
  bool state;
  hap_serv_t* hs;
  hap_char_t* hc;
} module_t;

typedef struct {
  uint16_t mcub;
  uint8_t overload_triggered_addr;
  uint8_t enable_pop;
} smart_modularized_fuse_status_t;

typedef struct {
  uint8_t num_modules;
  uint16_t sum_current;
  bool module_initialized;
  bool module_connected;
  bool reset2factorying;
} system_status_t;

class CoreBridgeClass {
private:
  module_t modules[MAX_MODULE_NUM];

public:
  char serial_number[SERIAL_NUMBER_LENGTH + 1];
  char device_name[DEVICE_NAME_LENGTH + 1];
  uint32_t running_time;

  static system_status_t system_status;
  static smart_modularized_fuse_status_t smf_status;

  CoreBridgeClass();

  void init();

  int setDeviceName(const char* name);
  int setEnablePOP(uint8_t state);
  int setRunningTime(uint32_t hrs);
  int setMQTTHost(const char* host);

  int addModule(uint8_t index, const char* name, uint8_t type, uint8_t priority, uint8_t state);
  int removeModules();

  int digitalWrite(uint8_t pin, uint8_t state);
  int reset2Factory();
  void restart();

  int requestModulesData(uint8_t type);
  int updateModulesData(uint8_t type, uint8_t* addrs, uint16_t* values, uint8_t length);
  int doModulesAction(uint8_t* addrs, uint8_t* actions, uint8_t length, bool malloc_ptr);
  int doModulesAction(uint8_t* addrs, uint8_t* actions, uint8_t length);

  int setModuleSwitchState(uint8_t index, uint8_t state);
  int setModuleCurrent(uint8_t index, uint16_t value);
  int setModulePrioirty(uint8_t index, uint8_t value);

  int getModuleNum();
  module_t* getModule(uint8_t index);

  void overloadProtectionCheck();
};

extern CoreBridgeClass CoreBridge;

#endif