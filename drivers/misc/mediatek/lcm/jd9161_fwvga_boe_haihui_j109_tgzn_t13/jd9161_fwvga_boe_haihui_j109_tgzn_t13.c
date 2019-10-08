#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  									480
#define FRAME_HEIGHT 									960

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0XFD
#define REGFLAG_END_OF_TABLE      							0xFE   // END OF REGISTERS MARKER

#define LCM_ID_JD9161 										(0x0091)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

       

//#define LCM_DSI_CMD_MODE

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

{0xBF, 3, {0x91,0x61,0xF2}},
{0xB3, 2, {0x00,0x74}},
{0xB4, 2, {0x00,0x7A}},
{0xB8, 6, {0x00,0xA0,0x01,0x00,0xA0,0x01}},
{0xBA, 3, {0x34,0x23,0x00}},

{0xC3, 1, {0x04}},
{0xC4, 2, {0x00,0x78}},
{0xC7, 9, {0x00,0x02,0x32,0x07,0x65,0x2B,0x19,0xA5,0xA5}},

{0xC8, 38, {0x6C,0x60,0x55,0x4C,0x4B,0x3D,0x42,0x2A,0x40,0x3C,0x39,0x55,0x42,0x44,0x33,0x30,0x22,0x11,0x06,0x6C,0x60,0x55,0x4C,0x4B,0x3D,0x42,0x2A,0x40,0x3C,0x39,0x55,0x42,0x44,0x33,0x30,0x22,0x11,0x06}},

{0xD4, 16, {0x1F,0x1F,0x00,0x02,0x1F,0x1E,0x1E,0x08,0x0A,0x04,0x06,0x1F,0x1F,0x1F,0x1F,0x1F}},

{0xD5, 16, {0x1F,0x1F,0x01,0x03,0x1F,0x1E,0x1E,0x09,0x0B,0x05,0x07,0x1F,0x1F,0x1F,0x1F,0x1F}},

{0xD6, 16, {0x1F,0x1E,0x03,0x01,0x1F,0x1F,0x1E,0x07,0x05,0x0B,0x09,0x1F,0x1F,0x1F,0x1F,0x1F}},

{0xD7, 16, {0x1F,0x1E,0x02,0x00,0x1F,0x1F,0x1E,0x06,0x04,0x0A,0x08,0x1F,0x1F,0x1F,0x1F,0x1F}},

{0xD8, 20, {0x20,0x00,0x00,0x30,0x04,0x30,0x01,0x02,0x30,0x01,0x02,0x06,0x7C,0x73,0xC8,0x73,0x06,0x06,0x7C,0x08}},

{0xD9, 19, {0x00,0x0A,0x0A,0x88,0x00,0x00,0x06,0x7B,0x00,0x80,0x00,0x3B,0x33,0x1F,0x00,0x00,0x00,0x06,0x70}},

{0xBE, 1, {0x01}},
{0xC1, 1, {0x10}},
{0xC5, 1, {0x05}},

{0xCC, 10, {0x34,0x20,0x38,0x60,0x11,0x91,0x00,0x40,0x00,0x00}},

{0xBE, 1, {0x00}},

{0x11, 0, {0x00}},
{REGFLAG_DELAY,120,{}},

{0x29, 0, {0x00}},
{REGFLAG_DELAY,10,{}},

{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
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

	// enable tearing-free
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if defined(LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
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

	params->dsi.DSI_WMEM_CONTI=0x3C; 
	params->dsi.DSI_RMEM_CONTI=0x3E; 

		
	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

#if 1
		params->dsi.vertical_sync_active				= 4;
		params->dsi.vertical_backporch					= 11;//18
		params->dsi.vertical_frontporch					= 18;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 50;
		params->dsi.horizontal_frontporch				= 50;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK				= 190;
#else
	params->dsi.vertical_sync_active				= 3;
	params->dsi.vertical_backporch					= 12;
	params->dsi.vertical_frontporch					= 3;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 63;
	params->dsi.horizontal_frontporch				= 63;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	// Bit rate calculation
	params->dsi.PLL_CLOCK				= 252;
#endif

	params->dsi.clk_lp_per_line_enable = 0;
//	params->dsi.esd_check_enable = 1;
//	params->dsi.customization_esd_check_enable = 0;
//	params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
//	params->dsi.lcm_esd_check_table[0].count        = 1;
//	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
}
#if 0
static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xBF,	3,	{0x92, 0x61, 0xf3}},
	{REGFLAG_DELAY, 10, {}},

        // Sleep Mode On
        // {0xC3, 1, {0xFF}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
static unsigned int lcm_compare_id(void)
{

//	int status = 0;
	int array[4];
	char buffer[5];

	//char id_high=0;
	//char id_midd=0;
	//char id_low=0;
	int id=0;
	   

/*
	reset_lcd_module(1);
	MDELAY(5);
	reset_lcd_module(0);
	MDELAY(9);
	reset_lcd_module(1);
	MDELAY(20);
	*/

	array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0x04, buffer, 1);
	
	id = buffer[0]; //we only need ID
	

#if defined(BUILD_LK)
	printf("\n\ jd9161  id1 = 0x%08x\n", id);
#else
	printk("jd9161 id1 = 0x%08x\n", id);
#endif


    return (id == LCM_ID_JD9161)?1:0;
}

static void lcm_init(void)
{
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}



static void lcm_suspend(void)
{
	unsigned int data_array[2];

	data_array[0] = 0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10); 
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(100);

#ifdef BUILD_LK
	printf("uboot %s\n", __func__);
#else
	printk("kernel %s\n", __func__);
#endif
}


static void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("uboot %s\n", __func__);
#else
	printk("kernel %s\n", __func__);
#endif
//	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
	lcm_init();
}

#ifdef LCM_DSI_CMD_MODE
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
	
#ifdef BUILD_LK
	printf("jd9161_fwvga_boe_hanlong_j109_tgzn_t13 uboot %s\n", __func__);
#else
	printk("jd9161_fwvga_boe_hanlong_j109_tgzn_t13 kernel %s\n", __func__);	
#endif

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}
#endif


LCM_DRIVER  jd9161_fwvga_boe_haihui_j109_tgzn_t13_lcm_drv = 
{
    .name			= "jd9161_fwvga_boe_haihui_j109_tgzn_t13",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id     = lcm_compare_id,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if defined(LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
