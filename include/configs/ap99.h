/*
 * This file contains the configuration parameters for the pb93 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7240.h>
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1	    /* max number of memory banks */
#if (FLASH_SIZE == 4)
#define CFG_MAX_FLASH_SECT     64    /* max number of sectors on one chip */
#define CFG_FLASH_SIZE          0x00400000 /* Total flash size */
#elif(FLASH_SIZE == 8)
#define CFG_MAX_FLASH_SECT      128    /* max number of sectors on one chip */
#define CFG_FLASH_SIZE          0x00800000 /* Total flash size */
#elif(FLASH_SIZE == 16)
#define CFG_MAX_FLASH_SECT	256		/* max number of sectors on one chip */
#define CFG_FLASH_SIZE          0x01000000 /* Total flash size */
#endif
#define CFG_FLASH_SECTOR_SIZE   (64*1024)


//#define ENABLE_DYNAMIC_CONF 1
#define CONFIG_SUPPORT_AR7241 1

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#error "Invalid flash configuration"
#endif

#define CFG_FLASH_WORD_SIZE     unsigned short 

/* 
 * We boot from this flash
 */
#define CFG_FLASH_BASE		    0x9f000000

/*
 * Defines to change flash size on reboot
 */
#ifdef ENABLE_DYNAMIC_CONF
#define UBOOT_FLASH_SIZE         (0x20000)     // (128 * 1024)
#define UBOOT_ENV_SEC_START        (CFG_FLASH_BASE + UBOOT_FLASH_SIZE)
//#define UBOOT_TEXT_END		    __
#define CFG_FLASH_MAGIC           0xaabacada  
#define CFG_FLASH_MAGIC_F         (UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x20)
#define CFG_FLASH_SECTOR_SIZE_F   *(volatile int *)(CFG_FLASH_MAGIC_F + 0x4)
#define CFG_FLASH_SIZE_F          *(volatile int *)(CFG_FLASH_MAGIC_F + 0x8) /* Total flash size */
#define CFG_MAX_FLASH_SECT_F      (CFG_FLASH_SIZE / CFG_FLASH_SECTOR_SIZE) /* max number of sectors on one chip */
#else
#define UBOOT_ENV_SEC_END        (CFG_FLASH_BASE + CFG_FLASH_SIZE)
#define CFG_FLASH_MAGIC           	0xaabacada  
#define CFG_FLASH_MAGIC_F         (UBOOT_ENV_SEC_END  - 0x20)
#define CFG_FLASH_SECTOR_SIZE_F   *(volatile int *)(CFG_FLASH_MAGIC_F)
#define CFG_FLASH_SIZE_F          *(volatile int *)(CFG_FLASH_MAGIC_F + 0x4) /* Total flash size */
#define CFG_MAX_FLASH_SECT_F      (CFG_FLASH_SIZE / CFG_FLASH_SECTOR_SIZE) /* max number of sectors on one chip */
#endif


/* 
 * The following #defines are needed to get flash environment right 
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#undef CONFIG_BOOTARGS
/* XXX - putting rootfs in last partition results in jffs errors */
#if (FLASH_SIZE == 4)
#define	CONFIG_BOOTARGS     "console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),2752k(rootfs),896k(uImage),64k(NVRAM),64k(ART) REVISIONID"
#elif(FLASH_SIZE == 8)
#define CONFIG_BOOTARGS     "console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar7240-nor0:128k(u-boot),6720k(rootfs),1280k(Kernel),64k(ART)"
#elif(FLASH_SIZE == 16)
#define CONFIG_BOOTARGS     "console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar7240-nor0:128k(u-boot),14912k(rootfs),1280k(uImage),64k(ART)"
#endif


/* default mtd partition table */
#undef MTDPARTS_DEFAULT
#define MTDPARTS_DEFAULT    "mtdparts=ar7240-nor0:128k(u-boot),6720k(rootfs),1216k(uImage),64k(u-bootenv),64k(ART)"


