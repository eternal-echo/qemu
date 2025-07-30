# ESP32C3的QEMU配置和操作文档
[qemu-MR处理.plan](../can/qemu-MR处理.plan.md)

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

运行后固件会持续发送can帧。在运行前需要通过`candump can0`检查是否有can报文输出。

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
- ESP32和ESP32-C3 TWAI都能正常工作，成功发送CAN消息到真实can0接口
- 固件在output目录下：`output/flash_image_esp32c3.bin` 和 `output/flash_image_esp32.bin`
- ESP32能连续发送多帧，ESP32-C3目前只能发送第一帧（中断使能寄存器问题待解决）

## 中断调试命令：

编译时添加SJA1000中断日志：
```bash
ninja -C build -j1
```

检查中断使能寄存器操作：
```bash
grep "SJA1000-IER" qemu.log
```

（可选GDB：添加`-S -gdb tcp::1234`）。
