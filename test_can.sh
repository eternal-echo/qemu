#!/bin/bash

# Start candump in background
candump can1 &
CANDUMP_PID=$!

# Start QEMU
./build/qemu-system-riscv32 -nographic \
  -machine esp32c3 \
  -drive file=firmware/flash_image_esp32c3.bin,if=mtd,format=raw \
  -object can-bus,id=canbus0 \
  -object can-host-socketcan,id=canhost0,if=can1,canbus=canbus0 \
  -global driver=esp32c3.twai,property=canbus,value=canbus0 \
  -d guest_errors,unimp \
  -D qemu.log &

QEMU_PID=$!

# Wait a bit
sleep 10

# Kill both processes
kill $CANDUMP_PID $QEMU_PID