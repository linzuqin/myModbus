/*
 * @Description:定义了modbus帧格式结构体 用于解析数据 
 * @Author: linzuqin
 * @Date: 2025-11-10 16:20:49
 * @LastEditTime: 2025-11-19 22:51:59
 * @LastEditors: linzuqin
 */

#ifndef MB_MAP_H
#define MB_MAP_H

#include "mb.h"

// Modbus 帧结构定义
#define MB_ADDR_BIT          0                  // modbus帧中地址对应的bit位
#define MB_FUNC_BIT          1                  // modbus帧中功能码对应的bit位
#define MB_REGH_ADDR_BIT     2                  // modbus帧中寄存器起始地址高位对应的bit位
#define MB_REGL_ADDR_BIT     3                  // modbus帧中寄存器起始地址低位对应的bit位
#define MB_REGH_COUNT_BIT    4                  // 寄存器数量高位
#define MB_REGL_COUNT_BIT    5                  // 寄存器数量低位
#define MB_BYTE_COUNT_BIT    6                  // 字节数


//主机操作帧结构体
typedef struct
{
    uint8_t dev_addr;
    mb_func_code_t fun_code;
    uint8_t reg_addr_h;
    uint8_t reg_addr_l;
}mb_payload_head;

typedef union {
    struct {  // 读取寄存器操作
        uint8_t quantity_h;
        uint8_t quantity_l;

        uint8_t crc_h;
        uint8_t crc_l;

    } r_regs;
    
    struct {  // 写入单个寄存器操作
        uint8_t value_h;
        uint8_t value_l;

        uint8_t crc_h;
        uint8_t crc_l;
    } w_reg;

    struct {  // 写入多个寄存器操作
        uint8_t quantity_h;
        uint8_t quantity_l;
        uint8_t byte_count;
        uint8_t payload[248];//包含校验位

    } w_regs;
}mb_payload;

//从机应答帧结构体
typedef struct
{
    uint8_t dev_addr;
    mb_func_code_t fun_code;

}mb_resp_head;

typedef union {
    struct {  // 读取寄存器的应答
        uint8_t byte_count;
        uint8_t data[248];//包含校验位
    } r_regs;
    
    struct {  // 写入单个寄存器的应答
        uint8_t reg_addr_h;
        uint8_t reg_addr_l;
			
        uint8_t value_h;
        uint8_t value_l;

        uint8_t crc_h;
        uint8_t crc_l;
    } w_reg;

    struct {  // 写入多个寄存器的应答
        uint8_t reg_addr_h;
        uint8_t reg_addr_l;
			
        uint8_t quantity_h;
        uint8_t quantity_l;
			
        uint8_t crc_h;
        uint8_t crc_l;
    } w_regs;
}mb_resp;








#endif /* MB_MAP_H */
