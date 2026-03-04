/*
 *@Author: CaiYuhao
 *@Date: 2026-03-05 01:47:50
 *@Description: 主要功能：
    1. 一般情况下为APP_RUN状态，运行主应用程序(此处简化为LED闪烁程序)
    2. 待KEY0按下时，向上位机发送CAN更新程序请求
    3. 同时接收上位机通过CAN发来的更新程序
    4. 将接收的程序同步写入W25Q16 起始地址为0x003000
    5. 校验程序正确性并更新W25Q16相关元数据(更新标志位、程序大小、校验位等信息)
    6. 重启系统，进入Bootloader，实现系统更新
 Copyright (c) 2025 by CaiYuhao, All Rights Reserved. 
 */

#include "App_update.h"

State_Type state = APP_RUN;
uint8_t can_Rxbuff[1024] = {0};
uint16_t can_Rxbuff_len = 0;
uint16_t can_real_receive_len = 0;
uint16_t written_size = 0;
uint8_t w25q16_erase_flag = 0;
uint32_t current_time = 0;

/**
 * @brief 状态机
 */
void App_Update_Work(void)
{
    switch (state)
    {
    case APP_RUN:
        App_Update_AppRun();
        break;
    case APP_SEND_REQUEST:
        App_Update_SendRequest_CAN();
        break;
    case APP_RECEIVE_UPDATE:
        App_Update_ReceiveUpdate_CAN();
        if (current_time != 0 && (HAL_GetTick() - 500 > current_time))
        {
            /* 将缓冲区剩余数据写入W25Q16 */
            uint16_t i = 0;
            while (can_Rxbuff_len > 0)
            {
                if (can_Rxbuff_len >= 256)
                {
                    Int_W25Q16_PageWrite_With32Addr(APP_START_ADDR_W25Q16 + written_size,can_Rxbuff + i,256);
                    written_size += 256;
                    can_Rxbuff_len -= 256;
                    i += 256;
                }
                else
                {
                    Int_W25Q16_PageWrite_With32Addr(APP_START_ADDR_W25Q16 + written_size,can_Rxbuff + i,can_Rxbuff_len);
                    written_size += can_Rxbuff_len;
                    can_Rxbuff_len = 0;
                }
            }

            state = APP_RECEIVE_CRC;
        } 
        break;
    case APP_RECEIVE_CRC:
        App_Update_ReceiveCRC_CAN();
        break;     
    case APP_UPDATE_DONE:
        HAL_Delay(1000);
        HAL_NVIC_SystemReset();
        break;    
    default:
        break;
    }
}

/**
 * @brief 主应用程序
 */
void App_Update_AppRun(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_RESET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);
    HAL_Delay(500);
}

/**
 * @brief 向上位机发送CAN更新请求
 */
void App_Update_SendRequest_CAN(void)
{
    HAL_Delay(20);
    uint8_t data[3] = {0xAC, 0xBD, 0xCE};
    CAN_SendMsg(0x01, 3, data);
    state = APP_RECEIVE_UPDATE;

    written_size = 0;
    can_Rxbuff_len = 0;
    can_real_receive_len = 0;
    w25q16_erase_flag = 0;
    current_time = 0;
    
    debug_printf("Send update request\n");
}

/**
 * @brief 接收CAN更新程序
 */
void App_Update_ReceiveUpdate_CAN(void)
{
    /* 擦除W25Q16 APP存储区 8个扇区 8*4k=32k 空间*/ 
    if (w25q16_erase_flag == 0)
    {
        w25q16_erase_flag = 1;
        for (uint8_t i = 0; i < 8; i++)
        {
            Int_W25Q16_SectorEarse(0,3 + i);
        }
    }
    
    /* 接收数据 */
    RxMsg rxMsg[3] = {0};
    uint8_t msgCount = 0;
    CAN_ReceiveMsg(rxMsg,&msgCount);

    for (uint8_t i = 0; i < msgCount; i++)
    {
        current_time = HAL_GetTick();
        for (uint8_t j = 0; j < rxMsg[i].len; j++)
        {
            can_Rxbuff[can_Rxbuff_len++] = rxMsg[i].data[j];
            can_real_receive_len++;
        }
    }

    /* 将缓冲区数据写入W25Q16 */
    if (can_Rxbuff_len >= 1024)
    {
        for (uint16_t i = 0; i < 4; i++)
        {
            Int_W25Q16_PageWrite_With32Addr(APP_START_ADDR_W25Q16 + written_size,can_Rxbuff + i * 256,256);
            written_size += 256;
        }
        can_Rxbuff_len = 0;
    }
}

/**
 * @brief 接收CAN更新程序CRC校验(暂以接收真实程序字节数代替CRC验证)
 */
void App_Update_ReceiveCRC_CAN(void)
{
    /* 接收数据 */
    RxMsg rxMsg[3];
    uint8_t msgCount;
    CAN_ReceiveMsg(rxMsg,&msgCount);
    if (msgCount > 0)
    {
        uint16_t app_size = rxMsg[0].data[0] | (rxMsg[0].data[1] << 8);
        if (app_size == written_size)
        {
            debug_printf("Receive program success,app_size=%d,real_receive_size=%d,written_size=%d\n",app_size,can_real_receive_len,written_size);
            state = APP_UPDATE_DONE;
            
            // 写入待更新标志
            uint8_t buff[3] = {0x4F, 0X4B, 0x00};
            Int_W25Q16_SectorEarse(0, 0);
            Int_W25Q16_PageWrite(0,0,0,0,buff,2);

            // 写入程序版本号及大小
            buff[0] = 0x00;
            buff[1] = written_size & 0xFF;
            buff[2] = (written_size >> 8) & 0xFF;
            Int_W25Q16_SectorEarse(0, 1);
            Int_W25Q16_PageWrite(0,1,0,0,buff,3);

            // 写入校验成功标志
            buff[0] = 0x68;
            Int_W25Q16_SectorEarse(0, 2);
            Int_W25Q16_PageWrite(0,2,0,0,buff,1);
        }
        else
        {
            debug_printf("Receive program failed,app_size=%d,real_receive_size=%d,written_size=%d\n",app_size,can_real_receive_len,written_size);
            state = APP_RUN;
        } 

        debug_printf("Receive program content:\n"); 
        for (uint16_t i = 0; i < written_size; i++)
        {
            uint8_t buff = 0;
            Int_W25Q16_ReadData_With32Addr(APP_START_ADDR_W25Q16 + i,&buff,1);
            debug_printf("%02X ", buff);
        }
    }
    
}

/**
 * @brief 外部中断回调函数(按键KEY0切换请求状态)
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY0_Pin)
    {
        state = APP_SEND_REQUEST;
    } 
}
