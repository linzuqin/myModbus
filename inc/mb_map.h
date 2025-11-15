#ifndef MB_MAP_H
#define MB_MAP_H

// 寄存器大小定义
#define MB_COIL_REG_SIZE        125            // 线圈寄存器大小
#define MB_DISC_REG_SIZE        125            // 离散量寄存器大小
#define MB_HOLD_REG_SIZE        125             // 保持寄存器大小
#define MB_INPUT_REG_SIZE       125             // 输入寄存器大小

// Modbus 帧结构定义
#define MB_ADDR_BIT          0                  // modbus帧中地址对应的bit位
#define MB_FUNC_BIT          1                  // modbus帧中功能码对应的bit位
#define MB_REGH_ADDR_BIT     2                  // modbus帧中寄存器起始地址高位对应的bit位
#define MB_REGL_ADDR_BIT     3                  // modbus帧中寄存器起始地址低位对应的bit位
#define MB_REGH_COUNT_BIT    4                  // 寄存器数量高位
#define MB_REGL_COUNT_BIT    5                  // 寄存器数量低位
#define MB_BYTE_COUNT_BIT    6                  // 字节数










#endif /* MB_MAP_H */
