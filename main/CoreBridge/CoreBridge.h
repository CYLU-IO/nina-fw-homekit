#ifndef COREBRIDGE_H
#define COREBRIDGE_H

#include <hap.h>

#define SERIAL_NUMBER_LENGTH 12
#define DEVICE_NAME_LENGTH   32
#define MAX_MODULE_NUM       20

typedef struct
{
  char *name;
  int type;
  int current;
  int priority;
  hap_serv_t *hs;
  hap_char_t *hc;
  bool state;
  bool event_triggered;
} module_t;

class CoreBridgeClass
{

public:
  char serial_number[SERIAL_NUMBER_LENGTH + 1];
  char device_name[DEVICE_NAME_LENGTH + 1];

  static module_t modules[MAX_MODULE_NUM];
  static int num_modules;

  CoreBridgeClass();

  void init();

  int setDeviceName(const char *name);

  /*** Homekit ***/
  int createAccessory();
  int countAccessory();
  int deleteAccessory();
  int beginHomekit();

  int addModule(uint8_t index, const char *name, uint8_t type, uint8_t priority, uint8_t state);

  int setModuleSwitchState(uint8_t index, uint8_t state, bool trigger);
  int setModuleSwitchState(uint8_t index, uint8_t state);
  int getModuleSwitchState(uint8_t index);

  int setModuleCurrent(uint8_t index, uint16_t value);

  int setModulePrioirty(uint8_t index, uint8_t value);
  int getModulePrioirty(uint8_t index);

  int readModuleTriggered(uint8_t index);

  int getModuleNum();
  module_t *getModule(uint8_t index);
};

extern CoreBridgeClass CoreBridge;

#endif