#undef CFG_PLL_FREQ

#ifdef CONFIG_SUPPORT_AR7241 
//#define CFG_AR7241_PLL_FREQ	CFG_PLL_400_400_200
//#define CFG_AR7241_PLL_FREQ	CFG_PLL_450_450_200
//#define CFG_AR7241_PLL_FREQ	CFG_PLL_500_500_200
#endif

//defines to set cpu freq

#define CFG_PLL_FREQ	CFG_PLL_400_400_200
//#define CFG_PLL_FREQ	CFG_PLL_450_450_200
#undef CFG_HZ
#define CFG_PLL_OFFSET  CFG_FLASH_BASE+0x01fd00
#define CFG_PLL_MAGIC	0xC700
/*
 * MIPS32 24K Processor Core Family Software User's Manual
 *
 * 6.2.9 Count Register (CP0 Register 9, Select 0)
 * The Count register acts as a timer, incrementing at a constant
 * rate, whether or not an instruction is executed, retired, or
 * any forward progress is made through the pipeline.  The counter
 * increments every other clock, if the DC bit in the Cause register
 * is 0.
 */
/* Since the count is incremented every other tick, divide by 2 */
/* XXX derive this from CFG_PLL_FREQ */
#if (CFG_PLL_FREQ == CFG_PLL_200_200_100)
#   define CFG_HZ          (200000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_300_300_150)
#   define CFG_HZ          (300000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_350_350_175)
#   define CFG_HZ          (350000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_333_333_166)
#   define CFG_HZ          (333000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_133)
#   define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_66)
#   define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200) || (CFG_PLL_FREQ == CFG_PLL_400_400_100)
#   define CFG_HZ          (400000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_320_320_80) || (CFG_PLL_FREQ == CFG_PLL_320_320_160)
#   define CFG_HZ          (320000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_410_400_200)
#   define CFG_HZ          (410000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_420_400_200)
#   define CFG_HZ          (420000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_240_240_120)
#   define CFG_HZ          (240000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_160_160_80)
#   define CFG_HZ          (160000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_400_200_200)
#   define CFG_HZ          (400000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_450_450_200)
#   define CFG_HZ          (450000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_480_480_200)
#   define CFG_HZ          (480000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_500_500_200)
#   define CFG_HZ          (500000000/2)
#endif

/*
 * timeout values are in ticks
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000

// lsz 081222
#undef CFG_ENV_IS_IN_FLASH
#ifndef	SKIP_LOWLEVEL_INIT
#define	CFG_ENV_IS_IN_FLASH    1
#else
#undef CFG_ENV_IS_IN_FLASH
#endif
#if (CFG_ENV_IS_IN_FLASH==1)
#undef CFG_ENV_IS_NOWHERE
#define CFG_ENV_SIZE		CFG_FLASH_SECTOR_SIZE
#define CFG_ENV_OFFSET		0x7e0000
#define CFG_ENV_ADDR		0x9f7e0000    //CFG_FLASH_BASE+CFG_ENV_OFFSET
#else
#undef	CFG_ENV_IS_IN_FLASH
#define CFG_ENV_IS_NOWHERE
#define CFG_ENV_SIZE		CFG_FLASH_SECTOR_SIZE
#endif

#define CONFIG_BOOTCOMMAND "bootm 0x9f020000"

/* DDR init values */

#define CONFIG_NR_DRAM_BANKS	2

/* DDR values to support AR7241 */

#ifdef CONFIG_SUPPORT_AR7241 
#define CFG_7241_DDR1_CONFIG_VAL      0xc7bc8cd0
//#define CFG_7241_DDR1_CONFIG_VAL      0x6fbc8cd0
#define CFG_7241_DDR1_MODE_VAL_INIT   0x133
#define CFG_7241_DDR1_EXT_MODE_VAL    0x0
#define CFG_7241_DDR1_MODE_VAL        0x33
//#define CFG_7241_DDR1_MODE_VAL        0x23
#define CFG_7241_DDR1_CONFIG2_VAL	0x9dd0e6a8


