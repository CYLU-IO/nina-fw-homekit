#include <lwip/sockets.h>
#include "esp_log.h"

#include <Arduino.h>
#include "CommandHandler.h"
#include "CoreBridge/CoreBridge.h"

#include <nvs_flash.h>
#include <esp_wifi.h>

int corebridge_getFreeHeap(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = esp_get_free_heap_size();

  return 6;
}

int corebridge_getEnablePOP(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.enable_pop;

  return 6;
}

int wifimgr_getStatus(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = WifiMgr.getStatus();

  return 6;
}

int resetNetwork(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  WifiMgr.resetNetwork();

  return 6;
}

int resetToFactory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  nvs_flash_erase();
  esp_restart();

  return 6;
}

int createAccessory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.createAccessory();

  return 6;
}

int countAccessory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.countAccessory();

  return 6;
}

int beginAccessory(const uint8_t command[], uint8_t response[])
{
  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.beginHomekit();

  MqttCtrl.modulesUpdate();

  return 6;
}

int addModule(const uint8_t command[], uint8_t response[])
{
  char name[25 + 1];

  uint8_t index = command[4];
  uint8_t type = command[6];
  uint8_t priority = command[8];
  uint8_t state = command[10];


  memset(name, 0x00, sizeof(name));
  memcpy(name, &command[12], command[11]);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.addModule(index, name, type, priority, state);

  return 6;
}

int setModuleSwitchState(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];
  uint8_t state = command[6];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.setModuleSwitchState(index, state, false);

  return 6;
}

int getModuleSwitchState(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.getModuleSwitchState(index);

  return 6;
}

int setModuleCurrent(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];
  uint16_t value = (command[7] & 0xff | (command[8] << 8) & 0xff00);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.setModuleCurrent(index, value);

  return 6;
}

int getModulePrioirty(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.getModulePrioirty(index);

  return 6;
}

int readModuleTriggered(const uint8_t command[], uint8_t response[])
{
  uint8_t index = command[4];

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = CoreBridge.readModuleTriggered(index);

  return 6;
}

int pushWarehouseBuffer(const uint8_t command[], uint8_t response[])
{
  uint16_t length = ((command[3] << 8) & 0xff00) | (command[4] & 0xff);

  int *warehouseBuffer = (int *)malloc(length / 2 * sizeof(int));

  for (int i = 0; i < length / 2; i++)
  {
    warehouseBuffer[i] = (command[5 + (i * 2)] & 0xff | (command[6 +(i * 2)] << 8) & 0xff00);
  }

  MqttCtrl.warehouseRequestBufferUpdate(warehouseBuffer, length / 2);
  free(warehouseBuffer);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  return 6;
}

int setPinMode(const uint8_t command[], uint8_t response[])
{
  uint8_t pin = command[4];
  uint8_t mode = command[6];

  pinMode(pin, mode);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  return 6;
}

int setDigitalWrite(const uint8_t command[], uint8_t response[])
{
  uint8_t pin = command[4];
  uint8_t value = command[6];

  digitalWrite(pin, value);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  return 6;
}

int setAnalogWrite(const uint8_t command[], uint8_t response[])
{
  uint8_t pin = command[4];
  uint8_t value = command[6];

  analogWrite(pin, value);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = 1;

  return 6;
}

int getDigitalRead(const uint8_t command[], uint8_t response[])
{
  uint8_t pin = command[4];

  int const pin_status = digitalRead(pin);

  response[2] = 1; // number of parameters
  response[3] = 1; // parameter 1 length
  response[4] = (uint8_t)pin_status;

  return 6;
}

extern "C"
{
#include <driver/adc.h>
}

