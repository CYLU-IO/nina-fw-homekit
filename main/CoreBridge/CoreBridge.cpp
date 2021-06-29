#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>

#include "CoreBridge.h"
#include "DB.h"
#include "Homekit.h"
#include "MqttCtrl.h"
#include "WifiManager.h"

CoreBridgeClass::CoreBridgeClass()
{
}

void CoreBridgeClass::init()
{
  Homekit.init();
  WifiMgr.begin();
}

int CoreBridgeClass::addModule()
{

}

int CoreBridgeClass::setModuleValue()
{
  
}

int CoreBridgeClass::getModuleValue()
{
  
}

int CoreBridgeClass::readModuleTriggered()
{
  
}

CoreBridgeClass CoreBridge;