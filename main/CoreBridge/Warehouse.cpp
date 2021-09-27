#include <stdio.h>
#include <Wire.h>

#include "CoreBridge.h"

void WarehouseClass::init() {
  Wire.begin();
}

int WarehouseClass::getCycleRecord() {
  int c = this->read(EEPROM_CYCLE_RECORD) << 16;

  c |= this->read(EEPROM_CYCLE_RECORD + 1) << 8;
  c |= this->read(EEPROM_CYCLE_RECORD + 2) & 0xff;
  return c;
}

void WarehouseClass::increaseCycleRecord() {
  int c = this->getCycleRecord() + 1;

  this->write(c >> 16 & 0xff, EEPROM_CYCLE_RECORD);
  this->write(c >> 8 & 0xff, EEPROM_CYCLE_RECORD + 1);
  this->write(c & 0xff, EEPROM_CYCLE_RECORD + 2);
}

///// Current Recorder /////
uint8_t WarehouseClass::getRecordedHourPtr() {
  return this->read(EEPROM_HOUR_HEAD_PTR);
}

void WarehouseClass::updateRecordedHourPtr(uint8_t ptr) {
  this->write(ptr, EEPROM_HOUR_HEAD_PTR);
}

uint8_t WarehouseClass::getRecordedDatePtr() {
  return this->read(EEPROM_DATE_HEAD_PTR);
}

void WarehouseClass::updateRecordedDatePtr(uint8_t ptr) {
  this->write(ptr, EEPROM_DATE_HEAD_PTR);
}

void WarehouseClass::appendHourlyRecord(uint8_t hr, uint16_t current) {
  uint8_t designatedHourPtr = this->getRecordedHourPtr() + 1;

  this->write(hr, EEPROM_HOUR_DATA_PTR + (designatedHourPtr * 3));
  this->writeAsInt16(current, EEPROM_HOUR_DATA_PTR + (designatedHourPtr * 3) + 1);
  this->updateRecordedHourPtr(designatedHourPtr);
  this->increaseCycleRecord();
}

void WarehouseClass::appendDateRecord(uint8_t yy, uint8_t mm, uint8_t dd, uint16_t avg_current) {
  uint8_t designatedDatePtr = this->getRecordedDatePtr() + 1; //0~30

  if (designatedDatePtr == EEPROM_DATE_RECORD_NUM)  //already complete a loop
    designatedDatePtr = 0;

  this->write(yy, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5));
  this->write(mm, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5) + 1);
  this->write(dd, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5) + 2);
  this->writeAsInt16(avg_current, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5) + 3);
  this->updateRecordedDatePtr(designatedDatePtr);
}

void WarehouseClass::formatZero(bool deep) {
  if (deep) {
    this->write(0, EEPROM_CYCLE_RECORD);
    this->write(0, EEPROM_CYCLE_RECORD + 1);
    this->write(0, EEPROM_CYCLE_RECORD + 2);
  }

  this->write(255, EEPROM_HOUR_HEAD_PTR);
  this->write(255, EEPROM_DATE_HEAD_PTR);

  this->write(0, EEPROM_DATE_DATA_PTR + ((EEPROM_DATE_RECORD_NUM - 1) * 5));
  this->write(0, EEPROM_DATE_DATA_PTR + ((EEPROM_DATE_RECORD_NUM - 1) * 5) + 1);
  this->write(0, EEPROM_DATE_DATA_PTR + ((EEPROM_DATE_RECORD_NUM - 1) * 5) + 2);
  this->writeAsInt16(0, EEPROM_DATE_DATA_PTR + ((EEPROM_DATE_RECORD_NUM - 1) * 5) + 3);
}

int WarehouseClass::getHourDataLength() {
  uint8_t n = this->getRecordedHourPtr() + 1;

  return n;
}

int WarehouseClass::getDateDataLength() {
  uint8_t n = this->getRecordedDatePtr() + 1;

  if (n < EEPROM_DATE_RECORD_NUM && this->read(EEPROM_DATE_DATA_PTR + ((EEPROM_DATE_RECORD_NUM - 1) * 5)) > 0)
    n = EEPROM_DATE_RECORD_NUM;

  return n;
}

cJSON* WarehouseClass::parseHourDatainJson() {
  cJSON* root;
  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "CURRENT_HISTORY_UPDATE");
  cJSON_AddStringToObject(root, "scale", "Hour");

  cJSON* timeArr = cJSON_CreateArray();
  cJSON* dataArr = cJSON_CreateArray();

  for (int i = 0; i < this->getHourDataLength(); i++) {
    cJSON* t;
    cJSON* d;
    cJSON_AddItemToArray(timeArr, t = cJSON_CreateNumber(this->read(EEPROM_HOUR_DATA_PTR + (i * 3))));
    cJSON_AddItemToArray(dataArr, d = cJSON_CreateNumber(this->readAsInt16(EEPROM_HOUR_DATA_PTR + (i * 3) + 1)));
  }

  cJSON_AddItemToObject(root, "time", timeArr);
  cJSON_AddItemToObject(root, "data", dataArr);

  return root;
}

