/*
 *@Author: CaiYuhao
 *@Date: 2026-02-14 17:17:31
 *@Description: STM32内部flash分为Bootloader和APP两个区
                Bootloader区执行逻辑：
                1.校验是否更新
                2.若更新，则先擦除APP区程序，同时将外部flash程序写入APP区
                3.若未更新，则跳转执行APP区程序
 Copyright (c) 2025 by CaiYuhao, All Rights Reserved. 
 */

#include "App_bootloader.h"
