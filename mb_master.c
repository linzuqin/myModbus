#include "mb_master.h"

static uint32_t mb_get_tick(void)
{

    return 0;
}

static void mb_m_clean(mb_dev_t *mb_dev)
{
    if(mb_dev->rx_size != 0)
    {
        mb_dev->rx_size = 0;
    }

    memset(mb_dev->rx_buffer , 0 , MB_MAX_SIZE);
}

static mb_err_t mb_m_build_request(mb_dev_t *mb_dev, mb_func_code_t func_code, uint16_t start_addr , uint16_t size , uint8_t *request , uint16_t request_size)
{
    uint16_t request_len = 0;
    mb_err_t result = MB_OK;
    if(request == NULL)
    {
        result = MB_ERR_BUILD;
    }
    else
    {
        switch(func_code) 
        {
            case MB_FUNC_READ_COILS:        //线圈读取
            case MB_FUNC_READ_DISCRETE:    // 离散读取
            case MB_FUNC_READ_HOLDING: // 保持寄存器
            case MB_FUNC_READ_INPUT:   //输入寄存器
            {
                request_len = 6;
                if (request_len >= request_size - 1)
                { 
                    result = MB_ERR_SIZE;
                }
                else
                {
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    request[4] = (uint8_t)(size >> 8);     // 数量高位
                    request[5] = (uint8_t)(size & 0xFF);   // 数量低位
                }
                break;
            }
            
            case MB_FUNC_WRITE_SINGLE_COIL: //单个线圈写入
            {
                request_len = 6;
                if (request_len >= request_size - 1)
                { 
                    result = MB_ERR_SIZE;
                }
                else
                {
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    
                    if(mb_dev->mb_coil_reg[start_addr]) 
                    {
                        request[4] = 0xFF;
                        request[5] = 0x00;
                    } else {
                        request[4] = 0x00;
                        request[5] = 0x00;
                    }
                }
                break;
            }
            
            case MB_FUNC_WRITE_SINGLE_REGISTER: //单个保持寄存器写入
            {
                request_len = 6;
                if (request_len >= request_size - 1)
                { 
                    result = MB_ERR_SIZE;
                }
                else
                {
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    request[4] = (uint8_t)(mb_dev->mb_hold_reg[start_addr] >> 8);  
                    request[5] = (uint8_t)(mb_dev->mb_hold_reg[start_addr]  & 0xFF); 
                }
                break;
            }

            case MB_FUNC_WRITE_MULTIPLE_COILS: //多个线圈写入
            {
                uint8_t byte_count = (size + 7) / 8; // 计算字节数，向上取整
                request_len = 7 + byte_count;
                if (request_len >= request_size - 1)
                { 
                    result = MB_ERR_SIZE;
                }
                else
                {
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    request[4] = (uint8_t)(size >> 8);     // 数量高位
                    request[5] = (uint8_t)(size & 0xFF);   // 数量低位
                    request[6] = byte_count;                // 字节数
                    
                    // 填充线圈状态数据
                    for(uint16_t i = 0; i < size; i++) {
                        if(mb_dev->mb_coil_reg[start_addr + i]) {
                            uint8_t byte_index = i / 8;
                            uint8_t bit_index = i % 8;
                            request[7 + byte_index] |= (1 << bit_index);
                        }
                    }
                }
                
                break;
            }

            case MB_FUNC_WRITE_MULTIPLE_REGISTERS:   //多个保持写入
            {
                request_len = 7 + size * 2;
                if (request_len >= request_size - 1)
                { 
                    result = MB_ERR_SIZE;
                }
                else
                {
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    request[4] = (uint8_t)(size >> 8);     // 数量高位
                    request[5] = (uint8_t)(size & 0xFF);   // 数量低位
                    request[6] = size * 2;                  // 字节数
                    
                    // 填充寄存器数据
                    for(uint16_t i = 0; i < size; i++) {
                        uint16_t reg_value = mb_dev->mb_hold_reg[start_addr + i];
                        request[7 + i * 2] = (uint8_t)((reg_value >> 8) & 0xFF);  // 高位
                        request[8 + i * 2] = (uint8_t)(reg_value & 0xFF);         // 低位
                    }
                }
                break;
            }
        }
    }

    if(result == MB_OK)
    {
        request[0] = mb_dev->addr;  // 设备地址
        request[1] = func_code;     // 功能码
            
        uint16_t crc = usMBCRC16(request, request_len);
        request[request_len] = (uint8_t)(crc & 0xFF);     // CRC低位
        request[request_len + 1] = (uint8_t)((crc >> 8) & 0xFF);  // CRC高位
    }

    return result;
}

