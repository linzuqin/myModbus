#ifndef _MB_SLAVE_H_
#define _MB_SLAVE_H_

#include "mb_check.h"

extern mb_dev_t mb_slave_devs[MB_SLAVE_NUM];

mb_err_t mb_s_data_get(uint8_t *data_buf , uint16_t data_len);
void mb_s_poll(void);


#endif /* _MB_SLAVE_H_ */
