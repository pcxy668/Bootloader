/*
 *@Author: CaiYuhao
 *@Date: 2026-02-14 17:17:31
 *@Description: STM32内部flash分为Bootloader和APP两个区
                0x0800 0000起始16KB空间，为Bootloader区
                0x0800 4000起始16KB空间，为保留空间(后续可扩展为出厂默认程序区)
                0x0800 8000起，为APP区.

                Bootloader区执行逻辑：
                1.校验是否更新
                2.若更新，则先擦除APP区程序，同时将外部flash程序写入APP区
                3.若未更新，则跳转执行APP区程序
 Copyright (c) 2025 by CaiYuhao, All Rights Reserved.
 */

#include "App_bootloader.h"
#include "Com_Debug.h"

Bootloader_Status_Type Bootloader_Status = STATUS_INIT;
uint8_t buff[FLASH_PAGE_SIZE] = {0};

/**
 * @brief 检查是否需要更新APP程序
 * W25Q16中:
 * 0x000000-0x000001 存储APP更新标志 0x4F 0x4B为待更新，0xAC 0X98为已更新
 * 0x001000 存储APP版本号
 * 0x001001-0x001002 存储APP大小 小端存储
 * 0x002000 存储APP校验值 0x68为校验通过，0x70为校验失败
 * 0x003000 APP程序起始地址
 */
void App_Bootloader_CheckUpdate(void)
{
    uint8_t update_flag[2] = {0};
    Int_W25Q16_ReadData(0, 0, 0, 0, update_flag, 2);
    if (update_flag[0] == 0x4F && update_flag[1] == 0x4B)
    {
        Bootloader_Status = STATUS_UPDATE;
    }
    else
    {
        Bootloader_Status = STATUS_NO_UPDATE;
    }
}

/**
 * @brief 擦除APP区程序
 */
void App_Bootloader_EarseFlash(void)
{
    uint8_t buff[2] = {0};
    uint16_t app_size = 0;
    Int_W25Q16_ReadData(0, 1, 0, 1, buff, 2);
    app_size = buff[0] + (buff[1] << 8);

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = APP_START_ADDR_FLASH;
    EraseInitStruct.NbPages = (app_size / FLASH_PAGE_SIZE) + 1;

    uint32_t PageError = 0;
    HAL_StatusTypeDef res = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    debug_printf("EraseInitStruct.NbPages=%d res=%d\n",EraseInitStruct.NbPages,res);

    HAL_FLASH_Lock();
}

/**
 * @brief 从外部flash读取程序写入内部flash
 */
void App_Bootloader_WriteFlash(void)
{
    uint16_t app_size = 0;
    Int_W25Q16_ReadData(0, 1, 0, 1, buff, 2);
    app_size = buff[0] + (buff[1] << 8);
    
    uint16_t remain = app_size, written_size = 0, data = 0;

    HAL_FLASH_Unlock();

    // 判断是否分页写入FLASH
    while (remain >= FLASH_PAGE_SIZE)
    {
        Int_W25Q16_ReadData_With32Addr(APP_START_ADDR_W25Q16 + written_size, buff, FLASH_PAGE_SIZE);

        for (uint16_t i = 0; i < FLASH_PAGE_SIZE; i += 2)
        {
            data = buff[i] | buff[i + 1] << 8;
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START_ADDR_FLASH + written_size + i, data);
        }

        // 写入完成后，更新剩余大小和已写入大小
        remain -= FLASH_PAGE_SIZE;
        written_size = app_size - remain;
    }

    // 如果剩余大小小于一页，则写入剩余大小
    if (remain > 0)
    {
        Int_W25Q16_ReadData_With32Addr(APP_START_ADDR_W25Q16 + written_size, buff, remain);

        if (remain % 2 == 0)
        {
            for (uint16_t i = 0; i < remain; i += 2)
            {
                data = buff[i] | buff[i + 1] << 8;
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START_ADDR_FLASH + written_size + i, data);
            }
        }
        else
        {
            for (uint16_t i = 0; i < remain - 1; i += 2)
            {
                data = buff[i] | buff[i + 1] << 8;
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START_ADDR_FLASH + written_size + i, data);
            }

            data = buff[remain - 1] | 0xFF << 8;
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START_ADDR_FLASH + written_size + remain - 1, data);
        }
    }

    remain = 0;
    written_size = app_size;

    HAL_FLASH_Lock();

    debug_printf("write flash success. APP size:%d,written size:%d,remain size:%d\n", app_size, written_size, remain);
}

/**
 * @brief 跳转到APP区执行程序
 */
void App_Bootloader_JumpToApp(uint32_t app_start_addr)
{
    typedef void (*pFunc)(void);

    uint32_t app_stack_ptr = *(volatile uint32_t *)app_start_addr;
    uint32_t app_reset_handler = *(volatile uint32_t *)(app_start_addr + 4);

    // 1. 健壮性判断
    // 1.1校验栈顶地址是否合法 其值应在[0x2000 0000-0x2001 0000)之间
    if ((app_stack_ptr & 0xFFFF0000) != STACK_ADDR)
    {
        debug_printf("stack addr error\n");
        return;
    }

    // 1.2校验复位中断地址是否合法 其值应在[0x0800 8000-0x0808 0000)之间
    if (app_reset_handler < APP_START_ADDR_FLASH || app_reset_handler > APP_END_ADDR_FLASH)
    {
        debug_printf("reset handler error\n");
        return;
    }

    // 2. 注销bootloader程序
    // 2.1 关闭HAL库 注销掉外设的配置 不会注销内核
    HAL_DeInit();

    // 2.2 关闭中断 重置systick及NVIC相关寄存器
    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    for (int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // 2.3 设置堆栈指针
    __set_MSP(app_stack_ptr);

    // 2.4 重定向中断向量表
    SCB->VTOR = app_start_addr;

    // 2.5 跳转到APP程序复位中断
    pFunc jump_to_app = (pFunc)app_reset_handler;
    jump_to_app();
}

/**
 * @brief 根据Bootloader状态更新APP程序
 */
void App_Bootloader_Update(void)
{
    if (Bootloader_Status == STATUS_UPDATE)
    {
        App_Bootloader_EarseFlash();
        App_Bootloader_WriteFlash();

        // 擦除待更新标志位
        uint8_t buff[2] = {0xAC, 0X98};
        Int_W25Q16_SectorEarse(0, 0);
        Int_W25Q16_PageWrite(0,0,0,0,buff,2);

        debug_printf("update success\n");
    }
}
