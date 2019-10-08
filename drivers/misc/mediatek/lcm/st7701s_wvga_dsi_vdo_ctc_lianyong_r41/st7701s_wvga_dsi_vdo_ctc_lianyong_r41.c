#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(800)

#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {	

{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
{0xD1,1,{0x11}},
{0x11,1,{0x00}},//SLP OUT
{REGFLAG_DELAY, 100, {}},
{0xFF,5,{0x77,0x01,0x00,0x00,0x10}},
{0xC0,2,{0x63,0x00}},
{0xC1,2,{0x0C,0x07}},
{0xC2,2,{0x01,0x08}},
{0xCC,1,{0x10}},
{0xB0,16,{0x40,0x02,0x87,0x0E,0x15,0x0A,0x03,0x0A,0x0A,0x18,0x08,0x16,0x13,0x07,0x09,0x19}},
{0xB1,16,{0x40,0x01,0x86,0x0D,0x13,0x09,0x03,0x0A,0x09,0x1C,0x09,0x15,0x13,0x91,0x16,0x19}},
{0xFF,5, {0x77,0x01,0x00,0x00,0x11}},
{0xB0,1, {0x1A}},//4D
{0xB1,1, {0x64}},
{0xB2,1, {0x07}},
{0xB3,1, {0x80}},
{0xB5,1, {0x47}},
{0xB7,1, {0x8A}},
{0xB8,1, {0x21}},
{0xB9,1, {0x33}},
{0xC1,1, {0x78}},
{0xC2,1, {0x78}},
{0xD0,1, {0x88}},
{0xE0,3, {0x00,0xB4,0x02}},
{0xE1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
{0xE2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xE3,4, {0x00,0x00,0x33,0x33}},
{0xE4,2, {0x44,0x44}},
{0xE5,16,{0x09,0x31,0xBE,0xA0,0x0B,0x31,0xBE,0xA0,0x05,0x31,0xBE,0xA0,0x07,0x31,0xBE,0xA0}},
{0xE6,4, {0x00,0x00,0x33,0x33}},
{0xE7,2, {0x44,0x44}},
{0xE8,16,{0x08,0x31,0xBE,0xA0,0x0A,0x31,0xBE,0xA0,0x04,0x31,0xBE,0xA0,0x06,0x31,0xBE,0xA0}},
{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},
{0xEB,7, {0x02,0x02,0x00,0x00,0x00,0x00,0x00}},
{0xEC,2, {0x02,0x00}},
{0xED,16,{0xF5,0x47,0x6F,0x0B,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xB0,0xF6,0x74,0x5F}},
{0xEF,12,{0x08,0x08,0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04}},
{0xFF,5, {0x77,0x01,0x00,0x00,0x00}},
	{0x29,0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}}    
};

/*

static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 200, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
            break;
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

	params->dbi.te_mode                     = LCM_DBI_TE_MODE_VSYNC_ONLY; 
	params->dbi.te_edge_polarity            = LCM_POLARITY_RISING;

	params->dsi.mode   = SYNC_PULSE_VDO_MODE ;//BURST_VDO_MODE ;//SYNC_EVENT_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 14;//20;//50
	params->dsi.vertical_frontporch					= 20;//40;//20
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 60;//42;
	params->dsi.horizontal_frontporch				= 56;//42;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=200;    //208 198
}


static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);
//	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	lcm_init();
	
//	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

static int adc_read_vol(void)
{
    int adc[1];
    int data[4] ={0,0,0,0};
    int sum = 0;
    int adc_vol=0;
    int num = 0;
    //	int count = 0;
    //re_read:
    for(num=0;num<10;num++)
    {
        IMM_GetOneChannelValue(1, data, adc);
        sum+=(data[0]*100+data[1]);
    }
    adc_vol = sum/10;
#if defined(BUILD_LK)
    printf("st7701s_wvga_dsi_vdo_ctc_lianyong_r41 test adc_vol is %d\n",adc_vol);
#else
    printk("st7701s_wvga_dsi_vdo_ctc_lianyong_r41 test adc_vol is %d\n",adc_vol);
#endif
    if(adc_vol>=0 && adc_vol<=100)
        return 1;
    else if(adc_vol>100)
        return 2;
	else
		return -1;
}



static unsigned int lcm_compare_id(void)
{
 	unsigned int id = 0;
    unsigned char buffer[3];
    unsigned int data_array[16];

	int id_adc = 0;
 
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(120);

	
	data_array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);  //dsi_set_cmdq(array, 3, 1);
	read_reg_v2(0xA1, buffer, 2);
    id_adc = adc_read_vol();
	id = buffer[0]<<8|buffer[1];
	#ifdef BUILD_LK
	printf("%s, LK st7701s_wvga_dsi_vdo_ctc_lianyong_r41 id = 0x%x%x%x\n", __func__, buffer[0],buffer[1],buffer[2]);//buffer[0]=0x83  buffer[1]=0x92  buffer[2]=0x0B
	#else
	printk("kevin: st7701s_wvga_dsi_vdo_ctc_lianyong_r41 id 0x04= 0x%x%x%x\n",buffer[0],buffer[1],buffer[2]);
	#endif
		
	if(id_adc==2&&0x8802 == id)
	{
		#ifdef BUILD_LK
	    printf("st7701s_wvga_dsi_vdo_ctc_lianyong_r41 return 1;");
	    #else
	    printk("st7701s_wvga_dsi_vdo_ctc_lianyong_r41 return 1;");
	    #endif
		return 1;
	}
	#ifdef BUILD_LK
    printf("st7701s_wvga_dsi_vdo_ctc_lianyong_r41 return 0;");
    #else
    printk("st7701s_wvga_dsi_vdo_ctc_lianyong_r41 return 0;");
    #endif
	return 0;
	
	//return 1;

}


LCM_DRIVER  st7701s_wvga_dsi_vdo_ctc_lianyong_r41_lcm_drv = 
{
    .name			= "st7701s_wvga_dsi_vdo_ctc_lianyong_r41",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,	
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};

