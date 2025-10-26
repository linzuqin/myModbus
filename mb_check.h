#ifndef _MB_CHECK_H_
#define _MB_CHECK_H_

#include "mb.h"

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


mb_err_code_t mb_check(mb_dev_t *mb_dev, uint8_t *frame);







#endif /* _MB_CHECK_H_ */
