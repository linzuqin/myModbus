#include "mb_check.h"

mb_err_code_t check(mb_func_code_t func_code , uint16_t start_addr, uint16_t quantity, mb_dev_t *mb_dev)
{
    mb_err_code_t result = PARSE_OK;
    uint16_t max_size = 0;
    uint16_t buf_size = 0;
    switch(func_code)
    {
        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_READ_COILS:
        {
            max_size = MB_COIL_REG_SIZE;
            buf_size = mb_dev->coil_size;
            break;
        }

        case MB_FUNC_READ_DISCRETE:
        {
            max_size = MB_DISC_REG_SIZE;
            buf_size = mb_dev->disc_size;
            break;
        }

        case MB_FUNC_READ_HOLDING:
        case MB_FUNC_WRITE_SINGLE_REGISTER:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        {
            max_size = MB_HOLD_REG_SIZE;
            buf_size = mb_dev->hold_size;

            break;
        }

        case MB_FUNC_READ_INPUT:
        {
            max_size = MB_INPUT_REG_SIZE;
            buf_size = mb_dev->input_size;
            break;
        }

        default:
        {
            return Illegal_Function;
        }
    }

    //1.先检查设备读取地址是否合法
    if(start_addr >= (mb_dev->start_addr + buf_size) || start_addr < mb_dev->start_addr)
    {
        return Illegal_Data_Address;
    }

    //2.在检查读取数量是否合法
    if(quantity < 1 || quantity > max_size || (quantity + start_addr - mb_dev->start_addr) > max_size) 
    {
        return Illegal_Data_Address;
    } 
    
    return result;
}

mb_err_code_t mb_check(mb_dev_t *mb_dev, uint8_t *frame)
{
    mb_func_code_t func_code = (mb_func_code_t)frame[MB_FUNC_BIT];
    uint16_t start_addr = (frame[MB_REGH_ADDR_BIT] << 8) | frame[MB_REGL_ADDR_BIT];
    uint16_t quantity = 0;

    mb_err_code_t result = PARSE_OK;
    
    switch(func_code)
    {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE:
        case MB_FUNC_READ_HOLDING:
        case MB_FUNC_READ_INPUT:
        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        {
            quantity = (frame[MB_REGH_COUNT_BIT] << 8) | frame[MB_REGL_COUNT_BIT];
            result = check(func_code, start_addr, quantity, mb_dev);
            break;
        }
        
        case MB_FUNC_WRITE_SINGLE_REGISTER:
        case MB_FUNC_WRITE_SINGLE_COIL:
        {
            quantity = 1;
            result = check(func_code, start_addr, quantity, mb_dev);
            
            if(func_code == MB_FUNC_WRITE_SINGLE_COIL && result == PARSE_OK)
            {
                uint16_t coil_value = (frame[MB_REGH_COUNT_BIT] << 8) | frame[MB_REGL_COUNT_BIT];
                if(coil_value != 0xFF00 && coil_value != 0x0000)
                {
                    result = Illegal_Data_Value;
                }
            }
            break;
        }
        
        default:
        {
            result = Illegal_Function;
            break;
        }
    }
    return result;
}

mb_err_code_t mb_slave_check(mb_dev_t *mb_dev, uint8_t *frame)
{
    mb_func_code_t func_code = (mb_func_code_t)frame[MB_FUNC_BIT];
    uint16_t start_addr = (frame[MB_REGH_ADDR_BIT] << 8) | frame[MB_REGL_ADDR_BIT];
    uint16_t quantity = 0;

    mb_err_code_t result = PARSE_OK;
    
    switch(func_code)
    {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE:
        case MB_FUNC_READ_HOLDING:
        case MB_FUNC_READ_INPUT:
        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        {
            quantity = (frame[MB_REGH_COUNT_BIT] << 8) | frame[MB_REGL_COUNT_BIT];
            result = check(func_code, start_addr, quantity, mb_dev);
            break;
        }
        
        case MB_FUNC_WRITE_SINGLE_REGISTER:
        case MB_FUNC_WRITE_SINGLE_COIL:
        {
            quantity = 1;
            result = check(func_code, start_addr, quantity, mb_dev);
            
            if(func_code == MB_FUNC_WRITE_SINGLE_COIL && result == PARSE_OK)
            {
                uint16_t coil_value = (frame[MB_REGH_COUNT_BIT] << 8) | frame[MB_REGL_COUNT_BIT];
                if(coil_value != 0xFF00 && coil_value != 0x0000)
                {
                    result = Illegal_Data_Value;
                }
            }
            break;
        }
        
        default:
        {
            result = Illegal_Function;
            break;
        }
    }
    return result;
}
