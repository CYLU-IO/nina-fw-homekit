#include <stdio.h>

#include <Wire.h>

#include "CoreBridge.h"

void WarehouseClass::begin() {
  Wire.begin();
}

int WarehouseClass::getHeadAddr() {
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
}

void WarehouseClass::clearStorage() {
  for (int i = 0; i <= EEPROM_BUFFER_LEN + 1 + (EEPROM_HEAD_ADDR + 2); i++) {
    this->write(0xff, i);
  }

  this->setHeadAddr(0x00);
}

void WarehouseClass::write(int val, int addr) {
  Wire.beginTransmission(EEPROM_I2C_ADDR);
  Wire.write((int)(addr >> 8));   // MSB
  Wire.write((int)(addr & 0xff)); // LSB

  Wire.write(val);
  Wire.endTransmission();

  delay(5);
}

int WarehouseClass::read(int addr) {
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

int WarehouseClass::readAsInt16(int addr) {
  return (this->read(addr) & 0xff) | this->read(addr + 1) << 8;
}

WarehouseClass Warehouse;