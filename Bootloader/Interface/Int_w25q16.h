#ifndef __W25Q16_H
#define __W25Q16_H

#include "SPI.h"
void W25Q16_Init(void);
void W25Q16_ReadID(uint8_t *mid,uint16_t *did);
void W25Q16_WriteEnable(void);
void W25Q16_WriteDisable(void);
void W25Q16_WaitNotBusy(void);
void W25Q16_SectorEarse(uint8_t block,uint8_t sector);
void W25Q16_PageWrite(uint8_t block,uint8_t sector,uint8_t page,uint8_t *data,uint8_t len);
void W25Q16_RandomWrite(uint32_t addr,uint8_t *data,uint8_t len);
void W25Q16_ReadData(uint8_t block,uint8_t sector,uint8_t page,uint8_t innerAddr,uint8_t *buffer,uint16_t len);
void W25Q16_ChipEarse(void);
void W25Q16_BlockEarse(uint8_t block);

#endif
