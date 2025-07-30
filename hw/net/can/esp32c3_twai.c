/*
 * ESP32-C3 TWAI (Two-Wire Automotive Interface) emulation
 *
 * Copyright (c) 2025 Espressif Systems (Shanghai) Co. Ltd.
 *
 * The ESP32-C3 TWAI peripheral is identical to ESP32 TWAI controller.
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
#include "hw/net/can/esp32c3_twai.h"
#include "hw/net/can/can_sja1000.h"

/* ESP32C3 TWAI 读取函数 - ESP32-C3使用和ESP32相同的地址映射 */
static uint64_t esp32c3_twai_read(void *opaque, hwaddr addr, unsigned int size)
{
    Esp32C3TWAIState *s = ESP32C3_TWAI(opaque);
    /* ESP32-C3 TWAI寄存器按4字节对齐，需要除4映射到SJA1000寄存器地址 */
    const uint64_t sja_addr = addr >> 2;

    qemu_log("[ESP32C3-TWAI] READ addr=0x%02" HWADDR_PRIx " sja_addr=0x%02" PRIx64 " size=%u\n", addr, sja_addr, size);

    uint64_t result = can_sja_mem_read(&s->parent.sja_state, sja_addr, size);
    
    qemu_log("[ESP32C3-TWAI] READ addr=0x%02" HWADDR_PRIx " result=0x%02" PRIx64 "\n", addr, result);
    
    return result;
}

/* ESP32C3 TWAI 写入函数 - ESP32-C3使用和ESP32相同的地址映射 */
static void esp32c3_twai_write(void *opaque, hwaddr addr, uint64_t value,
                              unsigned int size)
{
    Esp32C3TWAIState *s = ESP32C3_TWAI(opaque);
    /* ESP32-C3 TWAI寄存器按4字节对齐，需要除4映射到SJA1000寄存器地址 */
    const uint64_t sja_addr = addr >> 2;
    
    if (sja_addr == SJA_CDR) {
        value |= 0x80;
        qemu_log("[ESP32C3-TWAI] Intercepting CDR write. Forcing PeliCAN mode (value=0x%02" PRIx64 ").\n", value);
    }
    
    qemu_log("[ESP32C3-TWAI] WRITE addr=0x%02" HWADDR_PRIx " sja_addr=0x%02" PRIx64 " value=0x%02" PRIx64 " size=%u\n", addr, sja_addr, value, size);
    
    can_sja_mem_write(&s->parent.sja_state, sja_addr, value, size);
}

/* ESP32C3 TWAI 设备实现函数 */
static void esp32c3_twai_realize(DeviceState *dev, Error **errp)
{
    Esp32C3TWAIClass *esp32c3_class = ESP32C3_TWAI_GET_CLASS(dev);
    Esp32TWAIState *s = ESP32_TWAI(dev);

    /* 调用父类的realize函数 */
    esp32c3_class->parent_realize(dev, errp);

    /* ESP32-C3硬件默认PeliCAN模式且不支持配置，强制设置SJA1000为PeliCAN模式 */
    s->sja_state.clock = 0x80;  /* 设置PeliCAN模式标志 */
    qemu_log("[ESP32C3-TWAI] ESP32-C3 hardware defaults to PeliCAN mode - clock=0x%02x\n", 
             s->sja_state.clock);
}

/* ESP32C3 TWAI 重置函数 - 确保始终保持PeliCAN模式 */
static void esp32c3_twai_reset(Object *obj, ResetType type)
{
    Esp32C3TWAIState *s = ESP32C3_TWAI(obj);
    Esp32C3TWAIClass *esp32c3_class = ESP32C3_TWAI_GET_CLASS(obj);
    
    /* 调用父类的reset函数 */
    esp32c3_class->parent_reset(obj, type);
    
    /* ESP32-C3硬件始终在PeliCAN模式，重置后需要重新设置 */
    s->parent.sja_state.clock = 0x80;
    qemu_log("[ESP32C3-TWAI] After reset: forcing PeliCAN mode - clock=0x%02x\n", 
             s->parent.sja_state.clock);
}

/* ESP32C3 TWAI 实例初始化 */
static void esp32c3_twai_init(Object *obj)
{
    /* QOM会自动调用父类的init函数，所以这里不需要做额外工作 */
}

/* ESP32C3 TWAI 类初始化 */
static void esp32c3_twai_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    ResettableClass *rc = RESETTABLE_CLASS(klass);
    Esp32C3TWAIClass *esp32c3_class = ESP32C3_TWAI_CLASS(klass);
    Esp32TWAIClass *esp32_parent_class = ESP32_TWAI_CLASS(klass);

    /* 保存父类的realize函数，然后设置我们自己的realize函数 */
    device_class_set_parent_realize(dc, esp32c3_twai_realize, &esp32c3_class->parent_realize);

    /* 保存父类的reset函数，然后设置我们自己的reset函数 */
    esp32c3_class->parent_reset = rc->phases.hold;
    rc->phases.hold = esp32c3_twai_reset;

    /* 保存父类的虚函数指针 */
    esp32c3_class->parent_twai_write = esp32_parent_class->twai_write;
    esp32c3_class->parent_twai_read = esp32_parent_class->twai_read;
    
    /* 设置本类的虚函数 - 不要修改父类！ */
    esp32_parent_class->twai_write = esp32c3_twai_write;
    esp32_parent_class->twai_read = esp32c3_twai_read;
}

/* 类型信息 */
static const TypeInfo esp32c3_twai_type_info = {
    .name = TYPE_ESP32C3_TWAI,
    .parent = TYPE_ESP32_TWAI,  /* 继承自ESP32_TWAI */
    .instance_size = sizeof(Esp32C3TWAIState),
    .instance_init = esp32c3_twai_init,
    .class_size = sizeof(Esp32C3TWAIClass),
    .class_init = esp32c3_twai_class_init,
};

/* 注册类型 */
static void esp32c3_twai_register_types(void)
{
    type_register_static(&esp32c3_twai_type_info);
}

type_init(esp32c3_twai_register_types)