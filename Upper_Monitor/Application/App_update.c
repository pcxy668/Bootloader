/*
 *@Author: CaiYuhao
 *@Date: 2026-03-05 01:44:09
 *@Description: 该上位机程序功能：
    1. 通过usart接收一个更新的程序 储存在FLASH中 程序起始地址为0x08008000
    2. 等待下位机发送更新请求
    3. 收到更新请求后通过CAN发送更新程序 
    4. 发送完成后重新回到等待串口或CAN请求的状态
 
 Copyright (c) 2025 by CaiYuhao, All Rights Reserved. 
 */

#include "App_update.h"

State_Type app_state = INIT;
uint8_t usart_Rxbuff[256] = {0};
uint8_t can_Rxbuff[8] = {0};
uint8_t can_Rxbuff_len = 0;
uint16_t usart_cur_Rxlen = 0;
uint16_t app_update_size = 0;
uint16_t written_len = 0;
uint16_t remain_len = 0;
uint32_t current_time = 0;

/**
 * @brief 初始化函数
 */
void App_Update_Init(void)
{  
    debug_printf("App Update Init!\n");
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, usart_Rxbuff, 256);
}

/**
 * @brief 发送程序函数
 */
void App_Update_SendProgram(void)
{
    if((can_Rxbuff[0] == 0xAC) && (can_Rxbuff[1] == 0xBD) && (can_Rxbuff[2] == 0xCE))
    {
        debug_printf("receive can update request!Now start updating\n");
        HAL_Delay(1000);
        written_len = 0;
        remain_len = app_update_size;
        for (uint16_t i = 0; i < app_update_size / 1024; i++)
        {
            for (uint16_t j = 0; j < 1024; j += 8)
            {
                CAN_Send_anylength((uint8_t *)(APP_START_ADDR_FLASH + written_len + j),8);
                HAL_Delay(5);    
            }
            written_len += 1024;
            remain_len -= 1024;
            HAL_Delay(200);
        }

        while (remain_len > 0)
        {
            if (remain_len >= 8)
            {
                CAN_Send_anylength((uint8_t *)(APP_START_ADDR_FLASH + written_len),8);
                HAL_Delay(5);
                written_len += 8;
                remain_len -= 8;
            }
            else
            {
                CAN_Send_anylength((uint8_t *)(APP_START_ADDR_FLASH + written_len),remain_len);
                HAL_Delay(5);               
                written_len += remain_len;
                remain_len = 0;
            }
        }        

        HAL_Delay(1000);
        uint8_t buff[2];
        buff[0] = app_update_size & 0xFF;
        buff[1] = (app_update_size >> 8) & 0xFF;
        CAN_Send_anylength(buff,2);
        debug_printf("Program send over.APP size:%d,written size:%d,remain size:%d\n", app_update_size, written_len, remain_len);
        app_state = WAIT_UPDATE_REQUEST;
        HAL_Delay(1000);

        for (uint8_t i = 0; i < 8; i++)
        {
            can_Rxbuff[i] = 0;
        }
    }
}

/**
 * @brief 主运行函数
 */
void App_Update_Work(void)
{
    switch (app_state)
    {
    case INIT:
        Init2515();
        App_Update_Init();
        app_state = WAIT_UPDATE_REQUEST;
        debug_printf("Please send update program by UART! BRP=115200\n");
        break;
    case RECEIVE_PROGRAM:
        if (current_time > 0 && (HAL_GetTick() - current_time > 2000))
        {
            current_time = 0;
            debug_printf("write flash success. APP size:%d,written size:%d,remain size:%d\n", app_update_size, written_len, remain_len);
            app_state = WAIT_UPDATE_REQUEST;
            debug_printf("Please send uart or can update requeset!\n");
        }
        break; 
    case WAIT_UPDATE_REQUEST:
        App_Update_SendProgram();
        break; 
    default:
        break;
    }   
}

/**
 * @brief 擦除FLASH
 */
static void App_Update_EarseFlash(void)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = APP_START_ADDR_FLASH;
    EraseInitStruct.NbPages = (app_update_size / FLASH_PAGE_SIZE) + 1;

    uint32_t PageError = 0;
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    HAL_FLASH_Lock();
}

/**
 * @brief 写入FLASH
 */
static void App_Update_WriteFlash(void)
{
    uint16_t data = 0;
    HAL_FLASH_Unlock();

    //此处为简化逻辑，未考虑接收程序为奇数字节的情况
    for (uint16_t i = 0; i < usart_cur_Rxlen; i += 2)
    {
        data = usart_Rxbuff[i] | usart_Rxbuff[i + 1] << 8;
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START_ADDR_FLASH + written_len + i, data);
    }

    HAL_FLASH_Lock();

    // 写入完成后，更新剩余大小和已写入大小
    remain_len -= usart_cur_Rxlen;
    written_len = app_update_size - remain_len;
}

/**
 * @brief 串口接收中断回调函数
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        if ((app_state == WAIT_UPDATE_REQUEST) && (usart_Rxbuff[0] == 0x5A) && Size == 3)
        {
            debug_printf("receive %#X %#X %#X\n",usart_Rxbuff[0],usart_Rxbuff[1],usart_Rxbuff[2]);
            app_state = RECEIVE_PROGRAM;
            app_update_size = usart_Rxbuff[1] | usart_Rxbuff[2] << 8;
            remain_len = app_update_size;
            written_len = 0;
            App_Update_EarseFlash();
            debug_printf("Please send bin file!\n");   
        }
        else if (app_state == RECEIVE_PROGRAM)
        {
            current_time = HAL_GetTick();
            usart_cur_Rxlen = Size;
            App_Update_WriteFlash();
        } 
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, usart_Rxbuff, 256);
    }
}

/**
 * @brief MCP2515接收信息中断回调函数 PB3 低电平有效
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MCP2515_INT_Pin)
    {
        CAN_Receive_DLC(can_Rxbuff,&can_Rxbuff_len);
        debug_printf("Receive CAN requeset:\n");
        for (uint8_t i = 0; i < can_Rxbuff_len; i++)
        {
            debug_printf("%02X ",can_Rxbuff[i]);
        }
    }
}