#define CFG_7241_DDR2_CONFIG_VAL	0xc7bc8cd0
#define CFG_7241_DDR2_MODE_VAL_INIT	0x133
#define CFG_7241_DDR2_EXT_MODE_VAL	0x402
#define CFG_7241_DDR2_MODE_VAL		0x33
#define CFG_7241_DDR2_CONFIG2_VAL	0x9dd0e6a8
#endif /* _SUPPORT_AR7241 */

/* DDR settings for AR7240 */

#define CFG_DDR_REFRESH_VAL     0x4f10
#define CFG_DDR_CONFIG_VAL      0xc7bc8cd0
#define CFG_DDR_MODE_VAL_INIT   0x133
#ifdef LOW_DRIVE_STRENGTH
#       define CFG_DDR_EXT_MODE_VAL    0x2
#else
#       define CFG_DDR_EXT_MODE_VAL    0x0
#endif
#define CFG_DDR_MODE_VAL        0x33

#define CFG_DDR_TRTW_VAL        0x1f
#define CFG_DDR_TWTR_VAL        0x1e

#define CFG_DDR_CONFIG2_VAL      0x9dd0e6a8
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0x00ff

/* DDR2 Init values */
#define CFG_DDR2_EXT_MODE_VAL    0x402


#ifdef ENABLE_DYNAMIC_CONF
#define CFG_DDR_MAGIC           0xaabacada  
#define CFG_DDR_MAGIC_F         (UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x30)
#define CFG_DDR_CONFIG_VAL_F    *(volatile int *)(CFG_DDR_MAGIC_F + 4)
#define CFG_DDR_CONFIG2_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 8)
#define CFG_DDR_EXT_MODE_VAL_F  *(volatile int *)(CFG_DDR_MAGIC_F + 12)
#endif

#define CONFIG_NET_MULTI
#define CONFIG_AG7240_SPEPHY /* choose eth1 first for tftpboot interface added by tiger 07/20/09*/

#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI


#define CONFIG_IPADDR   192.168.1.32
#define CONFIG_SERVERIP 192.168.1.23
#if 0
#define CONFIG_ETHADDR 0x05:0x60:0x3F:0xEB:0x10:0x02
#else
#define CONFIG_ETHADDR 0x40:0x16:0x9F:0xAB:0xE0:0x82
#endif
#define CFG_FAULT_ECHO_LINK_DOWN    1


#define CFG_PHY_ADDR 0 
#define CFG_AG7240_NMACS 2
#define CFG_GMII     0
#define CFG_MII0_RMII             1
#define CFG_AG7100_GE0_RMII             1

#define CFG_BOOTM_LEN	(16 << 20) /* 16 MB */
/* lsz 081222
#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"
*/
#undef DEBUG
#undef CFG_HUSH_PARSER
#undef CFG_PROMPT_HUSH_PS2
/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/
#define UBOOT_OFFSET		    (0x000000)
#define MAC_OFFSET			    (0x01fc00)
#define PIN_OFFSET			    (0x01fe00)
#define KERNEL_OFFSET		    (0x020000)
#define ROOTFS_OFFSET		    (0x120000)
#define BOARDCAL                        (CFG_FLASH_BASE+0x001fc00)
#if(FLASH_SIZE ==4)
#define WLANCAL                          (CFG_FLASH_BASE+0x003f1000)
#define CONFIG_OFFSET		    (0x3e0000)
#define ART_OFFSET			    (0x3f0000)
#elif(FLASH_SIZE == 8)
#define WLANCAL				 (CFG_FLASH_BASE+0x007f1000)
#define CONFIG_OFFSET		    (0x7e0000)
#define ART_OFFSET			    (0x7f0000)
#elif(FLASH_SIZE == 16)
#define WLANCAL				 (CFG_FLASH_BASE+0x00ff1000)
#define CONFIG_OFFSET		    (0xfe0000)
#define ART_OFFSET			    (0xff0000)
#endif
#define ATHEROS_PRODUCT_ID              137
#define CAL_SECTOR                      (CFG_MAX_FLASH_SECT - 1)

