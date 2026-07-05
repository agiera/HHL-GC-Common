#include "webusb.h"

// Wire protocol (1KB-capable):
//   OUT  request 10: [cmd, port, cmd_len_lo, cmd_len_hi, resp_len_lo, resp_len_hi, ...cmd]
//   IN   request 11: [cmd, port, resp_len_lo, resp_len_hi, ...response]
// Up to 1024 raw response bytes + 4-byte response header.
#define WEBUSB_RESP_HEADER_LEN  4
#define WEBUSB_RAW_PAYLOAD_MAX  1024
#define WEBUSB_OUT_BUFFER_SIZE  (WEBUSB_RESP_HEADER_LEN + WEBUSB_RAW_PAYLOAD_MAX)

uint8_t _webusb_out_buffer[WEBUSB_OUT_BUFFER_SIZE] = {0x00};
volatile uint16_t _webusb_response_len = 0;  // 0 = no response ready
bool _web_usb_indicate = false;
static int8_t _webusb_joybus_port = -1;

void webusb_joybus_check()
{
    if (_webusb_joybus_port < 0) return;
    static uint8_t buf[WEBUSB_RAW_PAYLOAD_MAX];
    uint16_t resp_len = 0;
    if (joybus_itf_consume_raw_response((uint8_t)_webusb_joybus_port, buf, &resp_len)) {
        if (resp_len > WEBUSB_RAW_PAYLOAD_MAX) resp_len = WEBUSB_RAW_PAYLOAD_MAX;
        _webusb_out_buffer[0] = WEBUSB_CMD_JOYBUS_CMD;
        _webusb_out_buffer[1] = (uint8_t)_webusb_joybus_port;
        _webusb_out_buffer[2] = (uint8_t)(resp_len & 0xff);
        _webusb_out_buffer[3] = (uint8_t)((resp_len >> 8) & 0xff);
        memcpy(&_webusb_out_buffer[WEBUSB_RESP_HEADER_LEN], buf, resp_len);
        _webusb_response_len = WEBUSB_RESP_HEADER_LEN + resp_len;
        _webusb_joybus_port = -1;
    }
}

void webusb_set_indicate()
{
    _web_usb_indicate = true;
}

void webusb_save_confirm()
{
    if(!_web_usb_indicate) return;

    printf("Sending Save receipt...\n");
    memset(_webusb_out_buffer, 0, 64);
    _webusb_out_buffer[0] = 0xF1;
    _webusb_response_len = 1;
    _web_usb_indicate = false;
}

void webusb_command_processor(uint8_t *data)
{
    switch(data[0])
    {
        default:

            break;

        case WEBUSB_CMD_JOYBUS_CMD:
        {
            uint8_t  port     = data[1];
            uint16_t cmd_len  = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
            uint16_t resp_len = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
            if (port < 4 && cmd_len >= 1 && cmd_len <= WEBUSB_RAW_PAYLOAD_MAX
                && resp_len >= 1 && resp_len <= WEBUSB_RAW_PAYLOAD_MAX) {
                joybus_itf_queue_raw_cmd(port, &data[6], cmd_len, resp_len);
                _webusb_joybus_port = (int8_t)port;
            }
        }
        break;

        case WEBUSB_CMD_FW_SET:
            {
                adapter_ll_reboot_bootloader();
            }
            break;

        case WEBUSB_CMD_FW_GET:
            {
                _webusb_out_buffer[0] = WEBUSB_CMD_FW_GET;
                _webusb_out_buffer[1] = (ADAPTER_FIRMWARE_VERSION & 0xFF00)>>8;
                _webusb_out_buffer[2] = ADAPTER_FIRMWARE_VERSION & 0xFF;
                _webusb_response_len = 3;
            }
            break;

        case WEBUSB_CMD_SAVEALL:
            {
                printf("WebUSB: Got SAVE command.\n");
                webusb_set_indicate();
                settings_save();
            }
            break;
    }
}