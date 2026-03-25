#ifndef WEBUSB_H
#define WEBUSB_H

#include "adapter_includes.h"

typedef enum
{
  // Send raw joybus command to a specific port and return the response.
  // Packet: [0x02, port (0-3), cmd_len (1-8), cmd_byte_0, ...]
  // Response: [0x02, port, response_byte_0..7, 0x00...]
  WEBUSB_CMD_JOYBUS_CMD = 0x02,

  // Set FW update mode
  WEBUSB_CMD_FW_SET = 0x0F,

  // Get firmware version
  WEBUSB_CMD_FW_GET = 0xAF,

  // Save all settings
  WEBUSB_CMD_SAVEALL = 0xF1,
} webusb_cmd_t;

void webusb_set_indicate();
void webusb_save_confirm();
void webusb_command_processor(uint8_t *data);
void webusb_joybus_check();

#endif