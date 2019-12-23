#ifndef USER_FLASH_H
#define USER_FLASH_H

#include "user_common.h"
/*@brief
stm32内部的rom有2处：
flash 64K~256K不等
信息块(系统存储器2K Byte 用户选择字节 16 Byte

内部ram有1处，即sram。

联系到boot的3种方式：
系统存储器boot
flash存储器boot
sram存储器boot
一般都是把程序下载到flash里面。
*/

/*@brief
我们想移植fs
128K的flash一共有128个块，每个块有1K。这个类似与sector(扇区)
除去下载到flash中的代码和只读数据，剩下的flash大小，就是我们可以操作的范围了
*/
#define FLASH_START_ADDR  0x8000000 //设置flash的起始地址
#define FLASH_SECTOR_SIZE 1024 //设置flash的扇区大小
#define USER_START_SECTOR	40 //设置用户操作的起始扇区
#define FLASH_SECTOR_MAX_NUMBER 64 //设置扇区数量
#define FLASH_SECTOR_NUMBER  (FLASH_SECTOR_MAX_NUMBER-USER_START_SECTOR) //用户可用扇区数量
#define FLASH_BLOCK_SIZE (FLASH_SECTOR_SIZE*4) //设置1个block=4个sector
/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;
typedef BYTE DSTATUS;

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

//repeat from diskio.h
/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at _FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at _USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at _USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

struct flash_driver{
	DSTATUS (*initialize)(BYTE pdrv);
	DSTATUS (*status)(BYTE pdrv);
	DRESULT (*read)(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
	DRESULT (*write)(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
	DRESULT (*ioctl)(BYTE pdrv, BYTE cmd, void* buff);
};
/*@brief:
返回flash大小，单位：KB
*/
unsigned short 
flash_size(void);
extern const struct flash_driver flash_driver;
#endif