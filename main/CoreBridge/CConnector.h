#ifndef C_CONNECTOR_H 
#define C_CONNECTOR_H 

#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

  void CoreBridge_doModulesAction(uint8_t* addrs, uint8_t* actions, uint8_t length);
  uint8_t CoreBridge_getModuleAddrByHc(hap_char_t* hc);

#ifdef __cplusplus
}
#endif

#endif