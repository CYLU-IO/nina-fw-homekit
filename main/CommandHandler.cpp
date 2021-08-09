#include <lwip/sockets.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "cJSON.h"

#include <CoreBridge.h>

uartMsgQueue queue;

int uartReceive(const uint8_t command[], uint8_t response[]) {
  uint8_t port = command[4];
  uint16_t length = ((command[5] << 8) & 0xff00) | (command[6] & 0xff);

  char* payload = (char*)calloc(length, sizeof(char));
  memcpy(payload, &command[8], length - 1);

  ///// SAMD21 Event /////
  if (port == 255) {
    switch (command[7]) {
      case 0x00:
      {
        char* p = new char[2]{ CMD_LOAD_MODULE, 0x00 };
        queue.push(1, 2, p);
        break;
      }

      case 0x01: //Reset to Factory
      {
        CoreBridge.digitalWrite(WIFI_STATE_PIN, 0);
        CoreBridge.digitalWrite(MODULES_STATE_PIN, 0);

        nvs_flash_erase();
        esp_restart();
        break;
      }
    }

    goto end;
  }

  ///// UART Event /////
  switch (command[7]) {
    case CMD_REQ_ADR:
    {
      char* p = new char[2]{ CMD_LOAD_MODULE, 0x00 };
      queue.push(1, 2, p);
      break;
    }

    case CMD_LINK_MODULE:
    {
      cJSON* data = cJSON_Parse(payload);

      if (data == NULL) {
        printf("Json parsing failed.\n");
        char* p = new char[2]{ CMD_LOAD_MODULE, 0x00 };
        queue.push(1, 2, p);
        break;
      }

      int totalModules = cJSON_GetObjectItemCaseSensitive(data, "total")->valuedouble;
      int index = cJSON_GetObjectItemCaseSensitive(data, "addr")->valuedouble - 1;

      CoreBridge.digitalWrite(MODULES_STATE_PIN, 0);

      if (index + 1 == totalModules) //first module arrives
      {
        CoreBridge.system_status.module_initialized = false;
        CoreBridge.removeModules();
        Homekit.createAccessory(CoreBridge.serial_number, CoreBridge.device_name);
      }

      CoreBridge.addModule(index,
                           cJSON_GetObjectItemCaseSensitive(data, "name")->valuestring,
                           0, //type
                           cJSON_GetObjectItemCaseSensitive(data, "pri")->valuedouble,
                           cJSON_GetObjectItemCaseSensitive(data, "switch_state")->valuedouble);

      if (index == 0) {
        printf("[UART] Total modules: %i\n", totalModules);
        ///// Initialize connected modules /////
        char* p = new char[totalModules + 1]{ CMD_INIT_MODULE };
        for (int i = 1; i <= totalModules; i++)
          p[i] = i;
        queue.push(3, totalModules + 1, p);

        ///// Initially Update Modules Current /////
        CoreBridge.requestModulesData(MODULE_CURRENT);

        ///// Start Online Service /////
        Homekit.beginAccessory();
        MqttCtrl.modulesUpdate();

        CoreBridge.digitalWrite(MODULES_STATE_PIN, 1);
        CoreBridge.system_status.module_initialized = true;
        CoreBridge.system_status.module_connected = true;
      }
      break;
    }

    case CMD_HI:
    {
      CoreBridge.system_status.module_connected = true;
      break;
    }

    case CMD_UPDATE_DATA:
    {
      int addr = payload[0];
      int value = payload[2] & 0xff | payload[3] << 8;

      switch (payload[1]) {
        case MODULE_SWITCH_STATE:
        {
          //printf("[UART] Module %i state changes to %i\n", addr, value);
          CoreBridge.setModuleSwitchState(addr - 1, value);
          break;
        }

        case MODULE_CURRENT:
        {
          //printf("[UART] Module %i current updates to %i\n", addr, value);
          ///// Check MCUB Triggering /////
          if (value >= CoreBridge.getModule(addr - 1)->current + CoreBridge.smf_status.mcub) {
            //printf("[SMF] MCUB Triggered by module %i\n", addr);
            CoreBridge.smf_status.overload_triggered_addr = addr;
            CoreBridge.overloadProtectionCheck();
          }

          ///// Update module current data /////
          CoreBridge.setModuleCurrent(addr - 1, value);

          ///// Update System Sum Current /////
          int sum = 0;
          for (int i = 0; i < CoreBridge.system_status.num_modules; i++)
            sum += CoreBridge.getModule(i)->current;
          CoreBridge.system_status.sum_current = sum;

          ///// Update MCUB /////
          int mcub = (MAX_CURRENT - sum) / CoreBridge.system_status.num_modules;
          mcub = (mcub >= 0) ? mcub : 0;
          CoreBridge.smf_status.mcub = mcub;

          ////// Broadcast to Modules /////
          uint8_t* addrs = new uint8_t[1]{ 0 };
          uint16_t* values = new uint16_t[1]{ (uint16_t)mcub };

          CoreBridge.updateModulesData(MODULE_MCUB, addrs, values, 1);
          //printf("[SMF] Update MCUB: %i\n", mcub);
          break;
        }

        /*case MODULE_PRIORITY: {
              #if DEBUG
                Serial.print("[UART] Module ");
                Serial.print(addr);
                Serial.print(" priority changes to ");
                Serial.println(value);
              #endif
                sys_status.modules[addr - 1][0] = value;
                break;
              }*/
      }
      break;
    }
  }

end:
  free(payload);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = ESP_OK;

  return 6;
}

