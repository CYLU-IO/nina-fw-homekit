#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#define EEPROM_I2C_ADDR 0x50

#define EEPROM_CYCLE_RECORD 0x00
#define EEPROM_DAY_HEAD_PTR 0x03
#define EEPROM_DATE_HEAD_PTR 0x04
#define EEPROM_DAY_DATA 0x05

class WarehouseClass {
private:
  void write(uint8_t val, uint16_t addr);
  uint8_t read(uint16_t addr);

public:
  void begin();

  uint8_t getDayHeadPtr();
  void updateDayHeadPtr(uint8_t ptr);

  uint8_t getDateHeadPtr();
  void updateDateHeadPtr(uint8_t ptr);

  void writeDayData(uint16_t current);

    /*int getHeadAddr();
    int setHeadAddr(int addr);
    int getAvailableLength();

    int appendData(int value);
    void getDataPack(int addr, int& amount, int* buffer);
    void getDataByPage(int page, int& amount, int* buffer);*/

    void writeAsInt16(uint16_t ptr, uint16_t value);
  uint16_t readAsInt16(uint16_t ptr);
};

void clearStorage(void* param);

extern WarehouseClass Warehouse;

#endif