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
#include "hw/hw.h"
#include "hw/qdev-properties.h"
#include "hw/i2c/i2c.h"
#include "hw/i2c/smbus_slave.h"
#include "smbus.h"
#include "qapi/error.h"
#include "smbus_xboxhdmi.h"
#include "smbus_xboxhdmi_tables.h"

#define TYPE_SMBUS_XBOXHDMI "smbus-xboxhdmi"
#define SMBUS_XBOXHDMI(obj) OBJECT_CHECK(SMBusXboxHDMIDevice, (obj), TYPE_SMBUS_XBOXHDMI)

#define DEBUG
#ifdef DEBUG
# define DPRINTF(format, ...) printf(format, ## __VA_ARGS__)
#else
# define DPRINTF(format, ...) do { } while (0)
#endif

typedef struct SMBusXboxHDMIDevice {
    SMBusDevice smbusdev;
    SMBusXboxHDMIRegisters registers; // https://github.com/MakeMHz/xbox-hdmi-app/blob/master/source/i2c_map.h
    uint8_t cmd;
} SMBusXboxHDMIDevice;

// HACK
extern uint8_t  *cAvpRegister;
extern uint8_t  *cAvpCRTCRegister;
extern uint32_t *cAvpFpDebug0;

// TODO: optional?
static void smbus_xboxhdmi_quick_cmd(SMBusDevice *dev, uint8_t read)
{
    DPRINTF("%s: addr=0x%02x read=%d\n", __func__, dev->i2c.address, read);
}

static int smbus_xboxhdmi_write_data(SMBusDevice *dev, uint8_t *buf, uint8_t len)
{
    SMBusXboxHDMIDevice *cx = SMBUS_XBOXHDMI(dev);

    cx->cmd = buf[0];
    uint8_t cmd = cx->cmd;
    buf++;
    len--;

    if (len < 1) return 0;

    // The XboxHDMI processes each byte of the transfer so lets do the same here
    for(uint8_t index = 0; index < len; index++) {
        DPRINTF("%s: addr=0x%02X cmd=0x%02X val=0x%02X\n",
                __func__, dev->i2c.address, cmd + index, buf[index]);

        // Update register
        ((uint8_t *)&cx->registers)[cmd + index] = buf[index];

        // Check if video mode is being updated
        if(cmd == XBOXHDMI_REG_INDEX_BY_NAME(video_mode_post) + 3) {
            DPRINTF("%s: Updating timing tables\n", __func__);

            uint8_t iCRTC     = (cx->registers.video_mode_post & 0x0000FF00) >> 8;
            uint8_t iRegister = (cx->registers.video_mode_post & 0x00FF0000) >> 16;

            cAvpCRTCRegister = (uint8_t *)&AvpCRTCRegisters[iCRTC - 1];
            cAvpRegister     = (uint8_t *)&AvpRegisters[iRegister - 1];
            cAvpFpDebug0     = (uint32_t *)&AvpFpDebug0[iRegister - 1];
        }
    }

    return 0;
}

static uint8_t smbus_xboxhdmi_receive_byte(SMBusDevice *dev)
{
    SMBusXboxHDMIDevice *cx = SMBUS_XBOXHDMI(dev);

    // TODO: command handler
    uint8_t val = ((uint8_t *)&cx->registers)[cx->cmd];

    DPRINTF("%s: addr=0x%02x cmd=0x%02X val=0x%02X\n",
            __func__, dev->i2c.address, cx->cmd, val);

    cx->cmd++;
    return val;
}

static void smbus_xboxhdmi_realize(DeviceState *dev, Error **errp)
{
    SMBusXboxHDMIDevice *cx = SMBUS_XBOXHDMI(dev);

    // Clear memory
    memset(&cx->registers, 0, sizeof(SMBusXboxHDMIRegisters));

    // Set initial register values
    cx->registers.firmware_version[0] = 1;
    cx->registers.firmware_version[1] = 0;
    cx->registers.firmware_version[2] = 2;

    cx->cmd = 0;
}

static void smbus_xboxhdmi_class_initfn(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    SMBusDeviceClass *sc = SMBUS_DEVICE_CLASS(klass);

    dc->realize = smbus_xboxhdmi_realize;
    sc->quick_cmd = smbus_xboxhdmi_quick_cmd;   // TODO: optional?
    sc->receive_byte = smbus_xboxhdmi_receive_byte;
    sc->write_data = smbus_xboxhdmi_write_data;
}

static TypeInfo smbus_xboxhdmi_info = {
    .name = TYPE_SMBUS_XBOXHDMI,
    .parent = TYPE_SMBUS_DEVICE,
    .instance_size = sizeof(SMBusXboxHDMIDevice),
    .class_init = smbus_xboxhdmi_class_initfn,
};

static void smbus_xboxhdmi_register_devices(void)
{
    type_register_static(&smbus_xboxhdmi_info);
}

type_init(smbus_xboxhdmi_register_devices)

void smbus_xboxhdmi_init(I2CBus *smbus, int address)
{
    DeviceState *dev;
    dev = qdev_new(TYPE_SMBUS_XBOXHDMI);
    qdev_prop_set_uint8(dev, "address", address);
    qdev_realize_and_unref(dev, (BusState *)smbus, &error_fatal);
}
