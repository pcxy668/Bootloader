#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__

#include "can.h"
#include "Com_Debug.h"
#include "Int_w25q16.h"

#define APP_START_ADDR_FLASH 0x08008000
#define APP_START_ADDR_W25Q16 0x003000

typedef enum
{
    APP_RUN = 0,
    APP_SEND_REQUEST,
    APP_RECEIVE_UPDATE,
    APP_RECEIVE_CRC,
    APP_UPDATE_DONE,
} State_Type;

void App_Update_Work(void);

void App_Update_AppRun(void);

void App_Update_SendRequest_CAN(void);

void App_Update_ReceiveUpdate_CAN(void);

void App_Update_ReceiveCRC_CAN(void);

void App_Update_UpdateDone(void);

#endif /* __APP_UPDATE_H__ */
