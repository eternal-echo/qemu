/*
 * ESP32-C3 TWAI (Two-Wire Automotive Interface) emulation
 *
 * Copyright (c) 2025 Espressif Systems (Shanghai) Co. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or
 * (at your option) any later version.
 */
#ifndef ESP32C3_TWAI_H
#define ESP32C3_TWAI_H

#include "hw/net/can/esp32_twai.h"

#define TYPE_ESP32C3_TWAI "esp32c3.twai"
#define ESP32C3_TWAI(obj) OBJECT_CHECK(Esp32C3TWAIState, (obj), TYPE_ESP32C3_TWAI)
#define ESP32C3_TWAI_CLASS(klass) OBJECT_CLASS_CHECK(Esp32C3TWAIClass, klass, TYPE_ESP32C3_TWAI)
#define ESP32C3_TWAI_GET_CLASS(obj) OBJECT_GET_CLASS(Esp32C3TWAIClass, obj, TYPE_ESP32C3_TWAI)

typedef struct Esp32C3TWAIState {
    /* 继承自ESP32TWAIState */
    Esp32TWAIState parent;
    
    /* ESP32-C3特有的寄存器或状态 */
    /* 目前ESP32-C3 TWAI与ESP32 TWAI相同，所以暂时没有额外字段 */
} Esp32C3TWAIState;

typedef struct Esp32C3TWAIClass {
    /* 继承自Esp32TWAIClass */
    Esp32TWAIClass parent_class;

    /* 保存父类的虚函数指针，用于在子类中调用父类方法 */
    DeviceRealize parent_realize;
    void (*parent_twai_write)(void *opaque, hwaddr addr, uint64_t value, unsigned int size);
    uint64_t (*parent_twai_read)(void *opaque, hwaddr addr, unsigned int size);
} Esp32C3TWAIClass;

#endif