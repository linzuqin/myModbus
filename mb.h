#ifndef _MB_H_
#define _MB_H_
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "mb_crc.h"
#include "mb_map.h"

#define MB_MAX_SIZE        256
#define MB_MIN_SIZE        4

#define MB_SLAVE_NUM        3

// Modbus 帧结构定义
#define MB_ADDR_BIT          0                  // modbus帧中地址对应的bit位
#define MB_FUNC_BIT          1                  // modbus帧中功能码对应的bit位
#define MB_REGH_ADDR_BIT     2                  // modbus帧中寄存器起始地址高位对应的bit位
#define MB_REGL_ADDR_BIT     3                  // modbus帧中寄存器起始地址低位对应的bit位
#define MB_REGH_COUNT_BIT    4                  // 寄存器数量高位
#define MB_REGL_COUNT_BIT    5                  // 寄存器数量低位
#define MB_BYTE_COUNT_BIT    6                  // 字节数

// 错误码定义
#define MB_EXCEPTION_ILLEGAL_FUNCTION      0x01
#define MB_EXCEPTION_ILLEGAL_DATA_ADDRESS  0x02
#define MB_EXCEPTION_ILLEGAL_DATA_VALUE    0x03
#define MB_EXCEPTION_SLAVE_DEVICE_FAILURE  0x04

typedef enum
{
    MB_MASTER = 0,
    MB_SLAVE,
} mb_dev_type_t;

typedef enum {
    MB_FUNC_READ_COILS = 0x01,
    MB_FUNC_READ_DISCRETE = 0x02,
    MB_FUNC_READ_HOLDING = 0x03,
    MB_FUNC_READ_INPUT = 0x04,
    MB_FUNC_WRITE_SINGLE_COIL = 0x05,
    MB_FUNC_WRITE_SINGLE_REGISTER = 0x06,
    MB_FUNC_WRITE_MULTIPLE_COILS = 0x0F,
    MB_FUNC_WRITE_MULTIPLE_REGISTERS = 0x10
} mb_func_code_t;

typedef enum
{
    MB_OK = 0,
    MB_ERR_SIZE,
    MB_ERR_CRC,
    MB_ERR_ADDR,
    MB_ERR_FUNC,
    MB_ERR_DATA,
    MB_ERR_DEVICE,
    MB_ERR_TIMEOUT,
    MB_ERR_BUILD,
    MB_ERR_MEMORY,

} mb_err_t;


typedef struct
{
    const char *identifier;
    uint8_t addr;
    mb_dev_type_t type;
    // mb_uart_t uart_dev;
    
    // 寄存器存储
    uint8_t *mb_coil_reg;           // 线圈寄存器 (位操作)
    uint8_t *mb_disc_reg;           // 离散量寄存器 (位操作)
    uint16_t *mb_hold_reg;          // 保持寄存器 (16位)
    uint16_t *mb_input_reg;         // 输入寄存器 (16位)
    
    uint16_t coil_size;
    uint16_t disc_size;
    uint16_t hold_size;
    uint16_t input_size;

    uint16_t start_addr;
    // 写入回调函数
    void (*coil_write_cb)(uint16_t addr, uint16_t val);
    void (*hold_write_cb)(uint16_t addr, uint16_t val);
    
    // 统计信息
    uint32_t rx_count;
    uint32_t tx_count;
    uint32_t error_count;

    uint16_t timeout;
    uint8_t *rx_buffer;
    uint16_t rx_size;

    void (*send_callback)(uint8_t *buf, uint16_t len);
} mb_dev_t;


mb_err_t mb_dev_init(mb_dev_t *dev, uint8_t addr, mb_dev_type_t type, void (*send_cb)(uint8_t *, uint16_t));

#endif /* _MB_H_ */
