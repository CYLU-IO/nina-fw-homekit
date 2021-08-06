#ifndef HOMEKIT_H
#define HOMEKIT_H

#include <Arduino.h>
#include <hap.h>

class HomekitClass
{

public:
  static hap_acc_t *_accessory;

  HomekitClass();

  int init();

  int createAccessory(const char *serial, const char *name);

  int countAccessory();

  int beginAccessory();

  int deleteAccessory();

  int setServiceValue(hap_char_t *hc, uint8_t state);

  static int switchWrite(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv);
};

extern HomekitClass Homekit;

#endif