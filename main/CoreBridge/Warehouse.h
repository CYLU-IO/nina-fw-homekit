#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#define EEPROM_I2C_ADDR 0x50
#define EEPROM_HEAD_ADDR 0x00
#define EEPROM_BUFFER_LEN 14400

class WarehouseClass {
private:
  void write(int val, int addr);
  int read(int addr);
  int readAsInt16(int addr);

public:
  void begin();

  int getHeadAddr();
  int setHeadAddr(int addr);
  int getAvailableLength();

  int appendData(int value);
  void getDataPack(int addr, int& amount, int* buffer);
  void getDataByPage(int page, int& amount, int* buffer);

  void clearStorage();
};

extern WarehouseClass Warehouse;

#endif