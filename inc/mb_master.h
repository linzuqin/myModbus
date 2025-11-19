#ifndef _MB_MASTER_H_
#define _MB_MASTER_H_

#include "mb_check.h"

typedef struct
{
    mb_payload_head head;
    mb_payload payload;
}mb_m_send_frame;

typedef struct
{
    mb_resp_head head;
    mb_resp payload;
}mb_m_recv_frame;

void mb_m_poll(void);

// 主机函数声明
extern mb_dev_t mb_master_devs[MB_MASTER_NUM];


#endif /* _MB_MASTER_H_ */