int uartTransmit(const uint8_t command[], uint8_t response[]) {
  uart_msg_pack* pack = (!queue.isEmpty()) ? queue.pop() : new uart_msg_pack(255, 0, NULL);

  int length = pack->length;

  response[2] = 2;             // number of parameters
  response[3] = 0x00;          // parameter 1 length(H)
  response[4] = 0x01;          // parameter 1 length(L)
  response[5] = pack->port;    //port
  response[6] = length >> 8;   // parameter 2 length(H)
  response[7] = length & 0xff; // parameter 2 length(L)

  for (int i = 0; i < length; i++)
    response[8 + i] = pack->payload[i];

  delete[] pack->payload;
  delete pack;

  return 9 + length;
}

///// UART MSG Queue /////
void uartMsgQueue::push(int po, int l, char* p) {
  if (isEmpty()) {
    front = new uart_msg_pack(po, l, p);
    back = front;
    size++;

    return;
  }

  uart_msg_pack* newPack = new uart_msg_pack(po, l, p);
  back->next = newPack;
  back = newPack; // update back pointer
  size++;
}

uart_msg_pack* uartMsgQueue::pop() {
  if (isEmpty())
    return NULL;

  uart_msg_pack* pack = front;
  front = front->next; // update front pointer
  size--;

  return pack;
}

bool uartMsgQueue::isEmpty() {
  return ((front && back) == 0);
}

int uartMsgQueue::getSize() {
  return size;
}

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

///// SPI Command Handler /////
#define NUM_COMMAND_HANDLERS (sizeof(commandHandlers) / sizeof(commandHandlers[0]))

CommandHandlerClass::CommandHandlerClass() {}

static const int GPIO_IRQ = 0;

void CommandHandlerClass::begin() {
  pinMode(GPIO_IRQ, OUTPUT);

  _updateGpio0PinSemaphore = xSemaphoreCreateCounting(2, 0);

  xTaskCreatePinnedToCore(CommandHandlerClass::gpio0Updater, "gpio0Updater", 4096, NULL, 1, NULL, 1);
}

#define UDIV_UP(a, b) (((a) + (b)-1) / (b))
#define ALIGN_UP(a, b) (UDIV_UP(a, b) * (b))

int CommandHandlerClass::handle(const uint8_t command[], uint8_t response[]) {
  int responseLength = 0;

  if (command[0] == 0xe0 && command[1] < NUM_COMMAND_HANDLERS) {
    CommandHandlerType commandHandlerType = commandHandlers[command[1]];

    if (commandHandlerType)
      responseLength = commandHandlerType(command, response);
  }

  if (responseLength == 0) {
    response[0] = 0xef;
    response[1] = 0x00;
    response[2] = 0xee;

    responseLength = 3;
  } else {
    response[0] = 0xe0;
    response[1] = (0x80 | command[1]);
    response[responseLength - 1] = 0xee;
  }

  xSemaphoreGive(_updateGpio0PinSemaphore);

  return ALIGN_UP(responseLength, 4);
}

void CommandHandlerClass::gpio0Updater(void*) {
  while (1)
    CommandHandler.updateGpio0Pin();
}

void CommandHandlerClass::updateGpio0Pin() {
  xSemaphoreTake(_updateGpio0PinSemaphore, portMAX_DELAY);
  digitalWrite(GPIO_IRQ, HIGH);
  vTaskDelay(1);
}

CommandHandlerClass CommandHandler;