cJSON* WarehouseClass::parseDateDatainJson() {
  uint8_t length = this->getDateDataLength();
  uint8_t dataPtr = this->getRecordedDatePtr();

  cJSON* root;
  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", "CURRENT_HISTORY_UPDATE");
  cJSON_AddStringToObject(root, "scale", "Date");

  cJSON* timeArr = cJSON_CreateArray();
  cJSON* dataArr = cJSON_CreateArray();

  if (length == EEPROM_DATE_RECORD_NUM && dataPtr < EEPROM_DATE_RECORD_NUM) {
    for (int i = dataPtr + 1; i < EEPROM_DATE_RECORD_NUM; i++) {
      cJSON* t;
      cJSON* d;

      char* dateStr = new char[10];
      sprintf(dateStr, "%i-%i-%i",
      (1900 + this->read(EEPROM_DATE_DATA_PTR + (i * 5))),
      (this->read(EEPROM_DATE_DATA_PTR + (i * 5) + 1) + 1),
      this->read(EEPROM_DATE_DATA_PTR + (i * 5) + 2));

      cJSON_AddItemToArray(timeArr, t = cJSON_CreateString(dateStr));
      cJSON_AddItemToArray(dataArr, d = cJSON_CreateNumber(this->readAsInt16(EEPROM_DATE_DATA_PTR + (i * 5) + 3)));

      delete[] dateStr;
    }
  }

  if (this->getDateDataLength() > 0) {
    for (int i = 0; i < dataPtr + 1; i++) {
      cJSON* t;
      cJSON* d;

      char* dateStr = new char[10];
      sprintf(dateStr, "%i-%i-%i",
      (1900 + this->read(EEPROM_DATE_DATA_PTR + (i * 5))),
      (this->read(EEPROM_DATE_DATA_PTR + (i * 5) + 1) + 1),
      this->read(EEPROM_DATE_DATA_PTR + (i * 5) + 2));

      cJSON_AddItemToArray(timeArr, t = cJSON_CreateString(dateStr));
      cJSON_AddItemToArray(dataArr, d = cJSON_CreateNumber(this->readAsInt16(EEPROM_DATE_DATA_PTR + (i * 5) + 3)));

      delete[] dateStr;
    }
  }

  cJSON_AddItemToObject(root, "time", timeArr);
  cJSON_AddItemToObject(root, "data", dataArr);

  return root;
}

bool WarehouseClass::isLastDateDataToday(tm* timeinfo) {
  uint8_t dateDataPtr = this->getRecordedDatePtr();

  if (dateDataPtr < EEPROM_DATE_RECORD_NUM &&
    timeinfo->tm_year == this->read(EEPROM_DATE_DATA_PTR + (dateDataPtr * 5)) &&
    timeinfo->tm_mon == this->read(EEPROM_DATE_DATA_PTR + (dateDataPtr * 5) + 1) &&
    timeinfo->tm_mday == this->read(EEPROM_DATE_DATA_PTR + (dateDataPtr * 5) + 2)) {
    return true;
  }

  return false;
}

void clearWarehouseData(void*) {
  time_t now;
  tm timeinfo;

  Warehouse.formatZero(false);

  time(&now);
  localtime_r(&now, &timeinfo);
  Warehouse.appendHourlyRecord(timeinfo.tm_hour, CoreBridge.system_status.sum_current);
  Warehouse.appendDateRecord(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, 0);
  
  MqttCtrl.warehouseDataUpdate(0);
  MqttCtrl.warehouseDataUpdate(1);
  CoreBridge.system_status.reset2factorying = 0;
  vTaskDelete(NULL);
}

///// Logging System /////
/*uint8_t WarehouseClass::getLogHeadPtr() {

}

void WarehouseClass::updateLogHeadPtr(uint8_t ptr) {

}

void WarehouseClass::appendLog(tm* timeinfo, uint8_t type, ) {

}*/

///// Essentials /////
void WarehouseClass::write(uint8_t val, uint16_t addr) {
  Wire.beginTransmission(EEPROM_I2C_ADDR);
  Wire.write((int)(addr >> 8));   // MSB
  Wire.write((int)(addr & 0xff)); // LSB
  Wire.write(val);
  Wire.endTransmission();
  vTaskDelay(10 / portTICK_PERIOD_MS);
}

void WarehouseClass::writeAsInt16(uint16_t value, uint16_t ptr) {
  this->write(value & 0xff, ptr);
  this->write(value >> 8, ptr + 1);
}

uint8_t WarehouseClass::read(uint16_t addr) {
  char re = 0xff;

  Wire.beginTransmission(EEPROM_I2C_ADDR);
  Wire.write((int)(addr >> 8));   // MSB
  Wire.write((int)(addr & 0xff)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_I2C_ADDR, 1);

  if (Wire.available())
    re = Wire.read();

  return re;
}

uint16_t WarehouseClass::readAsInt16(uint16_t ptr) {
  return (this->read(ptr) & 0xff) | this->read(ptr + 1) << 8;
}

WarehouseClass Warehouse;