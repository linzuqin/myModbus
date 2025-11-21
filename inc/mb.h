/*
 * @Description:定义了与modbus设备相关的参数
 * @Author: linzuqin
 * @Date: 2025-11-10 16:20:49
 * @LastEditTime: 2025-11-19 22:58:29
 * @LastEditors: linzuqin
 */
#ifndef _MB_H_
#define _MB_H_

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


// modbus从机寄存器大小定义
#define MB_S_COIL_SIZE        125            // 线圈寄存器大小
#define MB_S_DISC_SIZE        125            // 离散量寄存器大小
#define MB_S_HOLD_SIZE        125             // 保持寄存器大小
#define MB_S_INPUT_SIZE       125             // 输入寄存器大小

// modbus主机寄存器大小定义
#define MB_M_COIL_SIZE        125            // 线圈寄存器大小
#define MB_M_DISC_SIZE        125            // 离散量寄存器大小
#define MB_M_HOLD_SIZE        125             // 保持寄存器大小
#define MB_M_INPUT_SIZE       125             // 输入寄存器大小

//modbus错误码
typedef enum
{
    PARSE_OK = 0x00,
    Illegal_Function = 0x01,
    Illegal_Data_Address = 0x02,
    Illegal_Data_Value = 0x03,
    Slave_Device_Failure = 0x04,
    Acknowledge = 0x05,
    Slave_Device_Busy = 0x06,
    Negative_Acknowledge = 0x07,
    Memory_Parity_Error = 0x08,
}mb_err_code_t;

//modbus设备类型
typedef enum
{
    MB_MASTER = 0,
    MB_SLAVE,
} mb_dev_type_t;

//modbus函数返回值
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

//这个结构体是主机部分特有 主要是为了能够根据标志位自动执行下发的操作 
typedef struct
{
    uint16_t value;
    uint8_t setFlag;
}mb_m_map;

typedef struct
{
    const char *identifier;
    uint8_t addr;
    mb_dev_type_t type;
    
    // 寄存器存储
    uint8_t *mb_coil_reg;           // 线圈寄存器 (位操作)
    uint8_t *mb_disc_reg;           // 离散量寄存器 (位操作)
    uint16_t *mb_hold_reg;          // 保持寄存器 (16位)
    uint16_t *mb_input_reg;         // 输入寄存器 (16位)

    //每个modbus设备都应该配置读取时的起始地址 和 每个寄存器的读取数量
    uint16_t coil_start_addr;
    uint16_t coil_read_size;

    uint16_t disc_start_addr;
    uint16_t disc_read_size;

    uint16_t hold_start_addr;
    uint16_t hold_read_size;

    uint16_t input_start_addr;
    uint16_t input_read_size;

    // 统计信息
    uint32_t rx_count;
    uint32_t tx_count;
    uint32_t error_count;

    uint16_t timeout;
    uint8_t *rx_buffer;
    uint16_t rx_size;

    void (*send_callback)(uint8_t *buf, uint16_t len);
  
    // 写入回调函数
    void (*coil_write_cb)(uint16_t addr, uint16_t val);
    void (*hold_write_cb)(uint16_t addr, uint16_t val);

    //主机映射表 用于数据的写入
    mb_m_map *coil_map;
    mb_m_map *hold_map;
} mb_dev_t;

uint16_t usMBCRC16( uint8_t * pucFrame, uint16_t usLen );
mb_err_t mb_data_get(void *dev ,uint8_t *data_buf , uint16_t data_len);


#endif /* _MB_H_ */
