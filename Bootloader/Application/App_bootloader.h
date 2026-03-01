#ifndef __APP_BOOTLOADER_H__
#define __APP_BOOTLOADER_H__

#include "Int_w25q16.h"

#define BOOTLOADER_START_ADDR 0x08000000
#define APP_START_ADDR_FLASH 0x08008000
#define APP_END_ADDR_FLASH 0x08080000
#define STACK_ADDR 0x20000000
#define APP_START_ADDR_W25Q16 0x003000

typedef enum {
    STATUS_INIT = 0,
    STATUS_UPDATE,
    STATUS_NO_UPDATE
} Bootloader_Status_Type;

void App_Bootloader_CheckUpdate(void);

void App_Bootloader_EarseFlash(void);

void App_Bootloader_WriteFlash(void);

void App_Bootloader_JumpToApp(uint32_t app_start_addr);

void App_Bootloader_Update(void);

#endif /* __APP_BOOTLOADER_H__ */
