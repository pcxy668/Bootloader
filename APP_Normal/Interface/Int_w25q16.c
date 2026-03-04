#include "Int_w25q16.h"

void Int_W25Q16_Select(void)
{
    HAL_GPIO_WritePin(W25Q16_CS_GPIO_Port, W25Q16_CS_Pin, GPIO_PIN_RESET);
}

void Int_W25Q16_Deselect(void)
{
    HAL_GPIO_WritePin(W25Q16_CS_GPIO_Port, W25Q16_CS_Pin, GPIO_PIN_SET);
}

void Int_W25Q16_ReadID(uint8_t *mid, uint16_t *did)
{
    uint8_t cmd = 0x9F;
    uint8_t buf[2] = {0};
    Int_W25Q16_Select();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    HAL_SPI_Receive(&hspi2, mid, 1, 1000);
    HAL_SPI_Receive(&hspi2, buf, 2, 1000);
    Int_W25Q16_Deselect();

    *did = buf[0] << 8 | buf[1];
}

void Int_W25Q16_WriteEnable(void)
{
    uint8_t cmd = 0x06;
    Int_W25Q16_Select();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    Int_W25Q16_Deselect();
}

void Int_W25Q16_WriteDisable(void)
{
    uint8_t cmd = 0x04;
    Int_W25Q16_Select();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    Int_W25Q16_Deselect();
}

void Int_W25Q16_WaitNotBusy(void)
{
    uint8_t buf = 0, cmd = 0x05;
    Int_W25Q16_Select();
    while (1)
    {
        HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
        HAL_SPI_Receive(&hspi2, &buf, 1, 1000);
        if ((buf & 0x01) == 0)
        {
            break;
        }
    }
    Int_W25Q16_Deselect();
}

void Int_W25Q16_PageWrite(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *data, uint16_t len)
{
    uint8_t cmd = 0x02;
    uint8_t tmp = (sector << 4) | page;

    Int_W25Q16_WaitNotBusy();
    Int_W25Q16_WriteEnable();
    Int_W25Q16_Select();

    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &block, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &tmp, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &innerAddr, 1, 1000);

    for (uint16_t i = 0; i < len; i++)
    {
        HAL_SPI_Transmit(&hspi2, data + i, 1, 1000);
    }

    Int_W25Q16_Deselect();
    Int_W25Q16_WriteDisable();
}

void Int_W25Q16_PageWrite_With32Addr(uint32_t addr, uint8_t *data, uint16_t len)
{
    uint8_t cmd = 0x02;
    uint8_t tmp[3] = {0};

    tmp[0] = (addr >> 16) & 0xFF;
    tmp[1] = (addr >> 8) & 0xFF;
    tmp[2] = addr & 0xFF;

    Int_W25Q16_WaitNotBusy();
    Int_W25Q16_WriteEnable();
    Int_W25Q16_Select();

    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp + 1, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp + 2, 1, 1000);

    for (uint16_t i = 0; i < len; i++)
    {
        HAL_SPI_Transmit(&hspi2, data + i, 1, 1000);
    }

    Int_W25Q16_Deselect();
    Int_W25Q16_WriteDisable();    
}

void Int_W25Q16_ReadData(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *buffer, uint16_t len)
{
    uint8_t cmd = 0x03,tmp = (sector << 4) | page;

    Int_W25Q16_WaitNotBusy();
    Int_W25Q16_Select();

    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &block, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &tmp, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &innerAddr, 1, 1000);

    HAL_SPI_Receive(&hspi2, buffer, len, 1000);

    Int_W25Q16_Deselect();
}

void Int_W25Q16_ReadData_With32Addr(uint32_t addr, uint8_t *buffer, uint16_t len)
{
    uint8_t cmd = 0x03;
    uint8_t tmp[3] = {0};

    tmp[0] = (addr >> 16) & 0xFF;
    tmp[1] = (addr >> 8) & 0xFF;
    tmp[2] = addr & 0xFF;

    Int_W25Q16_WaitNotBusy();
    Int_W25Q16_Select();

    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp + 1, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp + 2, 1, 1000);
    
    HAL_SPI_Receive(&hspi2, buffer, len, 1000);

    Int_W25Q16_Deselect();    
}

void Int_W25Q16_SectorEarse(uint8_t block, uint8_t sector)
{
    uint8_t cmd = 0x20, tmp[2] = {sector << 4, 0x00};
    Int_W25Q16_WaitNotBusy();
    Int_W25Q16_WriteEnable();

    Int_W25Q16_Select();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &block, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp, 1, 1000);
    HAL_SPI_Transmit(&hspi2, tmp + 1, 1, 1000);
    Int_W25Q16_Deselect();

    Int_W25Q16_WriteDisable();
}

void Int_W25Q16_BlockEarse(uint8_t block)
{
    uint8_t cmd = 0xD8, tmp = 0;
    Int_W25Q16_WaitNotBusy();
    Int_W25Q16_WriteEnable();

    Int_W25Q16_Select();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &block, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &tmp, 1, 1000);
    HAL_SPI_Transmit(&hspi2, &tmp, 1, 1000);
    Int_W25Q16_Deselect();

    Int_W25Q16_WriteDisable();
}

void Int_W25Q16_ChipEarse()
{
    uint8_t cmd = 0xC7;
    Int_W25Q16_WaitNotBusy();
    Int_W25Q16_WriteEnable();
    Int_W25Q16_Select();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 1000);
    Int_W25Q16_Deselect();
    Int_W25Q16_WriteDisable();
}
