#include <stdio.h>

#include <Wire.h>
#include "CoreBridge.h"

void WarehouseClass::begin() {
  Wire.begin();
}

uint8_t WarehouseClass::getDayHeadPtr() {
  return this->read(EEPROM_DAY_HEAD_PTR);
}

void WarehouseClass::updateDayHeadPtr(uint8_t ptr) {
  this->write(ptr, EEPROM_DAY_HEAD_PTR);
}

uint8_t WarehouseClass::getDateHeadPtr() {
  return this->read(EEPROM_DATE_HEAD_PTR);
}

void WarehouseClass::updateDateHeadPtr(uint8_t ptr) {
  this->write(ptr, EEPROM_DATE_HEAD_PTR);
}


void WarehouseClass::writeDayData(uint16_t current) {
  uint8_t current_ptr = this->getDayHeadPtr();
  uint8_t destined_ptr = current_ptr + 1;

  ///// Accumulated to 24 hours, calculate the average and save in Date Data Collection /////
  if (current_ptr == 24) { //TODO: Change to Midnight 24:00 to do the action
    destined_ptr = 1;

    uint32_t data_current_buffer;

    for (int i = 0; i < 24; i++) {
      data_current_buffer += this->readAsInt16(EEPROM_DAY_DATA + 2 + (i * 4));
    }

    data_current_buffer /= 24;
  }

  //logic

  this->updateDayHeadPtr(destined_ptr);
}

/*void WarehouseClass::writeDateData() {
  uint8_t destined_data_ptr = this->getDateHeadPtr() + 1;
}*/
/*int WarehouseClass::getHeadAddr() {
  return this->readAsInt16(EEPROM_HEAD_ADDR);
}

int WarehouseClass::setHeadAddr(int addr) {
  this->write(addr & 0xff, EEPROM_HEAD_ADDR);
  this->write((addr >> 8) & 0xff, EEPROM_HEAD_ADDR + 1);

  return addr;
}

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
  /*int endAddr = EEPROM_BUFFER_LEN + (EEPROM_HEAD_ADDR + 2);

  if ((bool)param) {
    for (int i = 0; i < endAddr; i++)
      Warehouse.write(0xff, i);
  }

  Warehouse.write(0xff, endAddr);
  Warehouse.write(0xff, endAddr + 1);

  Warehouse.setHeadAddr(0x00);*/
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