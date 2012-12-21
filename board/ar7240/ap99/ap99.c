#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include "ar7240_soc.h"

extern void ar7240_ddr_initial_config(uint32_t refresh);
extern int ar7240_ddr_find_size(void);


void ar7240_all_led_on(){
	int m;
	ar7240_reg_wr_nf(AR7240_GPIO_OE,ar7240_reg_rd(AR7240_GPIO_OE)|0x3e003);
	for(m=0;m<3;m++){
				ar7240_reg_wr_nf(AR7240_GPIO_CLEAR,ar7240_reg_rd(AR7240_GPIO_SET)|0x3e003);				
				udelay(200000);
				ar7240_reg_wr_nf(AR7240_GPIO_SET,ar7240_reg_rd(AR7240_GPIO_CLEAR)|0x3e003);
				udelay(200000);
			}	
}
void ar7240_all_led_off(){
	ar7240_reg_wr_nf(AR7240_GPIO_OUT,ar7240_reg_rd(AR7240_GPIO_OUT)|0x7f8);	
}
int ar7240_gpio_in_val(int gpio)
{
    return((1 << gpio) & (ar7240_reg_rd(AR7240_GPIO_IN)));
}

void
ar7240_usb_initial_config(void)
{
	ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0a04081e);
	ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0804081e);
}

/* make switch LED control independently modified by tiger 20091225 */
void ar7240_gpio_config(void)
{
	int index;
	/* Disable clock obs */
	ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & 0xffe7e0ff));
	/* Enable eth Switch LEDs */
#ifdef CONFIG_K31
	ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xd8));
#else
	ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xfa));
#endif
	ar7240_reg_rmw_clear(AR7240_GPIO_OE, (1 << CONFIG_GPIO_JUMPSTART_SW_BIT));
}
/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/////////////////////////need to be figure////////////////////////////
void ar7240_LED_tftp(void){
	int m;
	//u32 reg;
	
			ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & 0x00048003));
			ar7240_reg_wr_nf(AR7240_GPIO_OE,ar7240_reg_rd(AR7240_GPIO_OE)|0x3e003);
			for(m=0;m<3;m++){
				ar7240_reg_wr_nf(AR7240_GPIO_SET,ar7240_reg_rd(AR7240_GPIO_SET)|0x3e003);				
				udelay(200000);
				ar7240_reg_wr_nf(AR7240_GPIO_CLEAR,ar7240_reg_rd(AR7240_GPIO_CLEAR)|0x3e003);
				udelay(200000);
			}	
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
int
ar7240_mem_config(void)
{
	u32 cpufreq,ddrfreq,ahbfreq;
	unsigned int tap_val1, tap_val2;
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);
    ar7240_reg_wr_nf(AR7240_GPIO_SET,ar7240_reg_rd(AR7240_GPIO_CLEAR)|0x3e003);

    /* Default tap values for starting the tap_init*/
    if (!(is_ar7241() || is_ar7242()))  {
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x7);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x7);
    }
    else {
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x2);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x2);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL2, 0x0);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL3, 0x0);
    	}
    #ifndef SKIP_LOWLEVEL_INIT
	 ar7240_ddr_tap_init();
   #endif
    tap_val1 = ar7240_reg_rd(0xb800001c);
    tap_val2 = ar7240_reg_rd(0xb8000020);
    printf("#### TAP VALUE 1 = 0x%x, 2 = 0x%x [0x%x: 0x%x]\n",
                tap_val1, tap_val2, *(unsigned *)0x80500000,
                *(unsigned *)0x80500004); 
   
    /* move ddr tap init after GPIO config for LED blink problem moved by tiger 20091225 */

/*	printf("GPIO_OE=%x_____CLEAR=%x___SET=%x____IN=%x____OUT=%x___INTEN=%x___INTMSK=%x___INTTYP%x\n", \
	ar7240_reg_rd(AR7240_GPIO_OE),ar7240_reg_rd(AR7240_GPIO_CLEAR), \
	ar7240_reg_rd(AR7240_GPIO_SET),ar7240_reg_rd(AR7240_GPIO_IN),\
	ar7240_reg_rd(AR7240_GPIO_OUT),ar7240_reg_rd(AR7240_GPIO_INT_ENABLE),\
	ar7240_reg_rd(AR7240_GPIO_INT_MASK),ar7240_reg_rd(AR7240_GPIO_INT_TYPE));*/


    /* this function is also called when is_ar7241, current test is OK but we need datasheet to make sure */
    ar7240_sys_frequency(&cpufreq,& ddrfreq,& ahbfreq);
    printf("CPU_FREQ=%d##DDR_FREQ=%d##AHB_FREQ=%d\n",cpufreq/1000000,ddrfreq/1000000,ahbfreq/1000000);
   
   ar7240_usb_initial_config();
   ar7240_gpio_config();
    return (ar7240_ddr_find_size());
}

long int initdram(int board_type)
{
	return (ar7240_mem_config());
}

int checkboard (void)
{
    if ((is_ar7241() || is_ar7242()))
	printf("AP99 (ar7241 - Virian) U-boot\n");
    else
	printf("AP99 (ar7240 - Python) U-boot\n");

	return 0;
}
