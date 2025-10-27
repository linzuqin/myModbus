#ifndef _MB_MASTER_H_
#define _MB_MASTER_H_

#include "mb_check.h"

// 主机函数声明
mb_err_t mb_m_coil_get(mb_dev_t *mb_dev, uint16_t start_addr, uint16_t quantity);
mb_err_t mb_m_coil_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t value);
mb_err_t mb_m_coils_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity , uint16_t *value);
mb_err_t mb_m_disc_get(mb_dev_t *mb_dev, uint16_t start_addr, uint16_t quantity);
mb_err_t mb_m_hold_get(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity); 
mb_err_t mb_m_hold_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t value);
mb_err_t mb_m_holds_set(mb_dev_t *mb_dev , uint16_t start_addr , uint16_t quantity , uint16_t *value);
mb_err_t mb_m_input_get(mb_dev_t *mb_dev, uint16_t start_addr, uint16_t quantity);

#endif /* _MB_MASTER_H_ */
