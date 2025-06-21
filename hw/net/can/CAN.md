# Kvaser PCI CAN设备中SJA1000的中断是如何关联和处理的。

- 流程图：
    
    ```mermaid
    sequenceDiagram
        participant Guest as Guest OS/Driver
        participant PCI as PCI Bus
        participant Kvaser as KvaserPCI Device
        participant CAN as CAN Bus
        participant Backend as CAN Backend
    
        Note over Guest,Backend: 设备初始化阶段
        Guest->>PCI: 枚举PCI设备
        PCI->>Kvaser: kvaser_pci_realize()
        activate Kvaser
        Kvaser->>Kvaser: 初始化设备状态
        Kvaser->>PCI: pci_register_bar() 注册内存区域
        Kvaser->>CAN: can_bus_client_new() 创建CAN客户端
        Note right of Kvaser: 设置内存映射I/O区域<br/>配置中断处理器
        Kvaser-->>PCI: 设备就绪
        deactivate Kvaser
    
        Note over Guest,Backend: 设备配置阶段
        Guest->>Kvaser: 写入配置寄存器 (kvaser_pci_write)
        activate Kvaser
        Note right of Kvaser: 配置CAN波特率<br/>设置过滤器<br/>启用中断
        Kvaser->>Kvaser: 更新内部状态
        Kvaser-->>Guest: 配置完成
        deactivate Kvaser
    
        Note over Guest,Backend: CAN帧发送流程
        Guest->>Kvaser: 写入发送缓冲区 (kvaser_pci_write)
        activate Kvaser
        Kvaser->>Kvaser: 解析CAN帧数据
        Note right of Kvaser: 检查发送队列状态<br/>验证帧格式
        Kvaser->>CAN: can_bus_client_send() 发送到CAN总线
        activate CAN
        CAN->>Backend: 转发到后端 (socketcan/文件等)
        activate Backend
        Backend-->>CAN: ACK确认
        deactivate Backend
        CAN-->>Kvaser: 发送完成回调
        deactivate CAN
        Kvaser->>Guest: 触发发送完成中断
        Note right of Guest: 驱动程序处理中断<br/>更新发送状态
        deactivate Kvaser
    
        Note over Guest,Backend: CAN帧接收流程
        Backend->>CAN: 接收到CAN帧
        activate CAN
        CAN->>Kvaser: kvaser_pci_can_receive() 检查接收能力
        activate Kvaser
        Kvaser-->>CAN: 返回可接收状态
        CAN->>Kvaser: kvaser_pci_receive() 传递CAN帧
        Note right of Kvaser: 将帧数据写入接收缓冲区<br/>更新接收计数器
        Kvaser->>Guest: 触发接收中断
        deactivate Kvaser
        deactivate CAN
    
        Guest->>Kvaser: 读取接收缓冲区 (kvaser_pci_read)
        activate Kvaser
        Note right of Kvaser: 从FIFO队列读取帧<br/>更新读指针
        Kvaser-->>Guest: 返回CAN帧数据
        deactivate Kvaser
    
        Note over Guest,Backend: 错误处理流程
        Backend->>CAN: CAN总线错误 (总线关闭/错误帧)
        activate CAN
        CAN->>Kvaser: 错误状态通知
        activate Kvaser
        Kvaser->>Kvaser: 更新错误计数器
        Note right of Kvaser: 设置错误状态位<br/>可能进入总线关闭状态
        Kvaser->>Guest: 触发错误中断
        deactivate Kvaser
        deactivate CAN
    
        Guest->>Kvaser: 读取状态寄存器 (kvaser_pci_read)
        activate Kvaser
        Note right of Kvaser: 返回错误状态<br/>错误计数器值
        Kvaser-->>Guest: 错误状态信息
        deactivate Kvaser
    
        Note over Guest,Backend: 设备复位流程
        Guest->>Kvaser: 写入复位命令 (kvaser_pci_write)
        activate Kvaser
        Kvaser->>Kvaser: kvaser_pci_reset() 复位设备
        Note right of Kvaser: 清空发送/接收队列<br/>复位错误计数器<br/>恢复默认配置
        Kvaser->>CAN: 断开CAN总线连接
        Kvaser-->>Guest: 复位完成
        deactivate Kvaser
    
    ```