#ifndef JOYBUS_H
#define JOYBUS_H

#include "adapter_includes.h"

void joybus_init();

// Define these functions in user-space
void joybus_itf_init();

// This function causes the joybus to poll
void joybus_itf_poll(joybus_input_s **out);

void joybus_itf_enable_rumble(uint8_t interface, bool enable);

// Queue a raw joybus command (1-8 bytes) for a physical port (0-3).
// Executed on the next poll cycle instead of the normal poll command.
void joybus_itf_queue_raw_cmd(uint8_t port, uint8_t *cmd, uint8_t len);

// Consume the raw response from the last raw command (8 bytes).
// Returns true and fills buf if a response is ready, false otherwise.
bool joybus_itf_consume_raw_response(uint8_t port, uint8_t *buf);

#endif