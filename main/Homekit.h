#ifndef HOMEKIT_H
#define HOMEKIT_H

#include <Arduino.h>
#include <hap.h>

#define MAX_MODULE_NUM 20

#define HAP_PINCODE_LENGTH 10
#define HAP_SETUPID_LENGTH 4
#define HK_ACC_SERIAL_MAX_LENGTH 12
#define HK_ACC_ID_MAX_LENGTH 32
#define HK_ACC_NAME_MAX_LENGTH 32
#define HK_SERVICE_NAME_MAX_LENGTH 25

typedef struct {
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
  char _acc_id[HK_ACC_ID_MAX_LENGTH + 1];
  char _acc_name[HK_ACC_NAME_MAX_LENGTH + 1];
  char _acc_setupCode[HAP_PINCODE_LENGTH + 1];
  char _acc_setupId[HAP_SETUPID_LENGTH + 1];

  int num_modules;

public:
  static hap_acc_t *_accessory;
  static module_t modules[MAX_MODULE_NUM];

  HomekitClass();

  int init(const char *serial, const char *name, const char *setupCode, const char *setupId);

  int create();

  int addService(uint8_t index, uint8_t id, uint8_t state, const char *name);

  int begin();

  int getServiceValue(uint8_t index, uint8_t id);

  int setServiceValue(uint8_t index, uint8_t id, uint8_t state);

  int readTriggered(uint8_t index, uint8_t id);

  static int switchWrite(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv);

  /*void cleanModules();

    void destroy();

    static void* switchRead(void* arg);

    static void switchWrite(void* arg, void* value, int len);

    static void switchNotify(void* arg, void* ev_handle, bool enable);

    char *getFwVersion();*/
};

extern HomekitClass Homekit;

#endif