#ifndef __SCANKEY_H__
#define __SCANKEY_H__

#include <stdbool.h>
#include <stdint.h>

#define KEY1  GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)//PF4
#define KEY2  GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)//PF0

#define SW1	1
#define SW2	2

void key_scan_init(void);
uint8_t key_scan(void);

#endif 
