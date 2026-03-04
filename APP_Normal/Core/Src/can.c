/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 36;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_6TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = ENABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @description:配置过滤器
 */
void CAN_FilterConfig(void)
{
  CAN_FilterTypeDef filter0;
  filter0.FilterFIFOAssignment = CAN_RX_FIFO0; //设置经过筛选后数据存储到哪个接收FIFO
  filter0.FilterBank = 0; //过滤器编号，CAN1为0-13，CAN2为14-27
  filter0.FilterMode = CAN_FILTERMODE_IDMASK; //采用掩码模式
  filter0.FilterScale = CAN_FILTERSCALE_32BIT; //设置筛选器的尺度，采用32位
  filter0.FilterIdHigh = 0x0000; //过滤器ID高16位，即CAN_FxR1寄存器的高16位
  filter0.FilterIdLow = 0x0000; //过滤器ID低16位，即CAN_FxR1寄存器的低16位
  filter0.FilterMaskIdHigh = 0x0000; //过滤器掩码高16位，即CAN_FxR2寄存器的高16位
  filter0.FilterMaskIdLow = 0x0000; //过滤器掩码低16位，即CAN_FxR2寄存器的低16位
  filter0.FilterActivation = ENABLE; // 是否使能本筛选器
  HAL_CAN_ConfigFilter(&hcan, &filter0);
}

/**
 * @description: 发送消息
 */
void CAN_SendMsg(uint16_t id, uint8_t len, uint8_t *data)
{
  /* 1.检测发送邮箱是否可用 */
  while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
  {
  }
  CAN_TxHeaderTypeDef txHeader;
  txHeader.IDE = CAN_ID_STD; // 标准帧还是扩展帧
  txHeader.StdId = id; // 标准帧的id
  txHeader.RTR = CAN_RTR_DATA; // 帧的类型: 数据帧还是远程帧
  txHeader.DLC = len; //发送的数据长度 单位字节
  uint32_t TxMailbox; // 会把这次使用的邮箱存入到这个变量

  /* 2.发送消息 */
  HAL_CAN_AddTxMessage(&hcan, &txHeader, data, &TxMailbox);
  while ((__HAL_CAN_GET_FLAG(&hcan, CAN_FLAG_TXOK0)) == 0)
  {
  }
}

/**
 * @description: 接收消息
 */
void CAN_ReceiveMsg(RxMsg rxMsg[], uint8_t *msgConut)
{
  /* 1.检测FIFO0收到的报文个数 */
  *msgConut = HAL_CAN_GetRxFifoFillLevel(&hcan,CAN_RX_FIFO0);

  /* 2.遍历出所有消息 */
  CAN_RxHeaderTypeDef rxHeader;
  for (uint8_t i = 0; i < *msgConut; i++)
  {
    HAL_CAN_GetRxMessage(&hcan,CAN_RX_FIFO0,&rxHeader,rxMsg[i].data);
    rxMsg[i].id = rxHeader.StdId;
    rxMsg[i].len = rxHeader.DLC;
  }
}
/* USER CODE END 1 */
