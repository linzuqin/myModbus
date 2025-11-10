#ifndef _MB_H_
#define _MB_H_
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "mb_crc.h"
#include "mb_map.h"

#define MB_MAX_SIZE        256
#define MB_MIN_SIZE        4

#define MB_SLAVE_NUM        1
#define MB_MASTER_NUM        1

// 错误码定义
#define MB_EXCEPTION_ILLEGAL_FUNCTION      0x01
#define MB_EXCEPTION_ILLEGAL_DATA_ADDRESS  0x02
#define MB_EXCEPTION_ILLEGAL_DATA_VALUE    0x03
#define MB_EXCEPTION_SLAVE_DEVICE_FAILURE  0x04

// 功能码类型定义
typedef uint8_t mb_func_code_t;

#define MB_FUNC_READ_COILS        ((mb_func_code_t)0x01)
#define MB_FUNC_READ_DISCRETE     ((mb_func_code_t)0x02)
#define MB_FUNC_READ_HOLDING      ((mb_func_code_t)0x03)
#define MB_FUNC_READ_INPUT        ((mb_func_code_t)0x04)
#define MB_FUNC_WRITE_SINGLE_COIL ((mb_func_code_t)0x05)
#define MB_FUNC_WRITE_SINGLE_REGISTER ((mb_func_code_t)0x06)
#define MB_FUNC_WRITE_MULTIPLE_COILS ((mb_func_code_t)0x0F)
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS ((mb_func_code_t)0x10)


typedef enum
{
    MB_MASTER = 0,
    MB_SLAVE,
} mb_dev_type_t;

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


extern uint8_t coil_buf[MB_COIL_REG_SIZE];
extern uint8_t disc_buf[MB_DISC_REG_SIZE];
extern uint16_t keep_buf[MB_HOLD_REG_SIZE];
extern uint16_t input_buf[MB_INPUT_REG_SIZE];

mb_err_t mb_dev_init(mb_dev_t *dev, uint8_t addr, mb_dev_type_t type, void (*send_cb)(uint8_t *, uint16_t));

#endif /* _MB_H_ */
