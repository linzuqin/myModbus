#ifndef _MB_CHECK_H_
#define _MB_CHECK_H_

#include "mb_map.h"
#include "mb_crc.h"

// 错误码定义
#define MB_EXCEPTION_ILLEGAL_FUNCTION      0x01
#define MB_EXCEPTION_ILLEGAL_DATA_ADDRESS  0x02
#define MB_EXCEPTION_ILLEGAL_DATA_VALUE    0x03
#define MB_EXCEPTION_SLAVE_DEVICE_FAILURE  0x04

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


mb_err_code_t mb_slave_check(mb_dev_t *mb_dev, uint8_t *frame);







#endif /* _MB_CHECK_H_ */
