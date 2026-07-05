#ifndef JOYBUS_H
#define JOYBUS_H

#include "adapter_includes.h"

void joybus_init();

// Define these functions in user-space
void joybus_itf_init();

// This function causes the joybus to poll
void joybus_itf_poll(joybus_input_s **out);

void joybus_itf_enable_rumble(uint8_t interface, bool enable);

// Queue a raw joybus command for a physical port (0-3).
// cmd_len/resp_len are in bytes (max 1024).
// Executed on the next poll cycle instead of the normal poll command.
void joybus_itf_queue_raw_cmd(uint8_t port, uint8_t *cmd, uint16_t cmd_len, uint16_t resp_len);

// Consume the raw response from the last raw command.
// Returns true and fills buf/out_len if a response is ready, false otherwise.
bool joybus_itf_consume_raw_response(uint8_t port, uint8_t *buf, uint16_t *out_len);

// Returns true if any port has a raw command queued that hasn't been sent yet.
// Used by the comms task to bypass the normal polling interval so WebUSB
// raw commands fire immediately instead of waiting for the next poll tick.
bool joybus_itf_has_raw_pending(void);

#endif