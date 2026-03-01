#ifndef __INT_W25Q16_H__
#define __INT_W25Q16_H__

#include "spi.h"

void Int_W25Q16_Select(void);

void Int_W25Q16_Deselect(void);

void Int_W25Q16_ReadID(uint8_t *mid, uint16_t *did);

void Int_W25Q16_WriteEnable(void);

void Int_W25Q16_WriteDisable(void);

void Int_W25Q16_WaitNotBusy(void);

void Int_W25Q16_PageWrite(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *data, uint8_t len);

void Int_W25Q16_PageWrite_With32Addr(uint32_t addr, uint8_t *data, uint8_t len);

void Int_W25Q16_ReadData(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *buffer, uint16_t len);

void Int_W25Q16_ReadData_With32Addr(uint32_t addr, uint8_t *buffer, uint16_t len);

void Int_W25Q16_SectorEarse(uint8_t block, uint8_t sector);

void Int_W25Q16_BlockEarse(uint8_t block);

void Int_W25Q16_ChipEarse(void);


#endif
