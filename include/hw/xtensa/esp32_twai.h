#ifndef ESP32_TWAI_H
#define ESP32_TWAI_H

#include "hw/sysbus.h"
#include "net/can_emu.h"
#include "hw/irq.h"
#include "../../../hw/net/can/can_sja1000.h"

#define TYPE_ESP32_TWAI "esp32.twai"
#define Esp32_TWAI(obj) OBJECT_CHECK(Esp32TWAIState, (obj), TYPE_ESP32_TWAI)

#define ESP32_TWAI_MEM_SIZE CAN_SJA_MEM_SIZE

typedef struct Esp32TWAIState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    CanSJA1000State sja_state;
    qemu_irq        irq;
    CanBusState     *canbus;
} Esp32TWAIState;

#endif