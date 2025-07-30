/*
 * ESP32-S3 TWAI (Two-Wire Automotive Interface) emulation
 *
 * Copyright (c) 2025 Espressif Systems (Shanghai) Co. Ltd.
 *
 * The ESP32-S3 TWAI peripheral is identical to ESP32 TWAI controller.
 * It supports standard frame format (11-bit ID) and extended frame format
 * (29-bit ID) with programmable bit rate up to 1 Mbps.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or
 * (at your option) any later version.
 */

#ifndef HW_CAN_ESP32S3_TWAI_H
#define HW_CAN_ESP32S3_TWAI_H

#include "hw/net/can/esp32_twai.h"

struct Esp32S3TWAIState {
    Esp32TWAIState parent;
};

typedef struct Esp32S3TWAIClass {
    Esp32TWAIClass parent_class;
    void (*parent_realize)(DeviceState *dev, Error **errp);
    void (*parent_reset)(Object *obj, ResetType type);
    void (*parent_twai_write)(void *opaque, hwaddr addr, uint64_t value, unsigned int size);
    uint64_t (*parent_twai_read)(void *opaque, hwaddr addr, unsigned int size);
} Esp32S3TWAIClass;

#define TYPE_ESP32S3_TWAI "esp32s3.twai"
#define ESP32S3_TWAI esp32s3_twai
OBJECT_DECLARE_SIMPLE_TYPE(Esp32S3TWAIState, ESP32S3_TWAI)

#define ESP32S3_TWAI_CLASS(klass) OBJECT_CLASS_CHECK(Esp32S3TWAIClass, klass, TYPE_ESP32S3_TWAI)
#define ESP32S3_TWAI_GET_CLASS(obj) OBJECT_GET_CLASS(Esp32S3TWAIClass, obj, TYPE_ESP32S3_TWAI)

#endif /* HW_CAN_ESP32S3_TWAI_H */
