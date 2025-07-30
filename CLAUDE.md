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

运行后`firmware/flash_image_esp32c3.bin`这个固件会持续发送can帧。在运行前需要通过`candump can0`检查是否有can报文输出。

运行：

```
./build/qemu-system-riscv32 -nographic \
  -machine esp32c3 \
  -drive file=firmware/flash_image_esp32c3.bin,if=mtd,format=raw \
  -object can-bus,id=canbus0 \
  -object can-host-socketcan,id=canhost0,if=can0,canbus=canbus0 \
  -global driver=esp32c3.twai,property=canbus,value=canbus0 \
  -d guest_errors,unimp \
  -D qemu.log
```

（可选GDB：添加`-S -gdb tcp::1234`）。
