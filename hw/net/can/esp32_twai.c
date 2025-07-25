/*
 * ESP32 TWAI (Two-Wire Automotive Interface) emulation
 *
 * Copyright (c) 2025 Espressif Systems (Shanghai) Co. Ltd.
 *
 * The ESP32 TWAI peripheral is a CAN 2.0B controller based on SJA1000.
 * It supports standard frame format (11-bit ID) and extended frame format
 * (29-bit ID) with programmable bit rate up to 1 Mbps.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or
 * (at your option) any later version.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qapi/error.h"
#include "qemu/error-report.h"
#include "hw/net/can/esp32_twai.h"
#include "can_sja1000.h"
#include "qom/object.h"
#include "hw/qdev-properties.h"
#include "migration/vmstate.h"
#include "net/can_emu.h"

/* Device properties */
static Property esp32_twai_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};

/* Migration state description */
static const VMStateDescription vmstate_esp32_twai = {
    .name = TYPE_ESP32_TWAI,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_STRUCT(sja_state, Esp32TWAIState, 0, vmstate_can_sja, CanSJA1000State),
        VMSTATE_UINT32(interrupt_enable, Esp32TWAIState),
        VMSTATE_UINT32(interrupt_state, Esp32TWAIState),
        VMSTATE_END_OF_LIST()
    }
};

static void esp32_twai_reset(Object *obj, ResetType type)
{
    Esp32TWAIState *d = ESP32_TWAI(obj);
    CanSJA1000State *s = &d->sja_state;

    /* Reset underlying SJA1000 hardware to its default state */
    can_sja_hardware_reset(s);
    
    /* Initialize interrupt control registers to their reset values:
     * - Enable Transmit, Receive and Error interrupts by default
     * - Clear any pending interrupt state
     */
    d->interrupt_enable = ESP32_TWAI_INTR_TI | ESP32_TWAI_INTR_RI | ESP32_TWAI_INTR_EI;
    d->interrupt_state = 0;
}

static void esp32_twai_irq_handler(void *opaque, int irq_num, int level)
{
    Esp32TWAIState *d = (Esp32TWAIState *)opaque;

    /* Track the interrupt state from the underlying SJA1000 controller */
    d->interrupt_state = level;
    
    /* Only forward interrupts to the CPU if they are enabled in the mask.
     * The interrupt enable mask defaults to enabled state for basic operation.
     */
    if (d->interrupt_enable != 0) {
        if (level) {
            qemu_irq_raise(d->irq);
        } else {
            qemu_irq_lower(d->irq);
        }
    }
}

/* Memory-mapped I/O read handler for the TWAI peripheral.
 * Maps ESP32 TWAI register accesses to the underlying SJA1000 controller.
 */
static uint64_t esp32_twai_read(void *opaque, hwaddr addr, unsigned int size)
{
    Esp32TWAIState *d = ESP32_TWAI(opaque);
    CanSJA1000State *s = &d->sja_state;
    const uint64_t reg_addr = addr >> 2;

    if ((s->clock & 0x80) && reg_addr == SJA_RMC) {
        /* PeliCAN Mode */
        return s->rxmsg_cnt;
    }

    return can_sja_mem_read(s, reg_addr, size);
}

/* Memory-mapped I/O write handler for the TWAI peripheral.
 * Maps ESP32 TWAI register accesses to the underlying SJA1000 controller.
 */
static void esp32_twai_write(void *opaque, hwaddr addr, uint64_t value,
                            unsigned int size)
{
    Esp32TWAIState *d = ESP32_TWAI(opaque);
    CanSJA1000State *s = &d->sja_state;

    can_sja_mem_write(s, addr>>2, value, size);    
}

static const MemoryRegionOps esp32_twai_ops = {
    .read = esp32_twai_read,
    .write = esp32_twai_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void esp32_twai_init(Object * obj)
{
    Esp32TWAIState *s = ESP32_TWAI(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->iomem, obj, &esp32_twai_ops, s, TYPE_ESP32_TWAI, ESP32_TWAI_MEM_SIZE);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);

    object_property_add_link(obj, "canbus", TYPE_CAN_BUS,
                             (Object **)&s->canbus,
                             qdev_prop_allow_set_link_before_realize,
                             0);    
}

static void esp32_twai_realize(DeviceState *d, Error **errp)
{
    Esp32TWAIState *s = ESP32_TWAI(d);

    /* Allocate interrupt proxy handler */
    s->irq_handler = qemu_allocate_irq(esp32_twai_irq_handler, s, 0);

    /* Initialize SJA1000 with our interrupt handler */
    can_sja_init(&s->sja_state, s->irq_handler);

    if (can_sja_connect_to_bus(&s->sja_state, s->canbus) < 0) {
        error_setg(errp, "TWAI can_sja_connect_to_bus failed");
        return;
    }
}

static void esp32_twai_class_init(ObjectClass * klass, void * data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    ResettableClass *rc = RESETTABLE_CLASS(klass);
    
    rc->phases.hold = esp32_twai_reset;
    dc->realize = esp32_twai_realize;
    device_class_set_props(dc, esp32_twai_properties);
    dc->vmsd = &vmstate_esp32_twai;
}

static const TypeInfo esp32_twai_type_info = {
    .name = TYPE_ESP32_TWAI,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(Esp32TWAIState),
    .instance_init = esp32_twai_init,
    .class_init = esp32_twai_class_init,
};

static void esp32_twai_register_types(void)
{
    type_register_static(&esp32_twai_type_info);
}

type_init(esp32_twai_register_types)