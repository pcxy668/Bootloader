#include "w25q16.h"

void W25Q16_Init(void)
{
    SPI_Init();
}

void W25Q16_ReadID(uint8_t *mid, uint16_t *did)
{
    SPI_Start();
    SPI_Swap(0x9f);
    *mid = SPI_Swap(0xff);
    *did = SPI_Swap(0xff) << 8;
    *did = SPI_Swap(0xff);
    SPI_Stop();
}

void W25Q16_WriteEnable(void)
{
    SPI_Start();
    SPI_Swap(0x06);
    SPI_Stop();
}

void W25Q16_WriteDisable(void)
{
    SPI_Start();
    SPI_Swap(0x04);
    SPI_Stop();
}

void W25Q16_WaitNotBusy(void)
{
    SPI_Start();
    SPI_Swap(0x05);
    while (SPI_Swap(0xff) & 0x01)
    {
    }
    SPI_Stop();
}

void W25Q16_SectorEarse(uint8_t block, uint8_t sector)
{
    W25Q16_WaitNotBusy();
    W25Q16_WriteEnable();
    SPI_Start();
    SPI_Swap(0x20);
    SPI_Swap(block);
    SPI_Swap(sector << 4);
    SPI_Swap(0x00);
    SPI_Stop();//务必要先给个片选高电平才可生效
    W25Q16_WriteDisable();
}
void W25Q16_PageWrite(uint8_t block, uint8_t sector, uint8_t page, uint8_t *data, uint8_t len)
{
    W25Q16_WaitNotBusy();
    W25Q16_WriteEnable();
    SPI_Start();
    SPI_Swap(0x02);
    SPI_Swap(block);
    SPI_Swap((sector << 4) + page);
    SPI_Swap(0x00);
    for (uint8_t i = 0; i < len; i++)
    {
        SPI_Swap(data[i]);
    }
    SPI_Stop();
    W25Q16_WriteDisable();
}

void W25Q16_RandomWrite(uint32_t addr, uint8_t *data, uint8_t len)
{
    W25Q16_WaitNotBusy();
    W25Q16_WriteEnable();
    SPI_Start();
    SPI_Swap(0x02);
    SPI_Swap((addr >> 16) & 0xff);
    SPI_Swap((addr >> 8) & 0xff);
    SPI_Swap(addr & 0xff);
    for (uint8_t i = 0; i < len; i++)
    {
        SPI_Swap(data[i]);
    }
    SPI_Stop();
    W25Q16_WriteDisable();
}

void W25Q16_ReadData(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *buffer, uint16_t len)
{
    W25Q16_WaitNotBusy();
    SPI_Start();
    SPI_Swap(0x03);
    SPI_Swap(block);
    SPI_Swap((sector << 4) + page);
    SPI_Swap(innerAddr);
    for (uint16_t i = 0; i < len; i++)
    {
        buffer[i] = SPI_Swap(0xff);
    }
    SPI_Stop();
}

void W25Q16_ChipEarse()
{
    W25Q16_WaitNotBusy();
    W25Q16_WriteEnable();
    SPI_Start();
    SPI_Swap(0xc7);
    SPI_Stop();
    W25Q16_WriteDisable();
}

void W25Q16_BlockEarse(uint8_t block)
{
    W25Q16_WaitNotBusy();
    W25Q16_WriteEnable();
    SPI_Start();
    SPI_Swap(0xD8);
    SPI_Swap(block);
    SPI_Swap(0x00);
    SPI_Swap(0x00);
    SPI_Stop();
    W25Q16_WriteDisable();
}
