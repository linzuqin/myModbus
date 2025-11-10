#ifndef _MB_SLAVE_H_
#define _MB_SLAVE_H_

#include "mb_check.h"

extern mb_dev_t mb_slave_devs[MB_SLAVE_NUM];

typedef struct
{
    uint8_t dev_addr;
    mb_func_code_t fun_code;
    uint8_t reg_addr_h;
    uint8_t reg_addr_l;
}mb_constant_head;

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

typedef union {
    struct {  // 读取寄存器的应答
        uint8_t byte_count;
        uint8_t data[248];//包含校验位
    } r_regs;
    
    struct {  // 写入单个寄存器的应答
        uint8_t value_h;
        uint8_t value_l;

        uint8_t crc_h;
        uint8_t crc_l;
    } w_reg;

    struct {  // 写入多个寄存器的应答
        uint8_t quantity_h;
        uint8_t quantity_l;
        uint8_t crc_h;
        uint8_t crc_l;
    } w_regs;
}mb_resp;

typedef struct
{
    mb_constant_head head;
    mb_payload payload;
}mb_s_parse_frame;

typedef struct
{
    mb_constant_head head;
    mb_resp payload;
}mb_s_resp_frame;

mb_err_t mb_s_data_get(uint8_t *data_buf , uint16_t data_len);
void mb_s_poll(void);


#endif /* _MB_SLAVE_H_ */
