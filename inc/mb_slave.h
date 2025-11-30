/*
 * @Description: 定义了modbus从机帧格式的结构体,声明了从机设备以及相关的函数
 * @Author: linzuqin
 * @Date: 2025-11-10 16:20:49
 * @LastEditTime: 2025-11-21 15:51:17
 * @LastEditors: linzuqin
 */
#ifndef _MB_SLAVE_H_
#define _MB_SLAVE_H_

#include "mb.h"

void mb_s_poll(void);

// 从机函数声明
extern mb_dev_t mb_slave_devs[MB_SLAVE_NUM];

#endif /* _MB_SLAVE_H_ */
