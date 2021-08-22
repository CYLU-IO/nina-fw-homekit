#include <stdio.h>

#include <Wire.h>
#include "CoreBridge.h"

void WarehouseClass::begin() {
  Wire.begin();
}

void WarehouseClass::increaseCycleRecord() {
  int c = ((this->read(EEPROM_CYCLE_RECORD) << 16) | (this->read(EEPROM_CYCLE_RECORD + 1) << 8) | (this->read(EEPROM_CYCLE_RECORD + 2) & 0xff)) + 1;

  this->write(EEPROM_CYCLE_RECORD, c >> 16 & 0xff);
  this->write(EEPROM_CYCLE_RECORD + 1, c >> 8 & 0xff);
  this->write(EEPROM_CYCLE_RECORD + 2, c & 0xff);
}

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
  uint8_t designatedHourPtr = this->getRecordedHourPtr();
  designatedHourPtr = (designatedHourPtr == 255) ? 0 : designatedHourPtr + 1;

  this->write(hr, EEPROM_HOUR_DATA_PTR + (designatedHourPtr * 3));
  this->writeAsInt16(EEPROM_HOUR_DATA_PTR + (designatedHourPtr * 3) + 1, current);
  this->updateRecordedHourPtr(designatedHourPtr);
  this->increaseCycleRecord();
}

void WarehouseClass::appendDateRecord(uint8_t yy, uint8_t mm, uint8_t dd, uint16_t avg_current) {
  uint8_t designatedDatePtr = this->getRecordedDatePtr(); //0~30
  designatedDatePtr = (designatedDatePtr == 255) ? 0 : designatedDatePtr + 1;

  if (designatedDatePtr == EEPROM_DATE_RECORD_NUM)  //already complete a loop
    designatedDatePtr = 0;

  this->write(yy, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5));
  this->write(mm, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5) + 1);
  this->write(dd, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5) + 2);
  this->writeAsInt16(avg_current, EEPROM_DATE_DATA_PTR + (designatedDatePtr * 5) + 3);
  this->updateRecordedDatePtr(designatedDatePtr);
}

void WarehouseClass::formatZero(bool deep) {
  if (deep)
    this->write(0, EEPROM_CYCLE_RECORD);

  this->write(255, EEPROM_HOUR_HEAD_PTR);
  this->write(255, EEPROM_DATE_HEAD_PTR);

  this->write(0, EEPROM_DATE_DATA_PTR + (30 * 5));
  this->write(0, EEPROM_DATE_DATA_PTR + (30 * 5) + 1);
  this->write(0, EEPROM_DATE_DATA_PTR + (30 * 5) + 2);
  this->writeAsInt16(0, EEPROM_DATE_DATA_PTR + (30 * 5) + 3);
}

/*
int WarehouseClass::getAvailableLength() {
  return (this->getHeadAddr() / 2) + (this->readAsInt16((EEPROM_HEAD_ADDR + 2) + EEPROM_BUFFER_LEN) == 65535 ? 0 : EEPROM_BUFFER_LEN / 2);
}

int WarehouseClass::appendData(int value) {
  int addr = this->getHeadAddr() + 2;

  if (addr > EEPROM_BUFFER_LEN + (EEPROM_HEAD_ADDR + 2))
    addr = EEPROM_HEAD_ADDR + 2;

  this->write(value & 0xff, addr);
  this->write((value >> 8) & 0xff, addr + 1);

  return this->setHeadAddr(addr);
}

void WarehouseClass::getDataPack(int addr, int& amount, int* buffer) {
  int head_addr = this->getHeadAddr();
  int warehouse_length = this->getAvailableLength();

  if (amount > warehouse_length)
    amount = warehouse_length;

  //amount: 144
  for (int i = 0; i < amount; i++) {
    if (head_addr / 2 <= i)
      addr = EEPROM_BUFFER_LEN / 2 + (EEPROM_HEAD_ADDR + 2);

    buffer[i] = this->readAsInt16(addr - (i * 2));
  }
}

void WarehouseClass::getDataByPage(int page, int& amount, int* buffer) {
  int head_addr = this->getHeadAddr();
  int warehouse_length = this->getAvailableLength();

  int last_data_addr = 0x00;
  if (warehouse_length * 2 > head_addr) {
    last_data_addr = EEPROM_BUFFER_LEN - (warehouse_length * 2 - head_addr);
  }

  int constant_amount = amount;

  if (warehouse_length - (constant_amount * page) < constant_amount) {
    amount = warehouse_length - (constant_amount * page);
  }

  int target_addr = last_data_addr + (constant_amount * 2) + (constant_amount * 2 * (page - 1)) + (amount * 2);

  this->getDataPack(target_addr, amount, buffer);
}*/

void clearStorage(void* param) {

  vTaskDelete(NULL);
}

void WarehouseClass::write(uint8_t val, uint16_t addr) {
  Wire.beginTransmission(EEPROM_I2C_ADDR);
  Wire.write((int)(addr >> 8));   // MSB
  Wire.write((int)(addr & 0xff)); // LSB

  Wire.write(val);
  Wire.endTransmission();
  vTaskDelay(5);
}

void WarehouseClass::writeAsInt16(uint16_t ptr, uint16_t value) {
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