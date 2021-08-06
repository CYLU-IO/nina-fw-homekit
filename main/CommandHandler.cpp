#include <lwip/sockets.h>
#include "esp_log.h"

#include <Arduino.h>
#include "CommandHandler.h"
#include "CoreBridge/CoreBridge.h"

int uartReceive(const uint8_t command[], uint8_t response[])
{
  uint16_t length = ((command[3] << 8) & 0xff00) | (command[4] & 0xff);

  printf("LENGTH: %i\n", length);

  char *payload = (char *)calloc(length + 1, sizeof(char));
  memcpy(payload, &command[5], length);

  printf("DATA: %s\n", payload);

  free(payload);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = ESP_OK;

  return 6;
}

int uartTransmit(const uint8_t command[], uint8_t response[])
{
  response[2] = 2; // number of parameters
  response[3] = 0x00; // parameter 1 length(H)
  response[4] = 0x01; // parameter 1 length(L)
  response[5] = 1; //port
  response[6] = 0x00; // parameter 2 length(H)
  response[7] = 0x01; // parameter 2 length(L)
  response[8] = 0x51;
  
  return 10;
}
/*
int corebridge_getFreeHeap(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = esp_get_free_heap_size();

  return 6;
}

int corebridge_getEnablePOP(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.enable_pop;

  return 6;
}

int wifimgr_getStatus(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = WifiMgr.getStatus();

  return 6;
}

int resetNetwork(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  WifiMgr.resetNetwork();

  return 6;
}

int resetToFactory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  nvs_flash_erase();
  esp_restart();

  return 6;
}

int createAccessory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.createAccessory();

  return 6;
}

int countAccessory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.countAccessory();

  return 6;
}

int beginAccessory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.beginHomekit();

  MqttCtrl.modulesUpdate();

  return 6;
}

int addModule(const uint8_t command[], uint8_t response[])
{
  char name[25 + 1];

  uint8_t index = command[4];
  uint8_t type = command[6];
  uint8_t priority = command[8];
  uint8_t state = command[10];


  memset(name, 0x00, sizeof(name));
  memcpy(name, &command[12], command[11]);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.addModule(index, name, type, priority, state);

  return 6;
}

int setModuleSwitchState(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];
  uint8_t state = command[6];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.setModuleSwitchState(index, state, false);

  return 6;
}

int getModuleSwitchState(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.getModuleSwitchState(index);

  return 6;
}

int setModuleCurrent(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];
  uint16_t value = (command[7] & 0xff | (command[8] << 8) & 0xff00);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.setModuleCurrent(index, value);

  return 6;
}

int getModulePrioirty(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.getModulePrioirty(index);

  return 6;
}

int readModuleTriggered(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.readModuleTriggered(index);

  return 6;
}

int pushWarehouseBuffer(const uint8_t command[], uint8_t response[])
{
  uint16_t length = ((command[3] << 8) & 0xff00) | (command[4] & 0xff);

  int *warehouseBuffer = (int *)malloc(length / 2 * sizeof(int));

  for (int i = 0; i < length / 2; i++)
  {
    warehouseBuffer[i] = (command[5 + (i * 2)] & 0xff | (command[6 +(i * 2)] << 8) & 0xff00);
  }

  MqttCtrl.warehouseRequestBufferUpdate(warehouseBuffer, length / 2);
  free(warehouseBuffer);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  return 6;
}*/

typedef int (*CommandHandlerType)(const uint8_t command[], uint8_t response[]);

const CommandHandlerType commandHandlers[] = {
    // 0x00 -> 0x0f
    uartReceive,
    uartTransmit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

#define NUM_COMMAND_HANDLERS (sizeof(commandHandlers) / sizeof(commandHandlers[0]))

CommandHandlerClass::CommandHandlerClass()
{
}

static const int GPIO_IRQ = 0;

void CommandHandlerClass::begin()
{
  pinMode(GPIO_IRQ, OUTPUT);

  _updateGpio0PinSemaphore = xSemaphoreCreateCounting(2, 0);

  xTaskCreatePinnedToCore(CommandHandlerClass::gpio0Updater, "gpio0Updater", 4096, NULL, 1, NULL, 1);
}

#define UDIV_UP(a, b) (((a) + (b)-1) / (b))
#define ALIGN_UP(a, b) (UDIV_UP(a, b) * (b))

int CommandHandlerClass::handle(const uint8_t command[], uint8_t response[])
{
  int responseLength = 0;

  if (command[0] == 0xe0 && command[1] < NUM_COMMAND_HANDLERS)
  {
    CommandHandlerType commandHandlerType = commandHandlers[command[1]];

    if (commandHandlerType)
    {
      responseLength = commandHandlerType(command, response);
    }
  }

  if (responseLength == 0)
  {
    response[0] = 0xef;
    response[1] = 0x00;
    response[2] = 0xee;

    responseLength = 3;
  }
  else
  {
    response[0] = 0xe0;
    response[1] = (0x80 | command[1]);
    response[responseLength - 1] = 0xee;
  }

  xSemaphoreGive(_updateGpio0PinSemaphore);

  return ALIGN_UP(responseLength, 4);
}

void CommandHandlerClass::gpio0Updater(void *)
{
  while (1)
  {
    CommandHandler.updateGpio0Pin();
  }
}

void CommandHandlerClass::updateGpio0Pin()
{
  xSemaphoreTake(_updateGpio0PinSemaphore, portMAX_DELAY);
  digitalWrite(GPIO_IRQ, HIGH);
  vTaskDelay(1);
}

CommandHandlerClass CommandHandler;
