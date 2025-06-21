# 使用命令

```
sudo ip link add dev vcan0 type vcan
sudo ip link set vcan0 mtu 16
sudo ip link set up vcan0

ninja -C build
```