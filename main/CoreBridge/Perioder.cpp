#include <stdio.h>
#include "cJSON.h"

#include <hap_platform_keystore.h>

#include <CoreBridge.h>

static uint16_t avg_system_current = 0;
static uint8_t avg_sys_current_count = 0;

void moduleLiveCheck(void*) {
  bool sent = false;

  while (1) {
    if (CoreBridge.system_status.module_initialized) {
      if (!sent) {
        ///// Send HI Signal /////
        char* p = new char[1]{ CMD_HI };
        queue.push(3, 1, p);

        CoreBridge.system_status.module_connected = false;

        sent = true;
      }

      vTaskDelay(LIVE_DETECT_INTERVAL / portTICK_PERIOD_MS);

      if (sent) {
        static bool modules_removed = false;

        if (!CoreBridge.system_status.module_connected) {
          if (!modules_removed) {
            CoreBridge.digitalWrite(MODULES_STATE_PIN, 0);

            ///// Create an Empty Accessory /////
            CoreBridge.removeModules();
            Homekit.createAccessory(CoreBridge.serial_number, CoreBridge.device_name);
            Homekit.beginAccessory();
            MqttCtrl.modulesUpdate();

            modules_removed = true;
          }

          ///// Attempting Reconnection /////
          char* p = new char[2]{ CMD_LOAD_MODULE, 0x00 };
          queue.push(1, 2, p);
        } else {
          modules_removed = false;
        }

        sent = false;
      }
    } else {
      taskYIELD();
    }
  }
}

void onlinePeriodicTask(void*) {
  ///// Warehouse TEST /////
  //Warehouse.formatZero(false);

  //Test Hour Date Insertion
  /*printf("Current Hour Data Length: %i\n", Warehouse.getHourDataLength());
  printf("Cycle Record: %i\n", Warehouse.getCycleRecord());
  for (int i = 0; i < 24; i++) {
    Warehouse.appendHourlyRecord(24 - i, i);
  }
  printf("Current Hour Data Length: %i\n", Warehouse.getHourDataLength());
  printf("Cycle Record: %i\n", Warehouse.getCycleRecord());

  //Test Date Data Insertion
  int n = 0;
  for (int i = 0; i < Warehouse.getHourDataLength(); i++)
    n += Warehouse.readAsInt16(EEPROM_HOUR_DATA_PTR + (i * 3) + 1);
  n /= 24;
  //printf("Sum current for this day %i / %i is %i\n", timeinfo.tm_mon, timeinfo.tm_mday, n);
  Warehouse.appendDateRecord(timeinfo.tm_year, timeinfo.tm_mon, 0, n);
  //printf("Current Date Data Length: %i\n", Warehouse.getDateDataLength());

  //Test Date Length Loop Length
  for (int i = 0; i < EEPROM_DATE_RECORD_NUM; i++) {
    Warehouse.appendDateRecord(timeinfo.tm_year, timeinfo.tm_mon, 1 + i, n);
  }
  printf("Current Date Data Length: %i\n", Warehouse.getDateDataLength());
  //printf("Current Date Data Ptr: %i\n", Warehouse.getRecordedDatePtr());

  //Test Hour Data to Json
  cJSON* root = Warehouse.parseHourDatainJson();
  char* jsonPrint = cJSON_Print(root); //cJSON_PrintUnformatted
  printf("Hour Data in JSON: %s\n", jsonPrint);
  cJSON_free(jsonPrint);
  cJSON_Delete(root);

  //Test Date Data to Json
  cJSON*root = Warehouse.parseDateDatainJson();
  char* jsonPrint = cJSON_Print(root); //cJSON_PrintUnformatted
  printf("Date Data in JSON: %s\n", jsonPrint);
  cJSON_free(jsonPrint);
  cJSON_Delete(root);*/
  ///// Warehouse END /////

  ///// Sum Current if Not Today /////
  if (Warehouse.getDateDataLength() > 0 && !Warehouse.isLastDateDataToday(&CoreBridge.timeinfo)) {
    int avg_sum_current = 0;

    for (int i = 0; i < Warehouse.getHourDataLength(); i++)
      avg_sum_current += Warehouse.readAsInt16(EEPROM_HOUR_DATA_PTR + (i * 3) + 1);

    avg_sum_current /= 24;
    Warehouse.writeAsInt16(avg_sum_current, EEPROM_DATE_DATA_PTR + (Warehouse.getRecordedDatePtr() * 5) + 3);
    Warehouse.updateRecordedHourPtr(255);

    CoreBridge.getTime();
    Warehouse.appendDateRecord(CoreBridge.timeinfo.tm_year, CoreBridge.timeinfo.tm_mon, CoreBridge.timeinfo.tm_mday, 0);
  }

  ///// Check Hour Conflict //////
  int previousHr = Warehouse.getRecordedHourPtr();

  previousHr = (previousHr == 255) ? 255 : Warehouse.read(EEPROM_HOUR_DATA_PTR + (previousHr * 3));
  printf("previousHr: %i\n", previousHr);

  while (1) {
    if (CoreBridge.system_status.module_initialized) {
      CoreBridge.getTime();

      if (CoreBridge.timeinfo.tm_hour != previousHr) {
        ///// Restart Daily Current Record /////
        if (CoreBridge.timeinfo.tm_hour == 0)
          Warehouse.updateRecordedHourPtr(255);

        ///// Appened Today's Date Record if Not Existed
        if (!Warehouse.isLastDateDataToday(&CoreBridge.timeinfo)) {
          Warehouse.appendDateRecord(CoreBridge.timeinfo.tm_year, CoreBridge.timeinfo.tm_mon, CoreBridge.timeinfo.tm_mday, 0);
          //MqttCtrl.warehouseDataUpdate(1);
        }

        ///// Hold Average System Curre t if No Houry Data /////
        if (avg_sys_current_count == 0)
          avg_system_current = CoreBridge.system_status.sum_current;

        Warehouse.appendHourlyRecord(CoreBridge.timeinfo.tm_hour, avg_system_current);
        //MqttCtrl.warehouseDataUpdate(0);
        avg_system_current = 0;
        avg_sys_current_count = 0;

        if (CoreBridge.timeinfo.tm_hour == 23) { //New day is coming, summing current records
          int avg_sum_current = 0;

          for (int i = 0; i < Warehouse.getHourDataLength(); i++)
            avg_sum_current += Warehouse.readAsInt16(EEPROM_HOUR_DATA_PTR + (i * 3) + 1);

          avg_sum_current /= Warehouse.getHourDataLength();
          Warehouse.writeAsInt16(avg_sum_current, EEPROM_DATE_DATA_PTR + (Warehouse.getRecordedDatePtr() * 5) + 3);
          //MqttCtrl.warehouseDataUpdate(1);
        }

        previousHr = CoreBridge.timeinfo.tm_hour;
      } else {
        ///// Calculate Average System Current /////
        avg_system_current = (avg_system_current + CoreBridge.system_status.sum_current) / 2;
        avg_sys_current_count++;

        CoreBridge.requestModulesData(MODULE_CURRENT);
      }

      CoreBridge.getTime();
      vTaskDelay(min(5 * 60 * 1000, (((59 - CoreBridge.timeinfo.tm_min) * 60) + (60 - CoreBridge.timeinfo.tm_sec)) * 1000) / portTICK_PERIOD_MS);
    } else {
      taskYIELD();
    }
  }
}

void productLifetimeCounter(void*) {
  while (1) {
    vTaskDelay(60 * 1000 * 60 / portTICK_PERIOD_MS);
    
    CoreBridge.setRunningTime(CoreBridge.running_time + 1);
  }
}