#ifndef _MB_SLAVE_H_
#define _MB_SLAVE_H_

#include "mb_check.h"

extern mb_dev_t mb_slave_devs[MB_SLAVE_NUM];

typedef struct
{
    mb_payload_head head;
    mb_payload payload;
}mb_s_parse_frame;

typedef struct
{
    mb_resp_head head;
    mb_resp payload;
}mb_s_resp_frame;

mb_err_t mb_s_data_get(uint8_t *data_buf , uint16_t data_len);
void mb_s_poll(void);


#endif /* _MB_SLAVE_H_ */
