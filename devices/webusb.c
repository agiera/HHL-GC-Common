#include "webusb.h"

uint8_t _webusb_out_buffer[64] = {0x00};
bool _web_usb_indicate = false;
static int8_t _webusb_joybus_port = -1;

void webusb_joybus_check()
{
    if (_webusb_joybus_port < 0) return;
    uint8_t buf[8];
    if (joybus_itf_consume_raw_response((uint8_t)_webusb_joybus_port, buf)) {
        memset(_webusb_out_buffer, 0, 64);
        _webusb_out_buffer[0] = WEBUSB_CMD_JOYBUS_CMD;
        _webusb_out_buffer[1] = (uint8_t)_webusb_joybus_port;
        memcpy(&_webusb_out_buffer[2], buf, 8);
        tud_vendor_n_write(0, _webusb_out_buffer, 64);
        tud_vendor_n_flush(0);
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
    tud_vendor_n_write(0, _webusb_out_buffer, 64);
    tud_vendor_n_flush(0);
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
            uint8_t port    = data[1];
            uint8_t cmd_len = data[2];
            if (port < 4 && cmd_len >= 1 && cmd_len <= 8) {
                joybus_itf_queue_raw_cmd(port, &data[3], cmd_len);
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
                tud_vendor_n_write(0, _webusb_out_buffer, 64);
                tud_vendor_n_flush(0);
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