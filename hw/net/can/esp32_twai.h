#ifndef ESP32_TWAI_H
#define ESP32_TWAI_H

#include "hw/sysbus.h"
#include "net/can_emu.h"
#include "hw/irq.h"
#include "../../../hw/net/can/can_sja1000.h"

#define TYPE_ESP32_TWAI "esp32.twai"
#define ESP32_TWAI(obj) OBJECT_CHECK(Esp32TWAIState, (obj), TYPE_ESP32_TWAI)

#define ESP32_TWAI_MEM_SIZE CAN_SJA_MEM_SIZE

/* ESP32 TWAI interrupt control definitions */
#define ESP32_TWAI_INTR_TI    (0x1 << 1)    /* Transmit Interrupt */
#define ESP32_TWAI_INTR_RI    (0x1 << 0)    /* Receive Interrupt */
#define ESP32_TWAI_INTR_EI    (0x1 << 2)    /* Error Interrupt */

typedef struct Esp32TWAIState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    CanSJA1000State sja_state;
    qemu_irq        irq;            /* System bus IRQ */
    qemu_irq        irq_handler;    /* Interrupt proxy handler */
    uint32_t        interrupt_enable;   /* Interrupt enable control */
    uint32_t        interrupt_state;    /* Current interrupt state */
    CanBusState     *canbus;
} Esp32TWAIState;

#endif