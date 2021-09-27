#include <esp32/rom/uart.h>

extern "C"
{
#include <driver/periph_ctrl.h>
#include <driver/uart.h>

#include "esp_log.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
}

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <SPIS.h>

#include <CommandHandler.h>
#include "CoreBridge/CoreBridge.h"

#define SPI_BUFFER_LEN SPI_MAX_DMA_LEN

uint8_t* commandBuffer;
uint8_t* responseBuffer;

void setup() {
  ///// SPI Initialization /////
  SPIS.begin();
  commandBuffer = (uint8_t*)heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);
  responseBuffer = (uint8_t*)heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);
  CommandHandler.begin();

  ///// Service Initialization /////
  CoreBridge.init();
  CoreBridge.digitalWrite(WIFI_STATE_PIN, 0);
  CoreBridge.digitalWrite(MODULES_STATE_PIN, 0);

  ///// NINA Periodic Task /////
  xTaskCreate(productLifetimeCounter, "custom_plc", 2048, NULL, 1, NULL);

  ///// TEST //////
  /*WifiMgr.begin();
  CoreBridge.removeModules();
  Homekit.createAccessory(CoreBridge.serial_number, CoreBridge.device_name);
  CoreBridge.addModule(3, "Switch 4", 0, 1, 0);
  CoreBridge.addModule(2, "Switch 3", 0, 1, 0);
  CoreBridge.addModule(1, "Switch 2", 0, 1, 0);
  CoreBridge.addModule(0, "Switch 1", 0, 1, 0);
  Homekit.beginAccessory();
  MqttCtrl.modulesUpdate();*/
  //CoreBridge.setModuleValue(0, 1);
  ///// TEST END /////
}

void loop() {
  ///// SPI Handling /////
  memset(commandBuffer, 0x00, SPI_BUFFER_LEN);
  int commandLength = SPIS.transfer(NULL, commandBuffer, SPI_BUFFER_LEN);

  if (commandLength == 0)
    return;

  memset(responseBuffer, 0x00, SPI_BUFFER_LEN);
  int responseLength = CommandHandler.handle(commandBuffer, responseBuffer);

  SPIS.transfer(responseBuffer, NULL, responseLength);
}