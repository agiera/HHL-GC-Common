#include "webusb.h"

uint8_t _webusb_out_buffer[131] = {0x00};  // 3-byte header + up to 128 response
volatile uint8_t _webusb_response_len = 0;  // 0 = no response ready
bool _web_usb_indicate = false;
static int8_t _webusb_joybus_port = -1;

void webusb_joybus_check()
{
    if (_webusb_joybus_port < 0) return;
    uint8_t buf[128];
    uint8_t resp_len = 0;
    if (joybus_itf_consume_raw_response((uint8_t)_webusb_joybus_port, buf, &resp_len)) {
        memset(_webusb_out_buffer, 0, sizeof(_webusb_out_buffer));
        _webusb_out_buffer[0] = WEBUSB_CMD_JOYBUS_CMD;
        _webusb_out_buffer[1] = (uint8_t)_webusb_joybus_port;
        _webusb_out_buffer[2] = resp_len;
        memcpy(&_webusb_out_buffer[3], buf, resp_len);
        _webusb_response_len = 3 + resp_len;
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
            uint8_t port     = data[1];
            uint8_t cmd_len  = data[2];
            uint8_t resp_len = data[3];
            if (port < 4 && cmd_len >= 1 && cmd_len <= 128 && resp_len >= 1 && resp_len <= 128) {
                joybus_itf_queue_raw_cmd(port, &data[4], cmd_len, resp_len);
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