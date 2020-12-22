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
#include "smbus_xboxhdmi_tables.h"

#define TYPE_SMBUS_XBOXHDMI_TIMING "smbus-xboxhdmi_timing"
#define SMBUS_XBOXHDMI_TIMING(obj) OBJECT_CHECK(SMBusXboxHDMITimingDevice, (obj), TYPE_SMBUS_XBOXHDMI_TIMING)

// HACK
uint8_t hdmi_timing_registers[256] = { 0 };

#define DEBUG
#ifdef DEBUG
# define DPRINTF(format, ...) printf(format, ## __VA_ARGS__)
#else
# define DPRINTF(format, ...) do { } while (0)
#endif

typedef struct SMBusXboxHDMITimingDevice {
    SMBusDevice smbusdev;
    uint8_t cmd;
} SMBusXboxHDMITimingDevice;

const uint32_t *cAvpRegister;
uint8_t *cAvpCRTCRegister;
uint32_t *cAvpFpDebug0;

// TODO: optional?
static void smbus_xboxhdmi_timing_quick_cmd(SMBusDevice *dev, uint8_t read)
{
    DPRINTF("%s: addr=0x%02x read=%d\n", __func__, dev->i2c.address, read);
}

static int smbus_xboxhdmi_timing_write_data(SMBusDevice *dev, uint8_t *buf, uint8_t len)
{
    SMBusXboxHDMITimingDevice *cx = SMBUS_XBOXHDMI_TIMING(dev);

    cx->cmd = buf[0];
    uint8_t cmd = cx->cmd;
    buf++;
    len--;

    if (len < 1) return 0;

    DPRINTF("%s: addr=0x%02X cmd=0x%02X val=0x%02X\n",
            __func__, dev->i2c.address, cmd, buf[0]);

    return 0;
}

static uint8_t smbus_xboxhdmi_timing_receive_byte(SMBusDevice *dev)
{
    SMBusXboxHDMITimingDevice *cx = SMBUS_XBOXHDMI_TIMING(dev);

    // TODO: command handler
    uint8_t val = 0;

    // AvpRegisters
    if(cx->cmd < 104) {
        val = ((uint8_t *)cAvpRegister)[cx->cmd];
    // AvpCRTCRegisters
    } else if(cx->cmd < 138) {
        val = ((uint8_t *)cAvpCRTCRegister)[cx->cmd - 104];
    // AvpFpDebug0
    } else if(cx->cmd < 142) {
        val = ((uint8_t *)cAvpFpDebug0)[cx->cmd - 138];
    }

    DPRINTF("%s: addr=0x%02x cmd=0x%02X val=0x%02X\n",
            __func__, dev->i2c.address, cx->cmd, val);

    cx->cmd++;
    return val;
}

static void smbus_xboxhdmi_timing_realize(DeviceState *dev, Error **errp)
{
    SMBusXboxHDMITimingDevice *cx = SMBUS_XBOXHDMI_TIMING(dev);

    // Set default state of video registers
	cAvpRegister     = &AvpRegisters[0];
	cAvpCRTCRegister = &AvpCRTCRegisters[7];
	cAvpFpDebug0     = &AvpFpDebug0[0];

    cx->cmd = 0;
}

static void smbus_xboxhdmi_timing_class_initfn(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    SMBusDeviceClass *sc = SMBUS_DEVICE_CLASS(klass);

    dc->realize = smbus_xboxhdmi_timing_realize;
    sc->quick_cmd = smbus_xboxhdmi_timing_quick_cmd;   // TODO: optional?
    sc->receive_byte = smbus_xboxhdmi_timing_receive_byte;
    sc->write_data = smbus_xboxhdmi_timing_write_data;
}

static TypeInfo smbus_xboxhdmi_timing_info = {
    .name = TYPE_SMBUS_XBOXHDMI_TIMING,
    .parent = TYPE_SMBUS_DEVICE,
    .instance_size = sizeof(SMBusXboxHDMITimingDevice),
    .class_init = smbus_xboxhdmi_timing_class_initfn,
};

static void smbus_xboxhdmi_timing_register_devices(void)
{
    type_register_static(&smbus_xboxhdmi_timing_info);
}

type_init(smbus_xboxhdmi_timing_register_devices)

void smbus_xboxhdmi_timing_init(I2CBus *smbus, int address)
{
    DeviceState *dev;
    dev = qdev_new(TYPE_SMBUS_XBOXHDMI_TIMING);
    qdev_prop_set_uint8(dev, "address", address);
    qdev_realize_and_unref(dev, (BusState *)smbus, &error_fatal);
}
