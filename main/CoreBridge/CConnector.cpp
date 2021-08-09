#include <CoreBridge.h>
#include <CConnector.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void CoreBridge_doModulesAction(uint8_t* addrs, uint8_t* actions, uint8_t length)   {
    CoreBridge.doModulesAction(addrs, actions, length, true);
  }

  uint8_t CoreBridge_getModuleAddrByHc(hap_char_t* hc)   {
    for (int i = 0; i < CoreBridge.getModuleNum(); i++)     {
      if (hap_char_get_iid((hap_char_t*)CoreBridge.getModule(i)->hc) == hap_char_get_iid(hc))
        return i + 1;
    }

    return 0;
  }

#ifdef __cplusplus
}
#endif