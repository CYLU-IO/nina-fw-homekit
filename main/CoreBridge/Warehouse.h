#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#define EEPROM_I2C_ADDR 0x50

#define EEPROM_CYCLE_RECORD 0x00
#define EEPROM_HOUR_HEAD_PTR 0x03
#define EEPROM_DATE_HEAD_PTR 0x04
#define EEPROM_HOUR_DATA_PTR 0x05 //Minimum space: 72
#define EEPROM_DATE_DATA_PTR 0x4d //Minimum space: 5 * 31
#define EEPROM_DATE_RECORD_NUM 31

class WarehouseClass {
private:
  void write(uint8_t val, uint16_t addr);
  uint8_t read(uint16_t addr);

public:
  void begin();

  void increaseCycleRecord();

  uint8_t getRecordedHourPtr();
  void updateRecordedHourPtr(uint8_t ptr);

  uint8_t getRecordedDatePtr();
  void updateRecordedDatePtr(uint8_t ptr);

  void appendHourlyRecord(uint8_t hr, uint16_t current);
  void appendDateRecord(uint8_t yy, uint8_t mm, uint8_t dd, uint16_t avg_current);
  void formatZero(bool deep);

  void writeAsInt16(uint16_t ptr, uint16_t value);
  uint16_t readAsInt16(uint16_t ptr);
};

void clearStorage(void* param);

extern WarehouseClass Warehouse;

#endif