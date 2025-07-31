# ESP32 Family TWAI Controllers in QEMU

This document covers QEMU emulation of ESP32, ESP32-C3, and ESP32-S3 TWAI (Two-Wire Automotive Interface) controllers.

## Overview

All ESP32 family TWAI controllers are based on the SJA1000 CAN controller but have different hardware characteristics:

- **ESP32**: Supports both BasicCAN and PeliCAN modes, defaults to BasicCAN
- **ESP32-C3**: Hardware defaults to PeliCAN mode only, no BasicCAN support  
- **ESP32-S3**: Hardware defaults to PeliCAN mode only, no BasicCAN support

## Implementation Architecture

- `esp32_twai.c` - Base class implementing ESP32 TWAI functionality
- `esp32c3_twai.c` - ESP32-C3 specific implementation inheriting from ESP32
- `esp32s3_twai.c` - ESP32-S3 specific implementation inheriting from ESP32

Key technical details:
- All variants use `addr >> 2` address mapping to SJA1000 registers
- ESP32-C3/S3 force PeliCAN mode (clock = 0x80) at device creation and reset
- Proper QOM inheritance pattern with parent class method preservation

## 1. 配置阶段（Configure）

进入QEMU源代码目录：

```
./configure --target-list=riscv32-softmmu \
  --enable-gcrypt \
  --enable-slirp \
  --enable-debug \
  --enable-sdl \
  --disable-strip --disable-user \
  --disable-capstone --disable-vnc \
  --disable-gtk
```

检查输出无误。

## 2. 编译阶段（Build with Ninja）

```
ninja -C build
```

验证：`./qemu-system-riscv32 --version`。

## 3. 运行阶段（Run）

若显示can接口没打开，记得 `sudo ip link set can0 up type can bitrate 500000 dbitrate 1000000 fd on` 开启

运行后`output/flash_image_esp32c3.bin`这个固件会持续发送can帧。在运行前需要通过`candump can0`检查是否有can报文输出。

## ESP32-C3运行命令：

```
./build/qemu-system-riscv32 -nographic \
  -machine esp32c3 \
  -drive file=output/flash_image_esp32c3.bin,if=mtd,format=raw \
  -object can-bus,id=canbus0 \
  -object can-host-socketcan,id=canhost0,if=can0,canbus=canbus0 \
  -global driver=esp32c3.twai,property=canbus,value=canbus0 \
  -d guest_errors,unimp \
  -D qemu.log
```

## ESP32运行命令（参考）：

```
./output/qemu-system-xtensa -nographic \
  -machine esp32 \
  -drive file=output/flash_image_esp32.bin,if=mtd,format=raw \
  -bios pc-bios/esp32-v3-rom.bin \
  -object can-bus,id=canbus0 \
  -object can-host-socketcan,id=canhost0,if=can0,canbus=canbus0 \
  -global driver=esp32.twai,property=canbus,value=canbus0 \
  -d guest_errors,unimp \
  -D qemu.log
```

## 重要技术说明：

- ESP32寄存器默认写uint32，相对于SJA1000有地址偏移（需要/4映射）
- **关键修复**: ESP32-C3 TWAI必须使用和ESP32相同的地址映射(`addr >> 2`)，不可直接映射
- **ESP32-C3 PeliCAN模式修复**: ESP32-C3硬件默认PeliCAN模式且不支持BasicCAN，需要在设备创建和重置时强制设置PeliCAN模式(clock = 0x80)
- ESP32, ESP32-C3, ESP32-S3 TWAI都能正常工作，成功连续发送CAN消息到真实can0接口
- 固件文件：`output/flash_image_esp32c3.bin` 和 `output/flash_image_esp32.bin`
- **状态**: 所有ESP32 family TWAI控制器现在都支持连续多帧发送和接收 ✅
- **ESP32-C3 接收功能**: ✅ 已修复CAN消息接收功能，通过`cansend can0 123#AABBCCDD`验证成功
- **关键修复**: 修正了mask filter配置时序，确保在`twai_node_enable()`之前配置过滤器

（可选GDB：添加`-S -gdb tcp::1234`）。
