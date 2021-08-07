#ifndef COREBRIDGE_H
#define COREBRIDGE_H

#include <hap.h>

#include "Homekit.h"
#include "MqttCtrl.h"
#include "WifiManager.h"
#include "Warehouse.h"

#define SERIAL_NUMBER_LENGTH 12
#define DEVICE_NAME_LENGTH 32
#define MAX_MODULE_NUM 20

typedef struct
{
  char *name;
  int type;
  int current;
  int priority;
  hap_serv_t *hs;
  hap_char_t *hc;
  bool state;
} module_t;

typedef struct
{
  int mcub;
  int overload_triggered_addr;
  bool emerg_triggered;
} smart_modularized_fuse_status_t;

typedef struct
{
  int num_modules;
  int sum_current;
  bool module_initialized;
  bool module_connected;
} system_status_t;

class CoreBridgeClass
{

public:
  char serial_number[SERIAL_NUMBER_LENGTH + 1];
  char device_name[DEVICE_NAME_LENGTH + 1];

  uint8_t enable_pop;

  static module_t modules[MAX_MODULE_NUM];
  static system_status_t system_status;
  static smart_modularized_fuse_status_t smf_status;

  CoreBridgeClass();

  void init();

  int setDeviceName(const char *name);
  int setEnablePOP(uint8_t state);

  ///// Homekit /////
  int createAccessory();
  int countAccessory();
  int deleteAccessory();
  int beginHomekit();
  ///////////////////

  int addModule(uint8_t index, const char *name, uint8_t type, uint8_t priority, uint8_t state);

  int setModuleSwitchState(uint8_t index, uint8_t state, bool trigger);
  int setModuleSwitchState(uint8_t index, uint8_t state);
  int setModuleCurrent(uint8_t index, uint16_t value);

  int setModulePrioirty(uint8_t index, uint8_t value);

  int getModuleNum();
  module_t *getModule(uint8_t index);
};

extern CoreBridgeClass CoreBridge;

#endif