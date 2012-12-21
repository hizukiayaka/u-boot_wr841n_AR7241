#include <common.h>
#include <jffs2/jffs2.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include "ar7240_soc.h"
#include "ar7240_flash.h"

/*
 * globals
 */
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#undef display
#define display(x)  ;

/*
 * statics
 */
static void ar7240_spi_write_enable(void);
static void ar7240_spi_poll(void);
static void ar7240_spi_write_page(uint32_t addr, uint8_t *data, int len);
static void ar7240_spi_sector_erase(uint32_t addr);

static unsigned int
read_id(void)
{
    u32 flashid;
    ar7240_reg_wr_nf(AR7240_SPI_FS, 1);             
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
    ar7240_spi_bit_banger(0x9f);
    ar7240_spi_delay_8();
    ar7240_spi_delay_8();
    ar7240_spi_delay_8();
    ar7240_spi_delay_8();
    flashid = ar7240_reg_rd(AR7240_SPI_RD_STATUS); 
    ar7240_spi_done();
	
    // printf("->Oops: flash id 0x%x . \n", flashid);
     return flashid;
}

unsigned long 
flash_init (void)
{
    u32 rd;

    ar7240_reg_wr_nf(AR7240_SPI_CLOCK, 0x43);
    rd= read_id();
    flash_info[0].flash_id=(rd>>8);
    /*
     * hook into board specific code to fill flash_info
     */
    return (flash_get_geom(&flash_info[0]));
}


void flash_print_info (flash_info_t *info)
{

		//ar7240_LED_tftp();
		
		printf("\nFlash MF_ID:[0x%x]\n",flash_info[0].flash_id>>16);
		printf("Dev_ID:[0x%x]\n",(flash_info[0].flash_id>>8)&0xff);
		printf("Size_ID:[0x%x]\n",(flash_info[0].flash_id)&0xff);
		printf("Flash Size:[%dMByte]\n",flash_info[0].size>>20);
		printf("Flash has [%d] Sectors\n",flash_info[0].sector_count);
}

int
flash_erase(flash_info_t *info, int s_first, int s_last)
{
    int i, sector_size = info->size/info->sector_count;

	/*for (i=s_first; i<= s_last; i++) {
			if ((info->protect[i] != 0) ) {
				printf("\nERR!! PROTECTED SECTORS\n");
				return (ERR_PROTECTED);
			}
		}*/
	
    printf("\nFirst %#x last %#x sector size %#x\n",
           s_first, s_last, sector_size);

    for (i = s_first; i <= s_last; i++) {
        printf("\b\b\b\b%4d", i);
        ar7240_spi_sector_erase(i * sector_size);
    }
    ar7240_spi_done();
    printf("\n");

    return 0;
}

/*
 * Write a buffer from memory to flash:
 * 0. Assumption: Caller has already erased the appropriate sectors.
 * 1. call page programming for every 256 bytes
 */
int 
write_buff(flash_info_t *info, uchar *source, ulong addr, ulong len)
{
    int total = 0, len_this_lp, bytes_this_page;
    ulong dst;
    uchar *src;
    
    printf ("write addr: %x\n", addr); 
    addr = addr - CFG_FLASH_BASE;

    while(total < len) {
        src              = source + total;
        dst              = addr   + total;
        bytes_this_page  = AR7240_SPI_PAGE_SIZE - (addr % AR7240_SPI_PAGE_SIZE);
        len_this_lp      = ((len - total) > bytes_this_page) ? bytes_this_page
                                                             : (len - total);
        ar7240_spi_write_page(dst, src, len_this_lp);
        total += len_this_lp;
    }

    ar7240_spi_done();

    return 0;
}

static void
ar7240_spi_write_enable()  
{
    ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
    ar7240_spi_bit_banger(AR7240_SPI_CMD_WREN);             
    ar7240_spi_go();
}

static void
ar7240_spi_poll()   
{
    int rd;                                                 

    do {
        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
        ar7240_spi_bit_banger(AR7240_SPI_CMD_RD_STATUS);        
        ar7240_spi_delay_8();
        rd = (ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 1);               
    }while(rd);
}

static void
ar7240_spi_write_page(uint32_t addr, uint8_t *data, int len)
{
    int i;
    uint8_t ch;

    display(0x77);
    ar7240_spi_write_enable();
    ar7240_spi_bit_banger(AR7240_SPI_CMD_PAGE_PROG);
    ar7240_spi_send_addr(addr);

    for(i = 0; i < len; i++) {
        ch = *(data + i);
        ar7240_spi_bit_banger(ch);
    }

    ar7240_spi_go();
    display(0x66);
    ar7240_spi_poll();
    display(0x6d);
}

static void
ar7240_spi_sector_erase(uint32_t addr)
{
    ar7240_spi_write_enable();
    ar7240_spi_bit_banger(AR7240_SPI_CMD_SECTOR_ERASE);
    ar7240_spi_send_addr(addr);
    ar7240_spi_go();
    display(0x7d);
    ar7240_spi_poll();
}