int getAnalogRead(const uint8_t command[], uint8_t response[])
{
  uint8_t adc_channel = command[4];

  /* Initialize the ADC. */
  adc_gpio_init(ADC_UNIT_1, (adc_channel_t)adc_channel);
  /* Set maximum analog bit-width = 12 bit. */
  adc1_config_width(ADC_WIDTH_BIT_12);
  /* Configure channel attenuation. */
  adc1_config_channel_atten((adc1_channel_t)adc_channel, ADC_ATTEN_DB_0);
  /* Read the analog value from the pin. */
  uint16_t const adc_raw = adc1_get_raw((adc1_channel_t)adc_channel);

  response[2] = 1;               // number of parameters
  response[3] = sizeof(adc_raw); // parameter 1 length = 2 bytes
  memcpy(&response[4], &adc_raw, sizeof(adc_raw));

  return 7;
}

int writeFile(const uint8_t command[], uint8_t response[])
{
  char filename[32 + 1];
  size_t len;
  size_t offset;

  memcpy(&offset, &command[4], command[3]);
  memcpy(&len, &command[5 + command[3]], command[4 + command[3]]);

  memset(filename, 0x00, sizeof(filename));
  memcpy(filename, &command[6 + command[3] + command[4 + command[3]]], command[5 + command[3] + command[4 + command[3]]]);

  FILE *f = fopen(filename, "ab+");
  if (f == NULL)
  {
    return -1;
  }

  fseek(f, offset, SEEK_SET);
  const uint8_t *data = &command[7 + command[3] + command[4 + command[3]] + command[5 + command[3] + command[4 + command[3]]]];

  int ret = fwrite(data, 1, len, f);
  fclose(f);

  return ret;
}

int readFile(const uint8_t command[], uint8_t response[])
{
  char filename[32 + 1];
  size_t len;
  size_t offset;

  memcpy(&offset, &command[4], command[3]);
  memcpy(&len, &command[5 + command[3]], command[4 + command[3]]);

  memset(filename, 0x00, sizeof(filename));
  memcpy(filename, &command[6 + command[3] + command[4 + command[3]]], command[5 + command[3] + command[4 + command[3]]]);

  FILE *f = fopen(filename, "rb");
  if (f == NULL)
  {
    return -1;
  }
  fseek(f, offset, SEEK_SET);
  int ret = fread(&response[4], len, 1, f);
  fclose(f);

  response[2] = 1;   // number of parameters
  response[3] = len; // parameter 1 length

  return len + 5;
}

int deleteFile(const uint8_t command[], uint8_t response[])
{
  char filename[32 + 1];
  size_t len;
  size_t offset;

  memcpy(&offset, &command[4], command[3]);
  memcpy(&len, &command[5 + command[3]], command[4 + command[3]]);

  memset(filename, 0x00, sizeof(filename));
  memcpy(filename, &command[6 + command[3] + command[4 + command[3]]], command[5 + command[3] + command[4 + command[3]]]);

  int ret = -1;
  struct stat st;
  if (stat(filename, &st) == 0)
  {
    // Delete it if it exists
    ret = unlink(filename);
  }
  return 0;
}

#include <driver/uart.h>

int applyOTA(const uint8_t command[], uint8_t response[])
{
  return 0;
}

int renameFile(const uint8_t command[], uint8_t response[])
{
  char old_file_name[64 + 1] = {0};
  char new_file_name[64 + 1] = {0};

  memset(old_file_name, 0, sizeof(old_file_name));
  memcpy(old_file_name, &command[4], command[3]);

  memset(new_file_name, 0, sizeof(new_file_name));
  memcpy(new_file_name, &command[5 + command[3]], command[4 + command[3]]);

  errno = 0;
  rename(old_file_name, new_file_name);

  /* Set up the response packet containing the ERRNO error number */
  response[2] = 1;     /* Number of parameters */
  response[3] = 1;     /* Length of parameter 1 */
  response[4] = errno; /* The actual payload */

  return 6;
}

int existsFile(const uint8_t command[], uint8_t response[])
{
  char filename[32 + 1];
  size_t len;
  size_t offset;

  memcpy(&offset, &command[4], command[3]);
  memcpy(&len, &command[5 + command[3]], command[4 + command[3]]);

  memset(filename, 0x00, sizeof(filename));
  memcpy(filename, &command[6 + command[3] + command[4 + command[3]]], command[5 + command[3] + command[4 + command[3]]]);

  int ret = -1;

  struct stat st;
  ret = stat(filename, &st);
  if (ret != 0)
  {
    st.st_size = -1;
  }
  memcpy(&response[4], &(st.st_size), sizeof(st.st_size));

  response[2] = 1;                  // number of parameters
  response[3] = sizeof(st.st_size); // parameter 1 length

  return 10;
}

