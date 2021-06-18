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
#include "Homekit.h"
#include "Smartconfig.h"

#define SPI_BUFFER_LEN SPI_MAX_DMA_LEN

uint8_t *commandBuffer;
uint8_t *responseBuffer;

void setupSPI();
void setupWiFi();

void setup()
{
  pinMode(15, INPUT);
  pinMode(21, INPUT);

#if defined(NANO_RP2040_CONNECT)
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  digitalWrite(26, HIGH);
  digitalWrite(27, HIGH);
#endif

  setupSPI();

  Homekit.init();
  
  smartconfigStart();
}

void setupWiFi()
{
  esp_vfs_spiffs_conf_t conf = {
      .base_path = "/fs",
      .partition_label = "storage",
      .max_files = 20,
      .format_if_mount_failed = true};

  esp_err_t ret = esp_vfs_spiffs_register(&conf);
}

void setupSPI()
{
  SPIS.begin();

  commandBuffer = (uint8_t *)heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);
  responseBuffer = (uint8_t *)heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);

  CommandHandler.begin();
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