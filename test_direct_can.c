#include <stdio.h>
#include <stdint.h>

int main() {
    uint16_t can_id = 0x123;
    
    // SJA1000 11-bit ID encoding: ID needs to be left-shifted by 5 bits
    uint16_t sja_id = can_id << 5;
    uint8_t id0 = (sja_id >> 8) & 0xFF;    // High byte
    uint8_t id1 = sja_id & 0xFF;           // Low byte
    
    printf("CAN ID: 0x%03X\n", can_id);
    printf("SJA1000 encoded: 0x%04X\n", sja_id);
    printf("ID0 (0x11): 0x%02X\n", id0);
    printf("ID1 (0x12): 0x%02X\n", id1);
    
    return 0;
}