int downloadFile(const uint8_t command[], uint8_t response[])
{
  char url[64 + 1];
  char filename[64 + 1];

  memset(url, 0x00, sizeof(url));
  memset(filename, 0x00, sizeof(filename));

  memcpy(url, &command[4], command[3]);
  memcpy(filename, "/fs/", strlen("/fs/"));
  memcpy(&filename[strlen("/fs/")], &command[5 + command[3]], command[4 + command[3]]);

  FILE *f = fopen(filename, "w");
  downloadAndSaveFile(url, f, 0);
  fclose(f);

  return 0;
}

/**
 * Static table used for the table_driven implementation.
 */
static const uint32_t crc_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

uint32_t crc_update(uint32_t crc, const void *data, size_t data_len)
{
  const unsigned char *d = (const unsigned char *)data;
  unsigned int tbl_idx;

  while (data_len--)
  {
    tbl_idx = (crc ^ *d) & 0xff;
    crc = (crc_table[tbl_idx] ^ (crc >> 8)) & 0xffffffff;
    d++;
  }

  return crc & 0xffffffff;
}

int downloadOTA(const uint8_t command[], uint8_t response[])
{
  static const char *OTA_TAG = "OTA";
  static const char *OTA_FILE = "/fs/UPDATE.BIN.LZSS";
  static const char *OTA_TEMP_FILE = "/fs/UPDATE.BIN.LZSS.TMP";

  typedef enum OTA_Error
  {
    ERR_NO_ERROR = 0,
    ERR_OPEN = 1,
    ERR_LENGTH = 2,
    ERR_CRC = 3,
    ERR_RENAME = 4,
  };

  union
  {
    struct __attribute__((packed))
    {
      uint32_t len;
      uint32_t crc32;
    } header;
    uint8_t buf[sizeof(header)];
  } ota_header;

  int ota_size, c;
  uint32_t crc32;

  /* Retrieve the URL parameter. */
  char url[128 + 1];
  memset(url, 0, sizeof(url));
  memcpy(url, &command[4], command[3]);
  ESP_LOGI(OTA_TAG, "url: %s", url);

  /* Set up the response packet. */
  response[2] = 1;            /* Number of parameters */
  response[3] = 1;            /* Length of parameter 1 */
  response[4] = ERR_NO_ERROR; /* The actual payload */

  /* Download the OTA file */
  FILE *f = fopen(OTA_TEMP_FILE, "w+");
  if (!f)
  {
    ESP_LOGE(OTA_TAG, "fopen(..., \"w+\") error: %d", ferror(f));
    response[4] = ERR_OPEN;
    goto ota_cleanup;
  }
  downloadAndSaveFile(url, f, 0);

  /* Determine size of downloaded file. */
  ota_size = ftell(f) - sizeof(ota_header.buf);
  /* Reposition file pointer at start of file. */
  rewind(f);
  /* Read the OTA header. */
  fread(ota_header.buf, sizeof(ota_header.buf), 1, f);
  ESP_LOGI(OTA_TAG, "ota image length = %d", ota_header.header.len);
  ESP_LOGI(OTA_TAG, "ota image crc32 = %X", ota_header.header.crc32);

  /* Check length. */
  if (ota_header.header.len != ota_size)
  {
    ESP_LOGE(OTA_TAG, "error ota length: expected %d, actual %d", ota_header.header.len, ota_size);
    response[4] = ERR_LENGTH;
    goto ota_cleanup;
  }

  /* Init CRC */
  crc32 = 0xFFFFFFFF;
  /* Calculate CRC */
  c = fgetc(f);
  while (c != EOF)
  {
    crc32 = crc_update(crc32, &c, 1);
    c = fgetc(f);
  }
  /* Finalise CRC */
  crc32 ^= 0xFFFFFFFF;

  /* Check CRC. */
  if (ota_header.header.crc32 != crc32)
  {
    ESP_LOGE(OTA_TAG, "error ota crc: expected %X, actual %X", ota_header.header.crc32, crc32);
    response[4] = ERR_CRC;
    goto ota_cleanup;
  }

  /* Close the file. */
  fclose(f);

  /* Rename in case of success. */
  errno = 0;
  rename(OTA_TEMP_FILE, OTA_FILE);
  if (errno)
  {
    ESP_LOGE(OTA_TAG, "rename(...) error: %d", errno);
    response[4] = ERR_RENAME;
    goto ota_cleanup;
  }

  return 6;

ota_cleanup:
  fclose(f);
  unlink(OTA_TEMP_FILE);
  return 6;
}

