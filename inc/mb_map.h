/*
 * @Description:定义了modbus帧格式结构体 用于解析数据 
 * @Author: linzuqin
 * @Date: 2025-11-10 16:20:49
 * @LastEditTime: 2025-11-19 22:51:59
 * @LastEditors: linzuqin
 */
#ifndef MB_MAP_H
#define MB_MAP_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

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

#define MB_ADDR_BIT          0                  // modbus帧中地址对应的bit位
#define MB_FUNC_BIT          1                  // modbus帧中功能码对应的bit位
#define MB_REGH_ADDR_BIT     2                  // modbus帧中寄存器起始地址高位对应的bit位
#define MB_REGL_ADDR_BIT     3                  // modbus帧中寄存器起始地址低位对应的bit位
#define MB_REGH_COUNT_BIT    4                  // 寄存器数量高位
#define MB_REGL_COUNT_BIT    5                  // 寄存器数量低位
#define MB_BYTE_COUNT_BIT    6                  // 字节数

#define MB_VALUEH_BIT    4                  // 写入寄存器值高位
#define MB_VALUEL_BIT    5                  // 写入寄存器值低位


#endif /* MB_MAP_H */
