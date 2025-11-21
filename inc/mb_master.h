/*
 * @Description: modbus主机头文件 定义了主机帧格式 声明了主要的两个函数
 * @Author: linzuqin
 * @Date: 2025-11-20 17:15:46
 * @LastEditTime: 2025-11-20 17:49:59
 * @LastEditors: linzuqin
 */
#ifndef _MB_MASTER_H_
#define _MB_MASTER_H_

#include "mb.h"

mb_err_t mb_m_data_get(uint8_t *data_buf , uint16_t data_len);
void mb_m_poll(void);


// 主机函数声明
extern mb_dev_t mb_master_devs[MB_MASTER_NUM];


#endif /* _MB_MASTER_H_ */
