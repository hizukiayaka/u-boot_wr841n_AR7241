
/**************************************************************************************
* File Name  : tpLinuxTag.h
*
* Description: add tag with validation system to the firmware image file to be uploaded
*              via http
*
* Created    : 16Sep07,	Liang Qiming
**************************************************************************************/

#ifndef _TP_LINUX_TAG_H_
#define _TP_LINUX_TAG_H_


#define TP_SIG_1   "TP-LINK Technologies"
#define TP_SIG_2   "ver. 1.0"          // was "firmware version 2.0" now it is split 6 char out for chip id.

#define TP_TAG_VER         "6"
#define TP_TAG_VER_LAST    "26"

// file tag (head) structure all is in clear text except validationTokens (crc, md5, sha1, etc). Total: 128 unsigned chars
#define TAG_LEN         	512


typedef unsigned int	Elf32_Addr;

#define FILE_TAG_SIZE			512

typedef struct _LINUX_FLASH_STRUCT
{
	unsigned long bootOffset;
	unsigned long macAddrOffset;
	unsigned long pinOffset;
	unsigned long kernelOffset;
	unsigned long rootfsOffset;
	unsigned long configOffset;
	unsigned long radioOffset;
} LINUX_FLASH_STRUCT;

unsigned long tp_htonl(unsigned long value);
unsigned short tp_htons(unsigned short value);

int getElfInfo(char *elfFile, Elf32_Addr *eEntry, Elf32_Addr *pVaddr);

#endif // _TP_LINUX_TAG_H_