typedef int (*CommandHandlerType)(const uint8_t command[], uint8_t response[]);

const CommandHandlerType commandHandlers[] = {
    // 0x00 -> 0x0f
    corebridge_getEnablePOP,
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
    corebridge_getFreeHeap,

    // 0x10 -> 0x1f
    NULL,
    wifimgr_getStatus,
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
    resetNetwork,
    resetToFactory,

    // 0x20 -> 0x2f
    createAccessory,
    countAccessory,
    beginAccessory,
    addModule,
    setModuleSwitchState,
    getModuleSwitchState,
    setModuleCurrent,
    getModulePrioirty,
    readModuleTriggered,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    // 0x30 -> 0x3f
    pushWarehouseBuffer,
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
    NULL,

    // 0x40 -> 0x4f
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
    NULL,
    NULL,

    // 0x50 -> 0x5f
    setPinMode,
    setDigitalWrite,
    setAnalogWrite,
    getDigitalRead,
    getAnalogRead,
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

    // 0x60 -> 0x6f
    writeFile,
    readFile,
    deleteFile,
    existsFile,
    downloadFile,
    applyOTA,
    renameFile,
    downloadOTA,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

#define NUM_COMMAND_HANDLERS (sizeof(commandHandlers) / sizeof(commandHandlers[0]))

CommandHandlerClass::CommandHandlerClass()
{
}

static const int GPIO_IRQ = 0;

void CommandHandlerClass::begin()
{
  pinMode(GPIO_IRQ, OUTPUT);

  _updateGpio0PinSemaphore = xSemaphoreCreateCounting(2, 0);

  xTaskCreatePinnedToCore(CommandHandlerClass::gpio0Updater, "gpio0Updater", 4096, NULL, 1, NULL, 1);
}

#define UDIV_UP(a, b) (((a) + (b)-1) / (b))
#define ALIGN_UP(a, b) (UDIV_UP(a, b) * (b))

int CommandHandlerClass::handle(const uint8_t command[], uint8_t response[])
{
  int responseLength = 0;

  if (command[0] == 0xe0 && command[1] < NUM_COMMAND_HANDLERS)
  {
    CommandHandlerType commandHandlerType = commandHandlers[command[1]];

    if (commandHandlerType)
    {
      responseLength = commandHandlerType(command, response);
    }
  }

  if (responseLength == 0)
  {
    response[0] = 0xef;
    response[1] = 0x00;
    response[2] = 0xee;

    responseLength = 3;
  }
  else
  {
    response[0] = 0xe0;
    response[1] = (0x80 | command[1]);
    response[responseLength - 1] = 0xee;
  }

  xSemaphoreGive(_updateGpio0PinSemaphore);

  return ALIGN_UP(responseLength, 4);
}

void CommandHandlerClass::gpio0Updater(void *)
{
  while (1)
  {
    CommandHandler.updateGpio0Pin();
  }
}

void CommandHandlerClass::updateGpio0Pin()
{
  xSemaphoreTake(_updateGpio0PinSemaphore, portMAX_DELAY);
  digitalWrite(GPIO_IRQ, HIGH);
  vTaskDelay(1);
}

CommandHandlerClass CommandHandler;
