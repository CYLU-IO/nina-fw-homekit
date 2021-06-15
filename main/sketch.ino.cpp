/*
  This file is part of the Arduino NINA firmware.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <rom/uart.h>

extern "C"
{
#include <driver/periph_ctrl.h>

#include <driver/uart.h>
#include <esp_bt.h>

#include "esp_spiffs.h"
#include "esp_log.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "esp_partition.h"
}

#include <Arduino.h>

#include <SPIS.h>

#include "CommandHandler.h"

//For Wifi Provision
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;

static void wifi_init_sta()
{
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();
}
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    printf("Wifi Connected");
    printf("IP: %s \n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
  {
    esp_wifi_connect();
    xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    break;
  }
  default:
    break;
  }
  return ESP_OK;
}
/* Event handler for catching provisioning manager events */
static void prov_event_handler(void *user_data,
                               wifi_prov_cb_event_t event, void *event_data)
{
  switch (event)
  {
  case WIFI_PROV_START:
    printf("Provisioning started\n");
    break;
  case WIFI_PROV_CRED_RECV:
  {
    wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
    /* If SSID length is exactly 32 bytes, null termination
             * will not be present, so explicitly obtain the length */
    size_t ssid_len = strnlen((const char *)wifi_sta_cfg->ssid, sizeof(wifi_sta_cfg->ssid));
    printf("Received Wi-Fi credentials"
           "\n\tSSID     : %.*s\n\tPassword : %s",
           ssid_len, (const char *)wifi_sta_cfg->ssid,
           (const char *)wifi_sta_cfg->password);
    break;
  }
  case WIFI_PROV_CRED_FAIL:
  {
    wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
    printf("Provisioning failed!\n\tReason : %s"
           "\n\tPlease reset to factory and retry provisioning",
           (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi AP password incorrect" : "Wi-Fi AP not found");
    break;
  }
  case WIFI_PROV_CRED_SUCCESS:
    printf("Provisioning successful\n");
    break;
  case WIFI_PROV_END:
    /* De-initialize manager once provisioning is finished */
    wifi_prov_mgr_deinit();
    break;
  default:
    break;
  }
}

#define SPI_BUFFER_LEN SPI_MAX_DMA_LEN

int debug = 1;

uint8_t *commandBuffer;
uint8_t *responseBuffer;

void setDebug(int d)
{
  debug = d;

  if (debug)
  {
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[1], 0);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[3], 0);

    const char *default_uart_dev = "/dev/uart/0";
    _GLOBAL_REENT->_stdin = fopen(default_uart_dev, "r");
    _GLOBAL_REENT->_stdout = fopen(default_uart_dev, "w");
    _GLOBAL_REENT->_stderr = fopen(default_uart_dev, "w");

    uart_div_modify(CONFIG_CONSOLE_UART_NUM, (APB_CLK_FREQ << 4) / 115200);

    // uartAttach();
    ets_install_uart_printf();
    uart_tx_switch(CONFIG_CONSOLE_UART_NUM);
  }
  else
  {
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[1], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[3], PIN_FUNC_GPIO);

    _GLOBAL_REENT->_stdin = (FILE *)&__sf_fake_stdin;
    _GLOBAL_REENT->_stdout = (FILE *)&__sf_fake_stdout;
    _GLOBAL_REENT->_stderr = (FILE *)&__sf_fake_stderr;

    ets_install_putc1(NULL);
    ets_install_putc2(NULL);
  }
}

void setupSPI();
void setupProvision();
void setupWiFi();
void setupBluetooth();

void setup()
{
  setDebug(debug);

  // put SWD and SWCLK pins connected to SAMD as inputs
  pinMode(15, INPUT);
  pinMode(21, INPUT);

#if defined(NANO_RP2040_CONNECT)
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  digitalWrite(26, HIGH);
  digitalWrite(27, HIGH);
#endif

  setupSPI();
  //setupProvision();
  ///setupWiFi();

  /*pinMode(5, INPUT);
  if (digitalRead(5) == LOW) {
    setupBluetooth();
  } else {
    setupWiFi();
  }*/
}

void setupProvision()
{
  printf("Provision program begins\n");

  /* Initialize TCP/IP and the event loop */
  tcpip_adapter_init();
  esp_event_loop_init(event_handler, NULL);
  wifi_event_group = xEventGroupCreate();

  /* Initialize Wi-Fi */
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  /* Configuration for the provisioning manager */
  wifi_prov_mgr_config_t config = {
      /* What is the Provisioning Scheme that we want ?
         * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
      .scheme = wifi_prov_scheme_ble,

      /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * WIFI_PROV_EVENT_HANDLER_NONE when using wifi_prov_scheme_softap*/
      .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,

      /* Do we want an application specific handler be executed on
         * various provisioning related events */
      .app_event_handler = {
          .event_cb = prov_event_handler,
          .user_data = NULL}};

  /* Initialize provisioning manager with the
     * configuration parameters set above */
  wifi_prov_mgr_init(config);
}

void setupWiFi()
{
  esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

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

/*void setupBluetooth()
{
  periph_module_enable(PERIPH_UART1_MODULE);
  periph_module_enable(PERIPH_UHCI0_MODULE);

#if defined(UNO_WIFI_REV2)
  uart_set_pin(UART_NUM_1, 1, 3, 33, 0); // TX, RX, RTS, CTS
#elif defined(NANO_RP2040_CONNECT)
  uart_set_pin(UART_NUM_1, 1, 3, 33, 12); // TX, RX, RTS, CTS
#else
  uart_set_pin(UART_NUM_1, 23, 12, 18, 5);
#endif
  uart_set_hw_flow_ctrl(UART_NUM_1, UART_HW_FLOWCTRL_CTS_RTS, 5);

  esp_bt_controller_config_t btControllerConfig = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  btControllerConfig.hci_uart_no = UART_NUM_1;
#if defined(UNO_WIFI_REV2) || defined(NANO_RP2040_CONNECT)
  btControllerConfig.hci_uart_baudrate = 115200;
#else
  btControllerConfig.hci_uart_baudrate = 912600;
#endif

  esp_bt_controller_init(&btControllerConfig);
  while (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE)
    ;
  esp_bt_controller_enable(ESP_BT_MODE_BLE);
  esp_bt_sleep_enable();

  vTaskSuspend(NULL);

  while (1)
  {
    vTaskDelay(portMAX_DELAY);
  }
}*/