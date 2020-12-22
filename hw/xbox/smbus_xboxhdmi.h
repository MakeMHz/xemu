/*
 * QEMU SMBus MakeMHz XboxHDMI Device
 *
 * Copyright (c) 2020 Mike Davis
 * Copyright (c) 2020 Dustin Holden
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"

#ifndef HW_SMBUS_XBOXHDMI_H
#define HW_SMBUS_XBOXHDMI_H

// Largely copied from
// https://github.com/MakeMHz/xbox-hdmi-mcu/blob/master/Common/include/i2c_map.h
typedef struct SMBusXboxHDMIRegisters {
    /* 0x00 RW */ uint8_t  scratch[0x20];
    /* 0x20 RW */ uint8_t  boot_mode;
    /* 0x21 RW */ uint32_t boot_flag;
    /* 0x25 RW */ uint32_t compile_time;
    /* 0x29 RW */ uint32_t load_app;
    /* 0x2D RW */ uint8_t  prog_mode;
    /* 0x2E RW */ uint8_t  prog_write;  // Page number for programming
    /* 0x2F RW */ uint32_t prog_data;  // Data for programming
    /* 0x33 RW */ uint8_t  prog_page;   // Current flash page
    /* 0x34 RO */ uint16_t prog_pos;
    /* 0x36 RO */ uint8_t  prog_full;
    /* 0x37 RO */ uint8_t  prog_busy;
    /* 0x38 RO */ uint32_t prog_crc;
    /* 0x3C RO */ uint8_t  prog_error;
    /* 0x3C RO */ uint8_t  reservered;
    /* 0x3E RW */ uint32_t video_mode_pre;
    /* 0x42 RW */ uint32_t video_mode_post;
    /* 0x46 RW */ uint32_t video_mode_rev;
    /* 0x4A RW */ uint8_t  eeprom_save;
    /* 0x4B RW */ uint8_t  eeprom_widescreen;
    /* 0x4C RW */ uint8_t  eeprom_mode_out;
    /* 0x4D RW */ int8_t   eeprom_adjust_luma;
    /* 0x4E RW */ int8_t   eeprom_adjust_cb;
    /* 0x4F RW */ uint8_t  eeprom_adjust_cr;
    /* 0x50 RW */ uint8_t  xbox_encoder_smbus;
    /* 0x51 RW */ uint8_t  xbox_region;
    /* 0x52 RW */ uint32_t xbox_game_titleid;
    /* 0x56 RW */ uint8_t  video_table_page;
    /* 0x57 RW */ uint8_t  firmware_version[3];

    uint8_t scratch2[0xBD];
} __attribute__((packed)) SMBusXboxHDMIRegisters;

#define XBOXHDMI_REG_INDEX_BY_NAME(name) (unsigned int)offsetof(SMBusXboxHDMIRegisters, name)

#endif
