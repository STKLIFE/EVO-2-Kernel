#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/upmu_common.h>
	#include <platform/upmu_hw.h>

	#include <platform/mt_gpio.h>
	#include <platform/mt_i2c.h>
	#include <platform/mt_pmic.h>
	#include <string.h>
#else
#ifdef CONFIG_MTK_LEGACY
	#include <mach/mt_pm_ldo.h>	/* hwPowerOn */
	#include <mach/upmu_common.h>
	#include <mach/upmu_sw.h>
	#include <mach/upmu_hw.h>
	#include <mach/mt_gpio.h>
#endif
#endif

#ifdef CONFIG_MTK_LEGACY
#include <cust_gpio_usage.h>
#ifndef CONFIG_FPGA_EARLY_PORTING
#include <cust_i2c.h>
#endif
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  			(720)
#define FRAME_HEIGHT 			(1280)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xDD   // END OF REGISTERS MARKER

#define LCM_ID_OTM9806E	0x9816


#define LCM_DSI_CMD_MODE			0

#ifndef TRUE
    #define   TRUE     1
#endif

#ifndef FALSE
    #define   FALSE    0
#endif

#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#if defined(BUILD_UBOOT)
	#define LCM_PRINT printf
#else
	#define LCM_PRINT printk
#endif
#endif