/* For Merlin, both PCI, PCI-E interfaces are valid */
#define AR7240_ART_PCICFG_OFFSET        12

/* move from autoconf.h of kernel src, added by tiger 20091225 */
/* For GPIO setting
 * JumpStart LED
 */
#define CONFIG_GPIO_JUMPSTART_LED_BIT 0
#define CONFIG_GPIO_JUMPSTART_LED_ON 0

/*
 * Ready Status
 */
#define CONFIG_GPIO_READY_STATUS_BIT 1
#define CONFIG_GPIO_READY_STATUS_ON 0

/*
 * USB(TMS) LED
 */
#define CONFIG_GPIO_USB_LED_BIT 8
#define CONFIG_GPIO_USB_LED_ON 0

/*
 * Reset Factory Default
 */
#define CONFIG_GPIO_RESET_FAC_BIT 11
#define CONFIG_GPIO_FAC_RST_HOLD_TIME 5

/*
 * JumpStart Switch
 */
#define CONFIG_GPIO_JUMPSTART_SW_BIT 12

/*
 * Command line configuration.
 */
#ifndef SKIP_LOWLEVEL_INIT
#include <config_cmd_default.h>

//#undef CONFIG_CMD_FLASH	/* flinfo, erase, protect	*/
//#undef CONFIG_CMD_LOADB		/* loadb			*/
//#undef CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/

#undef CONFIG_CMD_AUTOSCRIPT	/* Autoscript Support		*/
#undef CONFIG_CMD_BDI	       	/* bdinfo			*/
#undef CONFIG_CMD_BOOTD		/* bootd			*/
//#undef CONFIG_CMD_CONSOLE	/* coninfo			*/
//#undef CONFIG_CMD_ECHO		/* echo arguments		*/
#ifndef CFG_ENV_IS_IN_FLASH
#undef CONFIG_CMD_ENV		/* saveenv			*/
#else
#undef CONFIG_CMD_CONSOLE	/* coninfo			*/
#undef CONFIG_CMD_ECHO		/* echo arguments		*/
//#undef CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#endif

#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/
#undef CONFIG_CMD_ITEST		/* Integer (and string) test	*/
#undef CONFIG_CMD_LOADB		/* loadb			*/
#undef CONFIG_CMD_LOADS		/* loads			*/
#undef CONFIG_CMD_MISC		/* Misc functions like sleep etc*/

#undef CONFIG_CMD_NFS		/* NFS support			*/
#undef CONFIG_CMD_RUN		/* run command in env variable	*/
#undef CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#undef CONFIG_CMD_XIMG		/* Load part of Multi Image	*/
#undef CONFIG_CMD_BEDBUG
#else
#include <config_cmd_default.h>

#undef CONFIG_CMD_ENV		/* saveenv	*/

//#define CONFIG_CMD_BMP		/* BMP support			*/
#define CONFIG_CMD_BSP		/* Board Specific functions	*/

#define CONFIG_CMD_CDP		/* Cisco Discovery Protocol 	*/



#define CONFIG_CMD_IMLS		/* List all found images	*/
#define CONFIG_CMD_IMMAP	/* IMMR dump support		*/

#define CONFIG_CMD_PCI		/* pciinfo			*/

#define CONFIG_CMD_PING		/* ping support			*/
#define CONFIG_CMD_PORTIO	/* Port I/O			*/
#define CONFIG_CMD_REGINFO	/* Register dump		*/

#define CONFIG_CMD_RUN		/* run command in env variable	*/

#define CONFIG_CMD_ELF		/* ELF (VxWorks) load/boot cmd	*/

//#define CONFIG_CMD_USB		/* USB Support			*/





#endif
#endif	/* __CONFIG_H */