static mb_err_t mb_m_check_ack(mb_dev_t *mb_dev)
{
    uint32_t start_time = mb_get_tick();
    
    // 等待响应
    while((mb_get_tick() - start_time) < mb_dev->timeout)
    {
        if(mb_dev->rx_size > 0)
        {
            mb_dev->rx_count++;
            break;
        }
        // 在这里添加延时等待
    }
    
    // 超时检查
    if((mb_get_tick() - start_time) >= mb_dev->timeout)
    {
        mb_dev->error_count++;
        return MB_ERR_TIMEOUT;
    }

    // 最小长度检查
    if (mb_dev->rx_size < MB_MIN_SIZE) {
        mb_dev->error_count++;
        return MB_ERR_SIZE;
    }
    
    // CRC检查
    uint16_t crc_cal = usMBCRC16(mb_dev->rx_buffer, mb_dev->rx_size - 2);
    uint16_t crc_recv = (mb_dev->rx_buffer[mb_dev->rx_size - 2] | 
                        (mb_dev->rx_buffer[mb_dev->rx_size - 1] << 8));
    if(crc_cal != crc_recv)
    {
        mb_dev->error_count++;
        return MB_ERR_CRC;
    }
    
    // 地址检查
    if (mb_dev->rx_buffer[0] != mb_dev->addr) {
        mb_dev->error_count++;
        return MB_ERR_ADDR;
    }
    
    if (mb_dev->rx_buffer[1] & 0x80) {
        mb_dev->error_count++;
        // 解析异常码
        if(mb_dev->rx_size >= 3) {
            mb_err_code_t exception_code = mb_dev->rx_buffer[2];
            switch(exception_code) {
                case Illegal_Function: return MB_ERR_FUNC;
                case Illegal_Data_Address: return MB_ERR_ADDR;
                case Illegal_Data_Value: return MB_ERR_DATA;
                case Slave_Device_Failure: return MB_ERR_DEVICE;
                default: return MB_ERR_FUNC;
            }
        }
        return MB_ERR_FUNC;
    }
    return MB_OK;
}

static uint16_t mb_m_get_request_size(mb_func_code_t func_code, uint16_t quantity)
{
    uint16_t size = 0;
    
    switch(func_code)
    {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE:
        case MB_FUNC_READ_HOLDING:
        case MB_FUNC_READ_INPUT:
            size = 1 + 1 + 2 + 2 + 2;// 地址(1) + 功能码(1) + 起始地址(2) + 数量(2) + CRC(2) = 8字节
            break;

        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_SINGLE_REGISTER:
            size = 1 + 1 + 2 + 2 + 2;// 地址(1) + 功能码(1) + 起始地址(2) + 数据(2) + CRC(2) = 8字节
            break;

        case MB_FUNC_WRITE_MULTIPLE_COILS:
        {
            uint8_t byte_count = (quantity + 7) / 8;
            size = 1 + 1 + 2 + 2 + 1 + byte_count + 2;// 地址(1) + 功能码(1) + 起始地址(2) + 数量(2) + 字节数(1) + 数据(byte_count) + CRC(2)
            break;
        }

        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        {
            size = 1 + 1 + 2 + 2 + 1 + (quantity * 2) + 2;// 地址(1) + 功能码(1) + 起始地址(2) + 数量(2) + 字节数(1) + 数据(quantity*2) + CRC(2)
            break;
        }

        default:
        {
            size = 0;
            break;
        }
    }
    return size;
}

static mb_err_t mb_m_send_request(mb_dev_t *mb_dev, mb_func_code_t func_code, uint16_t start_addr, uint16_t quantity)
{
    mb_err_t result = MB_OK;
    uint16_t size = mb_m_get_request_size(func_code, quantity);
    if(size == 0 || size > MB_MAX_SIZE)
    {
        return MB_ERR_BUILD;
    }

    uint8_t *request_buf = (uint8_t *)calloc(size , sizeof(uint8_t));
    if(request_buf == NULL)
    {
        result = MB_ERR_MEMORY;
        return result;
    }
    else
    {
        result = mb_m_build_request(mb_dev, func_code, start_addr, quantity, request_buf , size);
        if(result == MB_OK)
        {
            mb_dev->rx_size = 0;
            memset(mb_dev->rx_buffer, 0, MB_MAX_SIZE);

            mb_dev->send_callback(request_buf, size);
            mb_dev->tx_count++;

            result = mb_m_check_ack(mb_dev);
        }
    }

    if(request_buf != NULL)
    {
        free(request_buf);
    }

    return result;
}