#define LCM_DBG(fmt, arg...) \
	LCM_PRINT("[LCM_ILI9806C _FWVGA_DSI_VDO_TXD] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)


// ---------------------------------------------------------------------------
//#define FRAME_WIDTH  (720)
//#define FRAME_HEIGHT (1440)
// Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    	(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 		(lcm_util.udelay(n))
#define MDELAY(n) 		(lcm_util.mdelay(n))

static unsigned int lcm_compare_id(void);
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(para_tbl, size, force_update)   	lcm_util.dsi_set_cmdq_V3(para_tbl, size, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
unsigned char cmd;
unsigned char count;
unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {

{0xB9,3,{0xF1,0x12,0x83}},
{0xBA,27,{0x32,0x81,0x05,0xF9,0x0E,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x25,0x00,0x91,0x0A,0x00,0x00,0x02,0x4F,0xD1,0x00,0x00,0x37}},
{0xB8,2,{0x25,0x22}},
{0xBF,3,{0x02,0x11,0x00}},
{0xB3,10,{0x0C,0x10,0x0A,0x50,0x03,0xFF,0x00,0x00,0x00,0x00}},
{0xC0,9,{0x73,0x73,0x50,0x50,0x00,0x00,0x08,0x70,0x00}},
{0xBC,1,{0x46}},
{0xCC,1,{0x0B}},
{0xB4,1,{0x80}},
{0xB2,3,{0xC8,0x12,0x30}},
{0xE3,14,{0x07,0x07,0x0B,0x0B,0x03,0x0B,0x00,0x00,0x00,0x00,0xFF,0x00,0x40,0x10}},
{0xC1,12,{0x53,0x00,0x1E,0x1E,0x77,0xC1,0xFF,0xFF,0xAF,0xAF,0x7F,0x7F}},
{0xB5,2,{0x07,0x07}},
{0xB6,2,{0x70,0x70}},
{0xC6,5,{0x00,0x00,0xFF,0xFF,0x01,0xFF}},
{0xE9,63,{0XC2,0x10,0x05,0x04,0xFE,0x02,0xA1,0x12,0x31,0x45,0x3F,0x83,0x12,0xB1,0x3B,0x2A,0x08,0x05,0x00,0x00,0x00,0x00,0x08,0x05,0x00,0x00,0x00,0x00,0xFF,0x02,0x46,0x02,0x48,0x68,0x88,0x88,0x88,0x80,0x88,0xFF,0x13,0x57,0x13,0x58,0x78,0x88,0x88,0x88,0x81,0x88,0x00,0x00,0x00,0x00,0x00,0x12,0xB1,0x3B,0x00,0x00,0x00,0x00,0x00}},
{0xEA,61,{0x00,0x1A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x31,0x75,0x31,0x18,0x78,0x88,0x88,0x88,0x85,0x88,0xFF,0x20,0x64,0x20,0x08,0x68,0x88,0x88,0x88,0x84,0x88,0x20,0x10,0x00,0x00,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x30,0x02,0xA1,0x00,0x00,0x00,0x00}},
{0xE0,34,{0x00,0x05,0x07,0x1A,0x39,0x3F,0x33,0x2C,0x06,0x0B,0x0D,0x11,0x13,0x12,0x14,0x10,0x1A,0x00,0x05,0x07,0x1A,0x39,0x3F,0x33,0x2C,0x06,0x0B,0x0D,0x11,0x13,0x12,0x14,0x10,0x1A}},

{0x11,1,{0x00}}, ////Sleep Out
{REGFLAG_DELAY, 250, {}},
{0x29,1,{0x00}}, ///Display On
{REGFLAG_DELAY, 50, {}},

{REGFLAG_END_OF_TABLE, 0x00, {}}

};


#if 0
static struct LCM_setting_table lcm_sleep_out_setting[] = {
	//{0x13, 1, {0x00}},		 //add from mt6572
	//{REGFLAG_DELAY,20,{}},  //add from mt6572
    // Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Sleep Mode On

	{0x28, 1, {0x00}},
	{REGFLAG_DELAY,200,{}},

	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	/*
	SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);
	*/
//	 {0xff,6,{0x77,0x01,0x00,0x00,0x11,0x80}},
//	  {0xff,5,{0x77,0x01,0x00,0x00,0x91}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct LCM_setting_table lcm_backlight_level_setting[] = {

};
static struct LCM_setting_table lcm_compare_id_setting[] = {

	{0xD3,	3,	{0xFF, 0x83, 0x79}},
	{REGFLAG_DELAY, 10, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

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
    memcpy((void*)&lcm_util, (void*)util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS * params)

{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

		// enable tearing-free
		//params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

	params->dsi.mode   = BURST_VDO_MODE;//SYNC_PULSE_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_THREE_LANE;//LCM_FOUR_LANE
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
		params->dsi.word_count=480*3;

	params->dsi.vertical_sync_active				= 4;//3; //4
	params->dsi.vertical_backporch					=21;//10; //18 //8
	params->dsi.vertical_frontporch					= 17;//16; //16 //8
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;//50;
	params->dsi.horizontal_backporch				= 50;//60;85
	params->dsi.horizontal_frontporch				= 50;//60;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=208;//218
/*
	params->dsi.vertical_sync_active				= 4; //4
	params->dsi.vertical_backporch					= 20; //18 //8
	params->dsi.vertical_frontporch					= 18; //16 //8
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;//50;
	params->dsi.horizontal_backporch				= 85;//60;85
	params->dsi.horizontal_frontporch				= 85;//60;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=200;//218


	params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;
    params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
    params->dsi.lcm_esd_check_table[0].count        = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
*/
}




static unsigned int lcm_compare_id(void)
{


    int id=0;
	unsigned char buffer[4];
	unsigned int array[16];

    char id_high=0;
    char id_low=0;

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

#ifdef BUILD_LK
   DSI_clk_HS_mode(0);
 #endif
	array[0] = 0x00023700; //
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x04, &buffer[0], 2);

    id_high=buffer[0];
    id_low=buffer[1];
    id=(id_high<<8)| id_low;
		#ifdef BUILD_LK
		printf("st7703_hd720_dsi_vdo_cmi_s9b_kxd_wzyh55 %s:0x%2x,0x%2x,0x%2x,0x%2x id=0x%x\n", __func__,buffer[0],buffer[1],buffer[2],buffer[3], id);
	#else
		printk("st7703_hd720_dsi_vdo_cmi_s9b_kxd_wzyh55 %s:0x%2x,0x%2x,0x%2x,0x%2x id=0x%x\n", __func__,buffer[0],buffer[1],buffer[2],buffer[3], id);
	#endif
    return ((0x3821 == id) ? 1:0);

}



static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);
	//LCD_PRT("\n");
    //lcm_init_registers();
   #ifdef BUILD_LK
		printf("st7703_hd720_dsi_vdo_cmi_s9b_kxd_wzyh55 lcm_init\n");
	#else
		printk("st7703_hd720_dsi_vdo_cmi_s9b_kxd_wzyh55 lcm_init\n");
	#endif
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
//	LCD_PRT("\n");
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
    

}

static void lcm_resume(void)
{


	//lcm_initialization_setting[10].para_list[0]-=1;

//	lcm_compare_id();
    lcm_init();
	//lcm_initialization_setting[10].para_list[0]+=3;
    //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


#if (LCM_DSI_CMD_MODE)
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
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif


// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER st7703_hd720_dsi_vdo_cmi_s9b_kxd_wzyh55_lcm_drv=
{
	.name		= "st7703_hd720_dsi_vdo_cmi_s9b_kxd_wzyh55",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};


