#ifndef CUSTOM_DEFINITIONS_H
#define CUSTOM_DEFINITIONS_H

///// MAX NUMBER /////
#define SERIAL_NUMBER_LENGTH 12
#define DEVICE_NAME_LENGTH 32
#define MAX_MODULE_NUM 20
#define MAX_CURRENT 1500

///// INTERVALS /////
#define LIVE_DETECT_INTERVAL 1000
#define RECORD_SUM_CURRENT_INTERVAL 300000 //5 minutes

///// PIN SETS /////
#define RST_PIN                 2
#define WIFI_STATE_PIN          7
#define MODULES_STATE_PIN       9

///// UART COMMANDS /////
#define CMD_REQ_ADR 0x41         //'A'
#define CMD_LOAD_MODULE 0x42     //'B'
#define CMD_CONFIRM_RECEIVE 0x43 //'C'
#define CMD_DO_MODULE 0x44       //'D'
#define CMD_REQ_DATA 0x45        //'E'
#define CMD_UPDATE_FIRM 0x46     //'F'
#define CMD_HI 0x48              //'H'
#define CMD_INIT_MODULE 0x49     //'I'
#define CMD_LINK_MODULE 0x4C     //'L'
#define CMD_RESET_MODULE 0x52    //'R'
#define CMD_UPDATE_DATA 0x55     //'U'

///// Module Actions /////
#define DO_TURN_ON 0x6E  //'n'
#define DO_TURN_OFF 0x66 //'f'

///// Characteristic Type /////
#define MODULE_SWITCH_STATE 0x61 //'a'
#define MODULE_CURRENT 0x62      //'b'
#define MODULE_MCUB 0x63         //'c'
#define MODULE_PRIORITY 0x64     //'d'

#endif