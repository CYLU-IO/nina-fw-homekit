#include <esp32/rom/uart.h>

extern "C"
{
#include <driver/periph_ctrl.h>

#include <driver/uart.h>

#include "esp_spiffs.h"
#include "esp_log.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
}

#include <Arduino.h>
#include <SPIS.h>

#include "CommandHandler.h"
#include "CoreBridge/CoreBridge.h"

#define SPI_BUFFER_LEN SPI_MAX_DMA_LEN

uint8_t *commandBuffer;
uint8_t *responseBuffer;

void setupSpiffs();

void setup()
{
  SPIS.begin();
  commandBuffer = (uint8_t *)heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);
  responseBuffer = (uint8_t *)heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);
  CommandHandler.begin();

  CoreBridge.init();

  /*CoreBridge.createAccessory();
  CoreBridge.addModule(1, 1, "Switch 2");
  CoreBridge.addModule(0, 0, "Switch 1");
  CoreBridge.beginHomekit();*/
  //CoreBridge.setModuleValue(0, 1);
}

void setupSpiffs()
{
  esp_vfs_spiffs_conf_t conf = {
      .base_path = "/fs",
      .partition_label = "storage",
      .max_files = 20,
      .format_if_mount_failed = true};

  esp_err_t ret = esp_vfs_spiffs_register(&conf);
}

void loop()
{
  // wait for a command
  memset(commandBuffer, 0x00, SPI_BUFFER_LEN);
  int commandLength = SPIS.transfer(NULL, commandBuffer, SPI_BUFFER_LEN);

  if (commandLength == 0)
    return;

  // process
  memset(responseBuffer, 0x00, SPI_BUFFER_LEN);
  int responseLength = CommandHandler.handle(commandBuffer, responseBuffer);

  SPIS.transfer(responseBuffer, NULL, responseLength);
}