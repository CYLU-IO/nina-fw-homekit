#ifndef C_CONNECTOR_H 
#define C_CONNECTOR_H 

#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

#ifdef __cplusplus
extern "C" {
#endif

// /*** Module Actions ***/
#define DO_TURN_ON 0x6E  //'n'
#define DO_TURN_OFF 0x66 //'f'
 
void CoreBridge_doModulesAction(uint8_t* addrs, uint8_t *actions, uint8_t length);
int CoreBridge_getModuleAddrByHc(hap_char_t *hc);

#ifdef __cplusplus
}
#endif

#endif