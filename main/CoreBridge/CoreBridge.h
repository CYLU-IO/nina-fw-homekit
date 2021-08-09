#ifndef COREBRIDGE_H
#define COREBRIDGE_H

#include <Arduino.h>
#include <hap.h>

#include <Homekit.h>
#include <MqttCtrl.h>
#include <WifiManager.h>
#include <Warehouse.h>
#include <CommandHandler.h>

///// MAX NUMBER /////
#define SERIAL_NUMBER_LENGTH 12
#define DEVICE_NAME_LENGTH 32
#define MAX_MODULE_NUM 20
#define MAX_CURRENT 1500

///// INTERVALS /////
#define LIVE_DETECT_INTERVAL 1000
#define RECORD_SUM_CURRENT_INTERVAL 300000 //5 minutes

///// PIN SETS /////
#define WIFI_STATE_PIN          7
#define MODULES_STATE_PIN       9

typedef struct
{
  char *name;
  uint8_t type;
  uint8_t priority;
  uint16_t current;
  bool state;
  hap_serv_t *hs;
  hap_char_t *hc;
} module_t;

typedef struct
{
  uint16_t mcub;
  uint8_t overload_triggered_addr;
  uint8_t enable_pop;
} smart_modularized_fuse_status_t;

typedef struct
{
  uint8_t num_modules;
  uint16_t sum_current;
  bool module_initialized;
  bool module_connected;
} system_status_t;

class CoreBridgeClass
{
private:
  module_t modules[MAX_MODULE_NUM];

public:
  char serial_number[SERIAL_NUMBER_LENGTH + 1];
  char device_name[DEVICE_NAME_LENGTH + 1];

  static system_status_t system_status;
  static smart_modularized_fuse_status_t smf_status;

  CoreBridgeClass();

  void init();

  int setDeviceName(const char *name);
  int setEnablePOP(uint8_t state);

  int addModule(uint8_t index, const char *name, uint8_t type, uint8_t priority, uint8_t state);
  int removeModules();

  int digitalWrite(uint8_t pin, uint8_t state);

  int requestModulesData(uint8_t type);
  int updateModulesData(uint8_t type, uint8_t *addrs, uint16_t* values, uint8_t length);
  int doModulesAction(uint8_t *addrs, uint8_t *actions, uint8_t length, bool malloc_ptr);
  int doModulesAction(uint8_t* addrs, uint8_t *actions, uint8_t length);

  int setModuleSwitchState(uint8_t index, uint8_t state);
  int setModuleCurrent(uint8_t index, uint16_t value);
  int setModulePrioirty(uint8_t index, uint8_t value);

  int getModuleNum();
  module_t *getModule(uint8_t index);

  void overloadProtectionCheck();
  void moduleLiveCheck();
  void recordSumCurrent();
};

extern CoreBridgeClass CoreBridge;

#endif