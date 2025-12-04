#include "mb_master.h"
#include "usart.h"

//modbus主机专属的全局变量,专门用来存放需要下发数据的数组
static mb_m_map mb_m_coil_map[MB_M_COIL_SIZE];
static mb_m_map mb_m_hold_map[MB_M_HOLD_SIZE];

//modbus主机专属的全局变量,专门用来存放读取到的数据
static uint8_t mb_m_coil_buf[MB_M_COIL_SIZE];
static uint8_t mb_m_disc_buf[MB_M_DISC_SIZE];
static uint16_t mb_m_hold_buf[MB_M_HOLD_SIZE];
static uint16_t mb_m_input_buf[MB_M_INPUT_SIZE];


//串口发送函数 需要根据实际修改
static void mb_m_send(uint8_t *buf, uint16_t len)
{
    // test code for one slave device

	// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

	HAL_UART_Transmit(&huart1 , buf , len , 1000);
	
	// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

//保持寄存器写入回调函数
static void hold_set_callback(uint16_t addr, uint16_t val)
{

}

//线圈寄存器写入回调函数
static void coil_set_callback(uint16_t addr, uint16_t val)
{

}

// modbus主机设备列表
//   以下为默认配置 后续添加设备可以按照此参数配置
mb_dev_t mb_master_devs[MB_MASTER_NUM]	= 
{
    [0] = {
        .addr = 1,
        .dev_type = MB_MASTER,

        .mb_coil_reg = mb_m_coil_buf,
        .mb_disc_reg = mb_m_disc_buf,
        .mb_hold_reg = mb_m_hold_buf,
        .mb_input_reg = mb_m_input_buf,
        
        .coil_start_addr = 0,//线圈的起始地址
        .coil_read_size = sizeof(mb_m_coil_buf),//线圈的数量
            
        .disc_start_addr = 0,//离散的起始地址
        .disc_read_size = sizeof(mb_m_disc_buf),//离散的数量
            
        .hold_start_addr = 0,
        .hold_read_size = sizeof(mb_m_hold_buf)/2,//mb_s_hold_buf是uint16_t类型的 sizeof算出来的长度会是实际长度的2倍
            
        .input_start_addr = 0,
        .input_read_size = sizeof(mb_m_input_buf)/2,//mb_s_hold_buf是uint16_t类型的 sizeof算出来的长度会是实际长度的2倍
            
        .send_callback = mb_m_send,//发送函数
        .hold_write_cb = hold_set_callback,//保持寄存器设置回调函数
        .coil_write_cb = coil_set_callback,//线圈寄存器设置回调函数
        
        .coil_map = mb_m_coil_map,//主机线圈映射表
        .hold_map = mb_m_hold_map,//主机保持映射表
        
        .poll_interval = 100,//轮询时间100ms
        .index_func_code = MB_FUNC_READ_COILS,
    }
};


static void mb_m_clean(mb_dev_t *mb_dev)
{
    if(mb_dev->rx_size != 0)
    {
        mb_dev->rx_size = 0;
    }

    memset(mb_dev->rx_buffer , 0 , MB_MAX_SIZE);
}

//构建modbus主机请求报文
static mb_err_t mb_m_build_request(mb_dev_t *mb_dev, mb_func_code_t func_code, uint16_t start_addr , uint16_t size , uint16_t* val ,uint8_t *request , uint16_t request_size)
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
                else if(val == NULL)
                {
                    result = MB_ERR_SIZE;
                }
                else
                {
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    
                    if(*val) 
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
                else if(val == NULL)
                {
                    result = MB_ERR_SIZE;
                }
                else
                {
                    uint16_t reg_val = *val;
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    request[4] = (uint8_t)(reg_val >> 8);  
                    request[5] = (uint8_t)(reg_val  & 0xFF); 
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
                else if(val == NULL)
                {
                    result = MB_ERR_SIZE;
                }
                else
                {
                    uint16_t coil_value = 0;
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    request[4] = (uint8_t)(size >> 8);     // 数量高位
                    request[5] = (uint8_t)(size & 0xFF);   // 数量低位
                    request[6] = byte_count;                // 字节数
                    
                    // 填充线圈状态数据
                    for(uint16_t i = 0; i < size; i++) {
                        coil_value = val[i];
                        if(coil_value) {
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
                else if(val == NULL)
                {
                    result = MB_ERR_SIZE;
                }
                else
                {
                    uint16_t reg_value = 0;
                    request[2] = (uint8_t)(start_addr >> 8);   // 地址高位
                    request[3] = (uint8_t)(start_addr & 0xFF); // 地址低位
                    request[4] = (uint8_t)(size >> 8);     // 数量高位
                    request[5] = (uint8_t)(size & 0xFF);   // 数量低位
                    request[6] = size * 2;                  // 字节数
                    
                    // 填充寄存器数据
                    for(uint16_t i = 0; i < size; i++) {
                        reg_value = val[i];
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
    uint32_t end_time = 0;
    // 等待响应
    while((mb_get_tick() - start_time) < mb_dev->poll_interval)
    {
        if(mb_dev->rx_size > 0)
        {
			end_time = mb_get_tick();
            break;
        }
    }
    
    // 超时检查
    if((end_time - start_time) >= mb_dev->poll_interval)//超过一个轮询时间没收到应答则判断为超时
    {
        return MB_ERR_TIMEOUT;
    }

    // 最小长度检查
    if (mb_dev->rx_size < MB_MIN_SIZE) {
        return MB_ERR_SIZE;
    }
    
    if (mb_dev->rx_buffer[1] & 0x80) {
        // 解析异常码
        if(mb_dev->rx_size >= 3) {
            mb_err_code_t exception_code = (mb_err_code_t)mb_dev->rx_buffer[2];
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

//根据功能码和读取数量获取请求报文的理论长度
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

//modbus主机发送请求 包含了应答检测
static mb_err_t mb_m_send_request(mb_dev_t *mb_dev, mb_func_code_t func_code, uint16_t start_addr, uint16_t quantity , uint16_t *val)
{
    mb_err_t result = MB_OK;
    uint16_t size = mb_m_get_request_size(func_code, quantity);
    if(size == 0 || size > MB_MAX_SIZE)
    {
        return MB_ERR_BUILD;
    }

    uint8_t *payload_buf = mb_dev->tx_buffer;
    result = mb_m_build_request(mb_dev, func_code, start_addr, quantity, val , payload_buf , size);
    if(result == MB_OK)
    {
        mb_dev->rx_size = 0;
        memset(mb_dev->rx_buffer, 0, MB_MAX_SIZE);

        mb_dev->send_callback(payload_buf, size);

        result = mb_m_check_ack(mb_dev);

        memset(payload_buf , 0 , MB_MAX_SIZE);
    }
    return result;
}

static mb_err_t mb_m_coil_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_COILS, start_addr, quantity , NULL);
    
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

static mb_err_t mb_m_coil_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t value)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_SINGLE_COIL, start_addr, 1 , &value);
    mb_m_clean(mb_dev);
    return result;
}

static mb_err_t mb_m_coils_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity , uint16_t *value)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_MULTIPLE_COILS, start_addr, quantity , value);
    mb_m_clean(mb_dev);
    return result;
}

static mb_err_t mb_m_disc_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_DISCRETE, start_addr, quantity , NULL);
    
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

static mb_err_t mb_m_hold_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity) 
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_HOLDING, start_addr, quantity , NULL);
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

