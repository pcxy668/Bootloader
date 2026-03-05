# Bootloader项目

### 1.目录结构
#### 1.1 一级目录
* APP_Normal APP区应用程序
* APP_Update APP区新的应用程序（测试用）
* Bootloader 系统Bootloader引导程序
* Upper_Monitor 上位机程序
#### 1.2 二级目录
* Common 公共层
* Core 核心层
* Drivers 驱动层             
* Interface 接口层
* Application 应用层
* MDK-ARM MDK工具链
### 2.硬件组成
*  上位机 STM32F103C8T6
*  下位机 STM32F103ZET6
*  外置存储 W25Q16
### 3.项目说明
#### 3.1 上位机：
* 通过usart串口接收要更新的程序，将其储存在FLASH自定义区域中
* 通过CAN接收下位机更新请求及发送更新程序包
#### 3.2 下位机
* Bootloader 检测是否有更新标志。若有，从W25Q16中读取程序信息并写入FLASH中APP区；若无，则直接跳转APP区
* APP程序中更新模块通过CAN向上位机发送更新请求并接收更新程序
* 接收到的新程序同步写入W25Q16中进行持久化存储，并更新相关标志位
