#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

#include "stdio.h"
#include "stdarg.h"
#include "usart.h"

#define DEBUG_ON 2

#if DEBUG_ON == 1
#define debug_printf(format, ...) printf("[%s:%d]" format "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#elif DEBUG_ON == 2
#define debug_printf(format, ...) printf(format, ##__VA_ARGS__)
#else
#define debug_printf(format, ...)
#endif

#endif
