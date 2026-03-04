#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__

#include "usart.h"
#include "crc.h"
#include "Int_mcp2515.h"
#include "Com_Debug.h"

#define APP_START_ADDR_FLASH 0x08008000

typedef enum
{
    INIT = 0,
    RECEIVE_PROGRAM,
    WAIT_UPDATE_REQUEST,
    TRANSFER_PROGRAM
} State_Type;

void App_Update_Init(void);

void App_Update_SendProgram(void);

void App_Update_Work(void);

#endif /* __APP_UPDATE_H__ */
