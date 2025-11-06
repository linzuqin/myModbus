#include "mb_slave.h"
#include "myusart.h"

void mb_s_send(uint8_t *buf, uint16_t len)
{

}

void hold_set_callback(uint16_t addr, uint16_t val)
{

}

void coil_set_callback(uint16_t addr, uint16_t val)
{

}

mb_dev_t mb_slave_devs[MB_SLAVE_NUM] = 
{
	// [0] = {
	// 	.addr = 1,
	// 	.identifier = "device_1",
	// 	.mb_coil_reg = coil_buf,
	// 	.mb_disc_reg = disc_buf,
	// 	.mb_hold_reg = keep_buf,
	// 	.mb_input_reg = input_buf,
	// 	.send_callback = mb_s_send,
	// 	.rx_buffer = uart1_rx_buf,
	// 	.coil_size = MB_COIL_REG_SIZE - 1,
	// 	.disc_size = MB_DISC_REG_SIZE - 1,
	// 	.hold_size = MB_HOLD_REG_SIZE - 1,
	// 	.input_size = MB_INPUT_REG_SIZE - 1,
	// 	.hold_write_cb = hold_set_callback,
	// 	.coil_write_cb = coil_set_callback,
	// }
};

/*modbus从机对于读取指令的应答*/
static mb_err_t mb_s_build_response(mb_dev_t *mb_dev, mb_func_code_t func_code, uint16_t start_addr , uint16_t size, uint16_t wr_val , uint8_t *response , uint16_t response_size)
{
    mb_err_t result = MB_OK;
    uint16_t response_len = 0;
    
    switch(func_code) {
        case MB_FUNC_READ_COILS:        //线圈读取
        case MB_FUNC_READ_DISCRETE:    // 离散读取
        {
            uint8_t byte_count = (size + 7) / 8;
            response_len = 3 + byte_count;
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                response[2] = byte_count;  // 字节数
                memset(&response[3] , 0 , byte_count);

                // 打包位状态数据
                for(uint16_t i = 0; i < size; i++) 
                {
                    uint8_t status = 0;
                    
                    if(func_code == MB_FUNC_READ_COILS) 
                    {
                        status = mb_dev->mb_coil_reg[start_addr + i];
                    } 
                    else 
                    {
                        status = mb_dev->mb_disc_reg[start_addr + i];
                    }
                    
                    if(status) 
                    {
                        uint8_t byte_index = i / 8;
                        uint8_t bit_index = i % 8;
                        response[3 + byte_index] |= (1 << bit_index);
                    }
                }
            }

            break;
        }
        
        case MB_FUNC_READ_HOLDING: // 保持寄存器
        case MB_FUNC_READ_INPUT:   //输入寄存器
        {
            response[2] = (uint8_t)(size * 2);  // 字节数
            response_len = 3 + size * 2;
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                // 填充寄存器数据
                for(uint16_t i = 0; i < size; i++) 
                {
                    uint16_t reg_value = 0;
                    
                    if(func_code == MB_FUNC_READ_HOLDING) 
                    {
                        reg_value = mb_dev->mb_hold_reg[start_addr + i];
                    } 
                    else {
                        reg_value = mb_dev->mb_input_reg[start_addr + i];
                    }
                    
                    response[3 + i * 2] = (uint8_t)((reg_value >> 8) & 0xFF);  // 高位
                    response[4 + i * 2] = (uint8_t)(reg_value & 0xFF);         // 低位
                }
            }

            break;
        }
        
        case MB_FUNC_WRITE_SINGLE_COIL: //线圈写入
        {
            response_len = 6;
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                response[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                response[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                
                if(wr_val) {
                    response[4] = 0xFF;
                    response[5] = 0x00;
                } else {
                    response[4] = 0x00;
                    response[5] = 0x00;
                }
            }
            break;
        }
        
        case MB_FUNC_WRITE_SINGLE_REGISTER: //单个保持寄存器写入
        {
            response_len = 6;
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {   
                response[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                response[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                response[4] = (uint8_t)(wr_val >> 8);  // 值高位
                response[5] = (uint8_t)(wr_val & 0xFF); // 值低位
            }
            break;
        }
        
        case MB_FUNC_WRITE_MULTIPLE_COILS:  //多个线圈写入
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:   ///多个保持写入
        {
            response_len = 6;
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                response[2] = (uint8_t)(start_addr >> 8);   // 起始地址高位
                response[3] = (uint8_t)(start_addr & 0xFF); // 起始地址低位
                response[4] = (uint8_t)(size >> 8);     // 数量高位
                response[5] = (uint8_t)(size & 0xFF);   // 数量低位
            }
            break;
        }
        
        default:
            result = MB_ERR_FUNC;  // 不支持的功能码
            break;
    }
    
    if(result == MB_OK)
    {
        response[0] = mb_dev->addr;  // 设备地址
        response[1] = func_code;     // 功能码
        // 计算并添加CRC校验
        uint16_t crc = usMBCRC16(response, response_len);
        response[response_len] = (uint8_t)(crc & 0xFF);     // CRC低位
        response[response_len + 1] = (uint8_t)((crc >> 8) & 0xFF);  // CRC高位
    }

    
    return result;
}

