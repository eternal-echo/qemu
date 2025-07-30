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

/* ESP32C3 TWAI 读取函数 - 正确的继承实现 */
static uint64_t esp32c3_twai_read(void *opaque, hwaddr addr, unsigned int size)
{
    Esp32C3TWAIClass *class = ESP32C3_TWAI_GET_CLASS(opaque);
    uint64_t result;

    qemu_log_mask(LOG_TRACE, "ESP32C3_TWAI: READ addr=0x%lx reg_addr=0x%lx size=%d\n", addr, addr >> 2, size);

    // 调用父类方法，传递原始opaque指针
    result = class->parent_twai_read(opaque, addr, size);
    
    qemu_log_mask(LOG_TRACE, "ESP32C3_TWAI: READ addr=0x%lx reg_addr=0x%lx size=%d result=0x%lx\n", addr, addr >> 2, size, result);
    
    return result;
}

/* ESP32C3 TWAI 写入函数 - 正确的继承实现 */
static void esp32c3_twai_write(void *opaque, hwaddr addr, uint64_t value,
                              unsigned int size)
{
    Esp32C3TWAIClass *class = ESP32C3_TWAI_GET_CLASS(opaque);
    
    qemu_log_mask(LOG_TRACE, "ESP32C3_TWAI: WRITE addr=0x%lx reg_addr=0x%lx value=0x%lx size=%d\n", addr, addr >> 2, value, size);
    
    // 调用父类方法，传递原始opaque指针
    class->parent_twai_write(opaque, addr, value, size);
}

/* ESP32C3 TWAI 设备实现函数 */
static void esp32c3_twai_realize(DeviceState *dev, Error **errp)
{
    Esp32C3TWAIClass *esp32c3_class = ESP32C3_TWAI_GET_CLASS(dev);
    Esp32TWAIState *s = ESP32_TWAI(dev);

    /* 调用父类的realize函数 */
    esp32c3_class->parent_realize(dev, errp);

    /* Force PeliCAN mode by default for ESP32-C3 */
    s->sja_state.clock |= 0x80;
    qemu_log("[ESP32C3-TWAI] Forcing PeliCAN mode by default.\n");
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
    Esp32C3TWAIClass *esp32c3_class = ESP32C3_TWAI_CLASS(klass);
    Esp32TWAIClass *esp32_parent_class = ESP32_TWAI_CLASS(klass);

    /* 保存父类的realize函数，然后设置我们自己的realize函数 */
    device_class_set_parent_realize(dc, esp32c3_twai_realize, &esp32c3_class->parent_realize);

    /* 保存父类的虚函数指针 */
    esp32c3_class->parent_twai_write = esp32_parent_class->twai_write;
    esp32c3_class->parent_twai_read = esp32_parent_class->twai_read;
    
    /* 重写虚函数 - 指向我们自己的实现 */
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