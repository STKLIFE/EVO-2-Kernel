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

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/	
{0x11,1,{0x00}},
{REGFLAG_DELAY, 200, {}},
{0xFF,5,{0x77,0x01,0x00,0x00,0x10}},
{0xC0,2,{0x63,0x00}},
{0xC1,2,{0x0C,0x07}},
{0xC2,2,{0x31,0x08}},
{0xCC,1,{0x10 }},
//----------------------------------Gamma Cluster Setting----------------------------------------//
{0xB0,16,{0x40,0x02,0x87,0x0E,0x15,0x0A,0x03,0x0a,0x0a,0x18,0x08,0x16,0x13,0x07,0x09,0x19}},
{0xB1,16,{0x40,0x01,0x86,0x0D,0x13,0x09,0x03,0x0A,0x09,0x1c,0x09,0x15,0x13,0x91,0x16,0x19}},
//------------------------------------End Gamma Setting-------------------------------------------//
//---------------------------------End Display Control setting-------------------------------------//
//--------------------------------------Bank0 Setting End------------------------------------------//
//----------------------------------------Bank1 Setting------------------------------------------------//
//----------------------------- Power Control Registers Initial -----------------------------------//
{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
{0xB0,1,{0x4D}},//0x4D
//----------------------------------------Vcom Setting------------------------------------------------//
{0xB1,1,{0x69}},
//--------------------------------------End Vcom Setting--------------------------------------------//
{0xB2,1,{0x07}},
{0xB3,1,{0x80}},
{0xB5,1,{0x47}},
{0xB7,1,{0x85}},
{0xB8,1,{0x21}},
{0xB9,1,{0x10}}, 
{0xC1,1,{0x78}},
{0xC2,1,{0x78}},
{0xD0,1,{0x88}},
//------------------------------End Power Control Registers Initial ----------------------------//
{REGFLAG_DELAY, 100, {}},  
//------------------------------------------GIP Setting-------------------------------------------------//
{0xE0,3,{0x00,0xB4,0x02}}, 
{0xE1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}}, 
{0xE2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}}, 
{0xE3,4,{0x00,0x00,0x33,0x33}}, 
{0xE4,2,{0x44,0x44}}, 
{0xE5,16,{0x09,0x31,0xBE,0xA0,0x0B,0x31,0xBE,0xA0,0x05,0x31,0xBE,0xA0,0x07,0x31,0xBE,0xA0}}, 
{0xE6,4,{0x00,0x00,0x33,0x33}}, 
{0xE7,2,{0x44,0x44}}, 
{0xE8,  16, {0x08,0x31,0xBE,0xA0,0x0A,0x31,0xBE,0xA0,0x04,0x31,0xBE,0xA0,0x06,0x31,0xBE,0xA0} },
{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}}, 

{0xEB,7,{0x02,0x02,0x00,0x00,0x00,0x00,0x00}}, 
{0xEC,2,{0x02,0x00}}, 
{0xED,16,{0xF5,0x47,0x6F,0x0B,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xB0,0xF6,0x74,0x5F}}, 
{0xEF,12,{0x08,0x08,0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04}},
{0x29,  1, {0x00}},
{REGFLAG_DELAY, 120, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}}    
};

/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
    {REGFLAG_DELAY, 20, {}},
};
*/
/*static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{REGFLAG_DELAY, 10, {}},
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
	{0x10, 0, {0x00}},
	    {REGFLAG_DELAY, 120, {}},
};*/
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
    //push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	lcm_init();
	//lcm_compare_id();
//	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

/*
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}


static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

*/


static unsigned int lcm_compare_id(void)
{
 	unsigned int id = 0;
    unsigned char buffer[3];
    unsigned int data_array[16];

 
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(120);

	
	data_array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);  //dsi_set_cmdq(array, 3, 1);
	read_reg_v2(0xA1, buffer, 2);
	id = buffer[0]<<8|buffer[1];
	#ifdef BUILD_LK
	printf("%s, LK st7701 id = 0x%x%x%x\n", __func__, buffer[0],buffer[1],buffer[2]);//buffer[0]=0x83  buffer[1]=0x92  buffer[2]=0x0B
	#else
	printk("kevin: st7701id 0x04= 0x%x%x%x\n",buffer[0],buffer[1],buffer[2]);
	#endif
	return ((0x8802 == id) ? 1:0);//st7701S idcode=0x8802
	//return ((0x7701 == id) ? 1:0);  //st7701 idcode=0x7701 
	//return 1;

}


LCM_DRIVER  st7701s_wvga_dsi_vdo_ctc_kxd_tck_lcm_drv = 
{
    .name			= "st7701s_wvga_dsi_vdo_ctc_kxd_tck",
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