static uint16_t mb_s_get_response_size(mb_func_code_t func_code, uint16_t quantity)
{
    uint16_t size = 0;
    
    switch(func_code)
    {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE:
        {
            uint8_t byte_count = (quantity + 7) / 8;
            size = 1 + 1 + 1 + byte_count + 2; // 地址(1) + 功能码(1) + 字节数(1) + 数据(byte_count) + CRC(2)
            break;
        }

        case MB_FUNC_READ_HOLDING:
        case MB_FUNC_READ_INPUT:
        {
            size = 1 + 1 + 1 + quantity * 2 + 2; // 地址(1) + 功能码(1) + 字节数(1) + 数据(quantity*2) + CRC(2)
            break;
        }

        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_SINGLE_REGISTER:
        {
            size = 1 + 1 + 2 + 2 + 2; // 地址(1) + 功能码(1) + 起始地址(2) + 数据(2) + CRC(2) = 8字节
            break;
        }

        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        {
            size = 1 + 1 + 2 + 2 + 2; // 地址(1) + 功能码(1) + 起始地址(2) + 数量(2) + CRC(2) = 8字节
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

/*modbus从机对于写入指令的应答*/
static mb_err_t mb_s_coil_parse(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t val)
{
    mb_dev->mb_coil_reg[start_addr] = (val == 0xFF00) ? 1 : 0;
		if(mb_dev->coil_write_cb != NULL)
		{
			mb_dev->coil_write_cb(start_addr , val);
		}
    return MB_OK;
}

static mb_err_t mb_s_hold_parse(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t val)
{
    mb_dev->mb_hold_reg[start_addr] = val;
		if(mb_dev->hold_write_cb != NULL)
		{
			mb_dev->hold_write_cb(start_addr , val);
		}
    return MB_OK;
}

static mb_err_t mb_s_coils_parse(mb_dev_t *mb_dev , uint16_t start_addr , uint8_t *val , uint16_t quantity)
{
    for(uint16_t i = 0; i < quantity; i++) {
        uint8_t byte_index = i / 8;
        uint8_t bit_index = i % 8;
        uint8_t coil_value = (val[byte_index] >> bit_index) & 0x01;
        mb_dev->mb_coil_reg[start_addr + i] = coil_value;
				if(mb_dev->coil_write_cb != NULL)
				{
					mb_dev->coil_write_cb(start_addr + i , coil_value);
				}
    }
    return MB_OK;
}

static mb_err_t mb_s_holds_parse(mb_dev_t *mb_dev , uint16_t start_addr , uint8_t *val , uint16_t quantity)
{
    for(uint16_t i = 0; i < quantity; i++) {
        uint16_t reg_value = (val[i * 2] << 8) | val[i * 2 + 1];
        mb_dev->mb_hold_reg[start_addr + i] = reg_value;
				if(mb_dev->hold_write_cb != NULL)
				{
					mb_dev->hold_write_cb(start_addr + i , reg_value);
				}
    }
    return MB_OK;
}

static mb_err_t mb_s_parse(mb_dev_t *mb_dev)
{
    /* CRC及地址在mb_s_data_get函数中已校验过，这里不再重复 */

    if(mb_dev->rx_size < MB_MIN_SIZE) {
        mb_dev->error_count++;
        mb_dev->rx_size = 0;
        return MB_ERR_SIZE;
    }

    mb_func_code_t function_code = (mb_func_code_t)mb_dev->rx_buffer[MB_FUNC_BIT];
    uint16_t start_addr = mb_dev->rx_buffer[MB_REGH_ADDR_BIT] << 8 | mb_dev->rx_buffer[MB_REGL_ADDR_BIT];
    uint16_t reg_count = 0;
    uint8_t byte_count = 0;

    mb_err_code_t check_result = mb_check(mb_dev, mb_dev->rx_buffer);
    
    if(check_result != PARSE_OK) {
        uint8_t exception_frame[5] = {0};
        exception_frame[0] = mb_dev->addr;
        exception_frame[1] = function_code | 0x80;  // 设置异常标志
        exception_frame[2] = check_result;          // 异常码
        
        uint16_t crc = usMBCRC16(exception_frame, 3);
        exception_frame[3] = (uint8_t)(crc & 0xFF);
        exception_frame[4] = (uint8_t)((crc >> 8) & 0xFF);
        
        mb_dev->send_callback(exception_frame, 5);
        mb_dev->error_count++;
        mb_dev->rx_size = 0;
        return MB_ERR_FUNC;
    }

    /* 处理数据 */
    mb_err_t parse_result = MB_OK;
    
    switch(function_code)
    {
        case MB_FUNC_READ_COILS://读线圈
        case MB_FUNC_READ_DISCRETE://读离散量
        case MB_FUNC_READ_HOLDING://读保持寄存器
        case MB_FUNC_READ_INPUT://读输入寄存器
            reg_count = mb_dev->rx_buffer[MB_REGH_COUNT_BIT] << 8 | mb_dev->rx_buffer[MB_REGL_COUNT_BIT];
            break;

        case MB_FUNC_WRITE_SINGLE_COIL://写单个线圈
            reg_count = 1;
            parse_result = mb_s_coil_parse(mb_dev, start_addr, mb_dev->rx_buffer[4] << 8 | mb_dev->rx_buffer[5]);
            break;

        case MB_FUNC_WRITE_SINGLE_REGISTER://写单个保持寄存器
            reg_count = 1;
            parse_result = mb_s_hold_parse(mb_dev, start_addr, mb_dev->rx_buffer[4] << 8 | mb_dev->rx_buffer[5]);
            break;

        case MB_FUNC_WRITE_MULTIPLE_COILS://写多个线圈
            reg_count = mb_dev->rx_buffer[MB_REGH_COUNT_BIT] << 8 | mb_dev->rx_buffer[MB_REGL_COUNT_BIT];
            byte_count = mb_dev->rx_buffer[6];
            if(mb_dev->rx_size < (7 + byte_count + 2)) {
                parse_result = MB_ERR_SIZE;
            } else {
                parse_result = mb_s_coils_parse(mb_dev, start_addr, &mb_dev->rx_buffer[7], reg_count);
            }
            break;

        case MB_FUNC_WRITE_MULTIPLE_REGISTERS://写多个保持寄存器
            reg_count = mb_dev->rx_buffer[MB_REGH_COUNT_BIT] << 8 | mb_dev->rx_buffer[MB_REGL_COUNT_BIT];
            byte_count = mb_dev->rx_buffer[6];
            if(mb_dev->rx_size < (7 + byte_count + 2)) {
                parse_result = MB_ERR_SIZE;
            } else {
                parse_result = mb_s_holds_parse(mb_dev, start_addr, &mb_dev->rx_buffer[7], reg_count);
            }
            break;

        default://功能码不支持
            mb_dev->error_count++;
            mb_dev->rx_size = 0;
            parse_result = MB_ERR_FUNC;
    }

    // 若解析出现错误 则直接返回
    if (parse_result != MB_OK) {
        uint8_t exception_frame[5] = {0};
        exception_frame[0] = mb_dev->addr;
        exception_frame[1] = function_code | 0x80;
        exception_frame[2] = Slave_Device_Failure;  // 从机设备故障
        
        uint16_t crc = usMBCRC16(exception_frame, 3);
        exception_frame[3] = (uint8_t)(crc & 0xFF);
        exception_frame[4] = (uint8_t)((crc >> 8) & 0xFF);
        
        mb_dev->send_callback(exception_frame, 5);
        mb_dev->error_count++;
        mb_dev->rx_size = 0;
        return parse_result;
    }

    /* 处理完数据后开始正常应答 */
    uint16_t size = mb_s_get_response_size(function_code, reg_count);//理论响应数据长度
    uint8_t *response_buf = NULL;
    
    if(size < MB_MAX_SIZE && size != 0)
    {
        response_buf = (uint8_t *)calloc(size , sizeof(uint8_t));
        if(response_buf == NULL) //若申请内存失败
        {
            // 内存分配失败，发送异常响应
            uint8_t exception_frame[5] = {0};
            exception_frame[0] = mb_dev->addr;
            exception_frame[1] = function_code | 0x80;
            exception_frame[2] = Slave_Device_Failure;
            
            uint16_t crc = usMBCRC16(exception_frame, 3);
            exception_frame[3] = (uint8_t)(crc & 0xFF);
            exception_frame[4] = (uint8_t)((crc >> 8) & 0xFF);
            
            mb_dev->send_callback(exception_frame, 5);
            mb_dev->error_count++;
            mb_dev->rx_size = 0;
            return MB_ERR_MEMORY;
        }
        else
        {
            parse_result = mb_s_build_response(mb_dev, function_code, start_addr, reg_count, mb_dev->rx_buffer[4] << 8 | mb_dev->rx_buffer[5], response_buf , size);
            if ( parse_result != MB_OK)
            {
                uint8_t exception_frame[5] = {0};
                exception_frame[0] = mb_dev->addr;
                exception_frame[1] = function_code | 0x80;
                exception_frame[2] = Slave_Device_Failure;
                
                uint16_t crc = usMBCRC16(exception_frame, 3);
                exception_frame[3] = (uint8_t)(crc & 0xFF);
                exception_frame[4] = (uint8_t)((crc >> 8) & 0xFF);
                
                mb_dev->send_callback(exception_frame, 5);
                mb_dev->error_count++;
                mb_dev->rx_size = 0;
            }
        }
    }
    else //响应数据长度错误
    {
        parse_result = MB_ERR_SIZE;
        uint8_t exception_frame[5] = {0};
        exception_frame[0] = mb_dev->addr;
        exception_frame[1] = function_code | 0x80;
        exception_frame[2] = Slave_Device_Failure;
        
        uint16_t crc = usMBCRC16(exception_frame, 3);
        exception_frame[3] = (uint8_t)(crc & 0xFF);
        exception_frame[4] = (uint8_t)((crc >> 8) & 0xFF);
        
        mb_dev->send_callback(exception_frame, 5);
        mb_dev->error_count++;
        mb_dev->rx_size = 0;
        return parse_result;
    }

    if (parse_result == MB_OK) 
    {
        mb_dev->send_callback(response_buf, size);
        mb_dev->tx_count++;
        mb_dev->rx_size = 0;
    }

    if(response_buf != NULL)
    {
        free(response_buf);
    }

    return MB_OK;
}


mb_err_t mb_s_data_get(uint8_t *data_buf , uint16_t data_len)
{
    uint16_t crc_cal = usMBCRC16(data_buf , data_len - 2);//计算CRC
    uint16_t crc_recv = (uint16_t)(data_buf[data_len - 2] | (data_buf[data_len - 1] << 8));//接收的CRC
    if(crc_cal != crc_recv)//保证CRC正确
    {
        return MB_ERR_CRC;
    }
    if(data_len == 0 || data_len > MB_MAX_SIZE)//保证传入参数正确
    {
        return MB_ERR_SIZE;
    }

    for(uint8_t i = 0;i<MB_SLAVE_NUM;i++)
    {
        if(data_buf[MB_ADDR_BIT] == mb_slave_devs[i].addr)//地址匹配
        {
            //地址匹配成功，开始处理数据
            mb_slave_devs[i].rx_size = data_len;
            mb_slave_devs[i].rx_count ++;
            return MB_OK;
        }
    }
    return MB_ERR_ADDR;//地址不匹配
}

void mb_s_poll(void)
{
    for(uint8_t i = 0;i<MB_SLAVE_NUM;i++)
    {
        if(mb_slave_devs[i].rx_size > 0)
        {
            mb_s_parse(&mb_slave_devs[i]);
        }
    }
}
