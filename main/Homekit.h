#ifndef HOMEKIT_H
#define HOMEKIT_H

#include <Arduino.h>
#include <hap.h>

#define MAX_MODULE_NUM 20

#define HK_ACC_SERIAL_MAX_LENGTH 12
#define HK_ACC_NAME_MAX_LENGTH 32

typedef struct
{
  uint8_t id;
  char *name;
  hap_serv_t *hs;
  hap_char_t *hc;
  bool event_triggered;
} module_t;

class HomekitClass
{
private:
  char _acc_serial[HK_ACC_SERIAL_MAX_LENGTH + 1];
  char _acc_name[HK_ACC_NAME_MAX_LENGTH + 1];
  int num_modules;

public:
  static hap_acc_t *_accessory;
  static module_t modules[MAX_MODULE_NUM];

  HomekitClass();

  int init();

  int create(const char *serial, const char *name);

  int addService(uint8_t index, uint8_t id, uint8_t state, const char *name);

  int begin();

  int getServiceValue(uint8_t index, uint8_t id);

  int setServiceValue(uint8_t index, uint8_t id, uint8_t state);

  int readTriggered(uint8_t index, uint8_t id);

  static int switchWrite(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv);

  void deleateAccessory();

  int resetToFactory();
};

extern HomekitClass Homekit;

#endif