static mb_err_t mb_m_hold_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t value)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_SINGLE_REGISTER, start_addr, 1 , &value);
    mb_m_clean(mb_dev);
    return result;
}

static mb_err_t mb_m_holds_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity , uint16_t *value)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_WRITE_MULTIPLE_REGISTERS, start_addr, quantity , value);
    mb_m_clean(mb_dev);
    return result;
}

static mb_err_t mb_m_input_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity)
{
    mb_err_t result = mb_m_send_request(mb_dev, MB_FUNC_READ_INPUT, start_addr, quantity , NULL);
    
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

// 检查是否有待下发的数据
static uint8_t mb_map_check(mb_dev_t *dev)
{
    for(uint16_t i = 0; i < dev->coil_read_size; i++)
    {
        if(dev->coil_map[i].setFlag == 1)
            return 1;
    }
    
    for(uint16_t i = 0; i < dev->hold_read_size; i++)
    {
        if(dev->hold_map[i].setFlag == 1)
            return 1;
    }
    
    return 0;
}

/*modbus主机设置函数 根据标志位判断是否有下发任务*/
static mb_err_t mb_m_set(mb_dev_t *dev)
{
    mb_err_t result = MB_OK;
    uint16_t i = 0;
    for( i = 0; i < dev->coil_read_size ; i++ )
    {
        if(dev->coil_map[i].setFlag == 1)
        {
            result = mb_m_coil_set(dev , i + dev->coil_start_addr , dev->coil_map[i].value);
            if(result == MB_OK)
            {
                dev->coil_map[i].setFlag = 0;
            }
            else
            {
                //读取失败 返回错误
                return result;
            }
        }
    }

    for( i = 0; i < dev->hold_read_size ; i++ )
    {
        if(dev->hold_map[i].setFlag == 1)
        {
            result = mb_m_hold_set(dev , i + dev->hold_start_addr , dev->hold_map[i].value);
            if(result == MB_OK)
            {
                dev->hold_map[i].setFlag = 0;
            }
            else
            {
                //读取失败 返回错误
                return result;
            }
        }
    }

    return result;
}

static mb_err_t mb_m_get(mb_dev_t *dev)
{
    mb_err_t result = MB_OK;
    switch(dev->index_func_code)
    {
        case MB_FUNC_READ_COILS:
        {
            if(dev->coil_read_size > 0)
            {
                result = mb_m_coil_get(dev , dev->coil_start_addr , dev->coil_read_size);
                if( result != MB_OK)
                {
                    
                }
            }
						else
						{
							result = MB_ERR_FUNC;
						}
						dev->index_func_code = MB_FUNC_READ_DISCRETE;
						break;

        }

        case MB_FUNC_READ_DISCRETE:
        {
            if(dev->disc_read_size > 0)
            {
                result = mb_m_disc_get(dev , dev->disc_start_addr , dev->disc_read_size);
                if(result != MB_OK)
                {
                    
                }
            }
						else
						{
							result = MB_ERR_FUNC;
						}
						dev->index_func_code = MB_FUNC_READ_HOLDING;
						break;
        }

        case MB_FUNC_READ_HOLDING:
        {
            if(dev->hold_read_size > 0)
            {
                result = mb_m_hold_get(dev , dev->hold_start_addr , dev->hold_read_size);
                if( result != MB_OK)
                {
                    
                }
            }
						else
						{
							result = MB_ERR_FUNC;
						}
						dev->index_func_code = MB_FUNC_READ_INPUT;
						break;
        }

        case MB_FUNC_READ_INPUT:
        {
            if(dev->input_read_size > 0)
            {
                result = mb_m_input_get(dev , dev->input_start_addr , dev->input_read_size);
                if(result != MB_OK)
                {
                    
                }
            }
						else
						{
							result = MB_ERR_FUNC;
						}
						dev->index_func_code = MB_FUNC_READ_COILS;
						break;
					
        }
				
				default:
						result = MB_ERR_FUNC;
						dev->index_func_code = MB_FUNC_READ_COILS;
						break;
    }
    return result;
}

void mb_m_poll(void)
{
    for(uint8_t i = 0; i < MB_MASTER_NUM; i++)
    {
        mb_dev_t *dev = &mb_master_devs[i];
        
        switch(dev->mb_status)
        {
            case MB_IDLE:
            {
                uint32_t current_time = mb_get_tick();
                
                if(mb_map_check(dev))
                {
                    dev->mb_status = MB_SET;
                }
                else if(current_time - dev->last_poll_time >= dev->poll_interval)
                {
                    dev->last_poll_time = current_time;
                    dev->mb_status = MB_GET;
                }
                break;
            }

            case MB_GET:
            {
                if(mb_m_get(dev) != MB_OK)
                {
                    dev->error_count++;
                    if(dev->error_count > MAX_ERROR_COUNT)
                    {
                        dev->mb_status = MB_OFFLINE;
                    }
                    else
                    {
                        dev->mb_status = MB_IDLE;
                    }
                }
                else
                {
                    dev->error_count = 0;  // 成功时清零错误计数
                    dev->devOnline = 1;
                    dev->mb_status = MB_IDLE;
                }
                break;
            }

            case MB_SET:
            {
                if(mb_m_set(dev) != MB_OK)
                {
                    dev->error_count++;
                    if(dev->error_count > MAX_ERROR_COUNT)
                    {
                        dev->mb_status = MB_OFFLINE;
                    }
                    else
                    {
                        dev->mb_status = MB_IDLE;
                    }
                }
                else
                {
                    dev->error_count = 0;
                    dev->devOnline = 1;
                    dev->mb_status = MB_GET;
                }
                break;
            }

            case MB_OFFLINE:
            {
                dev->devOnline = 0;
                
                dev->mb_status = MB_IDLE;
                break;
            }

            default:
                dev->mb_status = MB_IDLE;  // 未知状态回到IDLE
                break;
        }
    }
}
