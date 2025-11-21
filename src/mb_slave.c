#include "mb_slave.h"
// #include "myusart.h"

static uint8_t mb_s_coil_buf[MB_S_COIL_SIZE];
static uint8_t mb_s_disc_buf[MB_S_DISC_SIZE];
static uint16_t mb_s_hold_buf[MB_S_HOLD_SIZE];
static uint16_t mb_s_input_buf[MB_S_INPUT_SIZE];

static void mb_s_send(uint8_t *buf, uint16_t len)
{
	uint8_t tx_s_buf[len];

	memcpy(tx_s_buf, buf, len);
    // test code for one slave device
    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

    // HAL_UART_Transmit(uart_devices[0].uartHandle , tx_s_buf , len , 1000);

    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

static void hold_set_callback(uint16_t addr, uint16_t val)
{

}

static void coil_set_callback(uint16_t addr, uint16_t val)
{

}

mb_dev_t mb_slave_devs[MB_SLAVE_NUM];
// test code for one slave device
//  mb_dev_t mb_slave_devs[MB_SLAVE_NUM] = 
//  {
//  	[0] = {
//  		.addr = 1,
//  		.identifier = "device_1",
//  		.mb_coil_reg = mb_s_coil_buf,
//  		.mb_disc_reg = mb_s_disc_buf,
//  		.mb_hold_reg = mb_s_hold_buf,
//  		.mb_input_reg = mb_s_input_buf,
		
//  		.coil_start_addr = 0,
//  		.coil_read_size = sizeof(mb_s_coil_buf),
			
//  		.disc_start_addr = 0,
//  		.disc_read_size = sizeof(mb_s_disc_buf),
			
//  		.hold_start_addr = 0,
//  		.hold_read_size = sizeof(mb_s_hold_buf)/2,//mb_s_hold_buf是uint16_t类型的 sizeof算出来的长度会是实际长度的2倍
			
//  		.input_start_addr = 0,
//  		.input_read_size = sizeof(mb_s_input_buf)/2,//mb_s_hold_buf是uint16_t类型的 sizeof算出来的长度会是实际长度的2倍
			
//  		.send_callback = mb_s_send,
//  		.rx_buffer = uart1_rx_buf,
//  		.hold_write_cb = hold_set_callback,
//  		.coil_write_cb = coil_set_callback,
//  	}
//  };

mb_err_code_t mb_slave_check(mb_dev_t *mb_dev, mb_func_code_t func_code , uint16_t start_addr , uint16_t quantity)
{
    // mb_func_code_t func_code = frame[MB_FUNC_BIT];
    // uint16_t start_addr = frame[MB_REGH_ADDR_BIT] << 8 | frame[MB_REGL_ADDR_BIT];
    // uint16_t quantity = 0;
    mb_err_code_t result = PARSE_OK;
    uint16_t max_size = 0;
    uint16_t buf_size = 0;
    uint16_t dev_start_addr = 0;
    
    switch(func_code)
    {
        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_READ_COILS:
        {
            max_size = MB_S_COIL_SIZE;
            buf_size = mb_dev->coil_read_size;
            dev_start_addr = mb_dev->coil_start_addr;
            break;
        }

        case MB_FUNC_READ_DISCRETE:
        {
            max_size = MB_S_DISC_SIZE;
            buf_size = mb_dev->disc_read_size;
            dev_start_addr = mb_dev->disc_start_addr;
            break;
        }

        case MB_FUNC_READ_HOLDING:
        case MB_FUNC_WRITE_SINGLE_REGISTER:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        {
            max_size = MB_S_HOLD_SIZE;
            buf_size = mb_dev->hold_read_size;
            dev_start_addr = mb_dev->hold_start_addr;
            break;
        }

        case MB_FUNC_READ_INPUT:
        {
            max_size = MB_S_INPUT_SIZE;
            buf_size = mb_dev->input_read_size;
            dev_start_addr = mb_dev->input_start_addr;
            break;
        }

        default:
        {
            return Illegal_Function;
        }
    }

    if(start_addr >= (dev_start_addr + buf_size) || start_addr < dev_start_addr)
    {
        return Illegal_Data_Address;
    }

    if(quantity < 1 || quantity > max_size || (quantity + start_addr - dev_start_addr) > max_size) 
    {
        return Illegal_Data_Address;
    }
    
    return result;
}

/*modbus从机对于读取指令的应答*/
static mb_err_t mb_s_build_response(mb_dev_t *mb_dev, mb_func_code_t func_code , uint16_t start_addr , uint16_t reg_count, uint8_t *response , uint16_t response_size)
{
    mb_err_t result = MB_OK;
    uint16_t response_len = 6;

    switch(func_code) 
    {
        case MB_FUNC_READ_COILS:        //线圈读取
        case MB_FUNC_READ_DISCRETE:    // 离散读取
        {
            uint8_t byte_count = (reg_count + 7) / 8;
            response_len = 3 + byte_count;
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                response[2] = byte_count;  // 字节数

                // 打包位状态数据
                for(uint16_t i = 0; i < reg_count; i++) 
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
						uint16_t crc = usMBCRC16((uint8_t *)response, response_len);
						response[response_size - 2] = (uint8_t)(crc & 0xFF);     // CRC低位
						response[response_size  - 1] = (uint8_t)((crc >> 8) & 0xFF);  // CRC高位
            break;
        }
        
        case MB_FUNC_READ_HOLDING: // 保持寄存器
        case MB_FUNC_READ_INPUT:   //输入寄存器
        {
            response[2] = (reg_count * 2);  // 字节数
            response_len = 3 + reg_count * 2;
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                // 填充寄存器数据
                for(uint16_t i = 0; i < reg_count; i++) 
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
                    response[3 + i * 2 + 1] = (uint8_t)(reg_value & 0xFF);         // 低位
                }
            }
            uint16_t crc = usMBCRC16((uint8_t *)response, response_len);
						
						response[response_size - 2] = (uint8_t)(crc & 0xFF);     // CRC低位
						response[response_size  - 1] = (uint8_t)((crc >> 8) & 0xFF);  // CRC高位
            break;
        }
        
        case MB_FUNC_WRITE_SINGLE_COIL: //线圈写入
        {
            response[2] = (start_addr >> 8) &0xff;
            response[3] = (start_addr &0xff);
					
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                if(mb_dev->mb_coil_reg[start_addr]) {
                    response[4] = 0xFF;
                    response[5] = 0x00;
                } else {
                    response[4] = 0x00;
                    response[5] = 0x00;
                }
            }
            uint16_t crc = usMBCRC16((uint8_t *)response, response_len);
            response[6] = (uint8_t)(crc & 0xFF);     // CRC低位
            response[7] = (uint8_t)((crc >> 8) & 0xFF);  // CRC高位
            break;
        }
        
        case MB_FUNC_WRITE_SINGLE_REGISTER: //单个保持寄存器写入
        {
            response[2] = (start_addr >> 8) &0xff;
            response[3] = (start_addr &0xff);

            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {   
                response[4] = (uint8_t)(mb_dev->mb_hold_reg[start_addr] >> 8);  // 值高位
                response[5] = (uint8_t)(mb_dev->mb_hold_reg[start_addr] & 0xFF); // 值低位
            }
            uint16_t crc = usMBCRC16((uint8_t *)response, response_len);
            response[6] = (uint8_t)(crc & 0xFF);     // CRC低位
            response[7] = (uint8_t)((crc >> 8) & 0xFF);  // CRC高位
            break;
        }
        
        case MB_FUNC_WRITE_MULTIPLE_COILS:  //多个线圈写入
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:   ///多个保持写入
        {
            response[2] = (start_addr >> 8) &0xff;
            response[3] = (start_addr &0xff);
            if (response_len > response_size- 2)// -2是为了预留CRC空间
            { 
                result = MB_ERR_SIZE;
            }
            else
            {
                response[4]  = (uint8_t)(reg_count >> 8);  // 寄存器数量高位
                response[5]  = (uint8_t)(reg_count & 0xFF); // 寄存器数量低位
            }
            uint16_t crc = usMBCRC16((uint8_t *)response, response_len);
            response[6] = (uint8_t)(crc & 0xFF);     // CRC低位
            response[7] = (uint8_t)((crc >> 8) & 0xFF);  // CRC高位
            break;
        }
        
        default:
            result = MB_ERR_FUNC;  // 不支持的功能码
            break;
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
    uint8_t * response_buf;

    if(mb_dev->rx_size < MB_MIN_SIZE) {
        mb_dev->error_count++;
        mb_dev->rx_size = 0;
        return MB_ERR_SIZE;
    }

    mb_func_code_t function_code = mb_dev->rx_buffer[MB_FUNC_BIT];
    uint16_t start_addr = mb_dev->rx_buffer[MB_REGH_ADDR_BIT] << 8 | mb_dev->rx_buffer[MB_REGL_ADDR_BIT];
    uint16_t reg_count = 0;
    if(function_code == MB_FUNC_WRITE_SINGLE_COIL || function_code == MB_FUNC_WRITE_SINGLE_REGISTER)
    {
        reg_count = 1;
    }
    else
    {
        reg_count = mb_dev->rx_buffer[MB_REGH_COUNT_BIT] << 8 | mb_dev->rx_buffer[MB_REGL_COUNT_BIT];
    }
    uint8_t byte_count = 0;

    mb_err_code_t check_result = mb_slave_check(mb_dev, function_code , start_addr , reg_count);
    
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
            parse_result = mb_s_coil_parse(mb_dev, start_addr, mb_dev->rx_buffer[MB_VALUEH_BIT] << 8 | mb_dev->rx_buffer[MB_VALUEL_BIT]);
            break;
        case MB_FUNC_WRITE_SINGLE_COIL://写单个线圈
            reg_count = 1;
            parse_result = mb_s_coil_parse(mb_dev, start_addr, mb_dev->rx_buffer[MB_VALUEH_BIT] << 8 | mb_dev->rx_buffer[MB_VALUEL_BIT]);
            break;

        case MB_FUNC_WRITE_SINGLE_REGISTER://写单个保持寄存器
            reg_count = 1;
            parse_result = mb_s_hold_parse(mb_dev, start_addr, mb_dev->rx_buffer[MB_VALUEH_BIT] << 8 | mb_dev->rx_buffer[MB_VALUEL_BIT]);
            break;

        case MB_FUNC_WRITE_MULTIPLE_COILS://写多个线圈
            byte_count = mb_dev->rx_buffer[MB_BYTE_COUNT_BIT];
            if(mb_dev->rx_size < (7 + byte_count + 2)) {
                parse_result = MB_ERR_SIZE;
            } else {
                parse_result = mb_s_coils_parse(mb_dev, start_addr, &mb_dev->rx_buffer[MB_BYTE_COUNT_BIT + 1], reg_count);
            }
            break;

        case MB_FUNC_WRITE_MULTIPLE_REGISTERS://写多个保持寄存器
            byte_count = mb_dev->rx_buffer[MB_BYTE_COUNT_BIT];
            if(mb_dev->rx_size < (7 + byte_count + 2)) {
                parse_result = MB_ERR_SIZE;
            } else {
                parse_result = mb_s_holds_parse(mb_dev, start_addr, &mb_dev->rx_buffer[MB_BYTE_COUNT_BIT + 1], reg_count);
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
    response_buf = (uint8_t *)malloc(size);
    if(size < MB_MAX_SIZE && size != 0)
    {
        response_buf[0] = mb_dev->addr;  // 设备地址
        response_buf[1] = function_code;     // 功能码

        parse_result = mb_s_build_response(mb_dev , function_code  ,start_addr , reg_count , response_buf , size);
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
        free(response_buf);
    }
    return MB_OK;
}

mb_err_t mb_s_data_get(uint8_t *data_buf , uint16_t data_len)
{
    uint16_t crc_cal = usMBCRC16(data_buf , data_len - 2);//计算CRC
    uint16_t crc_recv = (uint16_t)((data_buf[data_len-1] << 8) | (data_buf[data_len-2] ));//接收的CRC
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