mb_err_t mb_m_coil_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_COILS, start_addr, quantity);
    
    if(result == MB_OK)
    {
        uint8_t byte_count = mb_dev->rx_buffer[2];
        uint8_t expected_byte_count = (quantity + 7) / 8;
        
        // 验证字节数
        if(byte_count != expected_byte_count) {
            result = MB_ERR_SIZE;
        } 
        else {
            // 解析数据
            for(uint16_t i = 0; i < quantity; i++) {
                uint8_t byte_index = i / 8;
                uint8_t bit_index = i % 8;
                mb_dev->mb_coil_reg[start_addr + i] = 
                    (mb_dev->rx_buffer[3 + byte_index] >> bit_index) & 0x01;
            }
        }
    }
    
    mb_m_clean(mb_dev);
    return result;
}

mb_err_t mb_m_coil_set(mb_dev_t *mb_dev , uint16_t start_addr)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_SINGLE_COIL, start_addr, 1);
    mb_m_clean(mb_dev);
    return result;
}

mb_err_t mb_m_coils_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_MULTIPLE_COILS, start_addr, quantity);
    mb_m_clean(mb_dev);
    return result;
}

mb_err_t mb_m_disc_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_DISCRETE, start_addr, quantity);
    
    if(result == MB_OK)
    {
        uint8_t byte_count = mb_dev->rx_buffer[2];
        uint8_t expected_byte_count = (quantity + 7) / 8;
        
        if(byte_count != expected_byte_count) 
        {
            result = MB_ERR_SIZE;
        } 
        else 
        {
            for(uint16_t i = 0; i < quantity; i++) 
            {
                uint8_t byte_index = i / 8;
                uint8_t bit_index = i % 8;
                mb_dev->mb_disc_reg[start_addr + i] = (mb_dev->rx_buffer[3 + byte_index] >> bit_index) & 0x01;
            }
        }
    }
    
    mb_m_clean(mb_dev);
    return result;
}

mb_err_t mb_m_hold_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity) 
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_HOLDING, start_addr, quantity);
    if(result == MB_OK)
    {
        uint8_t byte_count = mb_dev->rx_buffer[2];
        uint8_t expected_byte_count = quantity * 2;
        
        if(byte_count != expected_byte_count) 
        {
            result = MB_ERR_SIZE;
        } 
        else 
        {
            for(uint16_t i = 0; i < quantity; i++) 
            {
                uint16_t reg_value = (mb_dev->rx_buffer[3 + i * 2] << 8) | mb_dev->rx_buffer[4 + i * 2];
                mb_dev->mb_hold_reg[start_addr + i] = reg_value;
            }
        }
    }

    mb_m_clean(mb_dev);
    return result;
}

mb_err_t mb_m_hold_set(mb_dev_t *mb_dev , uint16_t start_addr)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_SINGLE_REGISTER, start_addr, 1);
    mb_m_clean(mb_dev);
    return result;
}

mb_err_t mb_m_holds_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_MULTIPLE_REGISTERS, start_addr, quantity);
    mb_m_clean(mb_dev);
    return result;
}

mb_err_t mb_m_input_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_INPUT, start_addr, quantity);
    
    if(result == MB_OK)
    {
        uint8_t byte_count = mb_dev->rx_buffer[2];
        uint8_t expected_byte_count = quantity * 2;
        
        if(byte_count != expected_byte_count) 
        {
            result = MB_ERR_SIZE;
        } 
        else 
        {
            for(uint16_t i = 0; i < quantity; i++) 
            {
                uint16_t reg_value = (mb_dev->rx_buffer[3 + i * 2] << 8) | mb_dev->rx_buffer[4 + i * 2];
                mb_dev->mb_input_reg[start_addr + i] = reg_value;
            }
        }
    }
    mb_m_clean(mb_dev);
    return result;
}

