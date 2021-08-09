/*
  This file is part of the Arduino NINA firmware.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdint.h>

// /*** SERIAL ***/
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

// /*** Module Actions ***/
#define DO_TURN_ON 0x6E  //'n'
#define DO_TURN_OFF 0x66 //'f'

// /*** Characteristic Type ***/
#define MODULE_SWITCH_STATE 0x61 //'a'
#define MODULE_CURRENT 0x62      //'b'
#define MODULE_MCUB 0x63         //'c'
#define MODULE_PRIORITY 0x64     //'d'

struct uart_msg_pack
{
  int port;
  int length;
  char *payload;
  uart_msg_pack *next;

  uart_msg_pack(int po, int l, char *p) : port(po), length(l), payload(p), next(0){};
};

class uartMsgQueue
{
private:
  uart_msg_pack *front;
  uart_msg_pack *back;
  int size;

public:
  uartMsgQueue() : front(0), back(0), size(0){};
  void push(int po, int l, char *p);
  uart_msg_pack *pop();
  bool isEmpty();
  int getSize();
};

extern uartMsgQueue queue;

class CommandHandlerClass
{
public:
  CommandHandlerClass();

  void begin();
  int handle(const uint8_t command[], uint8_t response[]);

private:
  static void gpio0Updater(void *);
  void updateGpio0Pin();

private:
  SemaphoreHandle_t _updateGpio0PinSemaphore;
};

extern CommandHandlerClass CommandHandler;

#endif
