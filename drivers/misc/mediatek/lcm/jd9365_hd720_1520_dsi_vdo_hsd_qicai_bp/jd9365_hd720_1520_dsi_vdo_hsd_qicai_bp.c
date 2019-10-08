#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1520)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0XFD
#define REGFLAG_END_OF_TABLE      							0xFE   // END OF REGISTERS MARKER
#define LCM_ID     (0x9261)

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
	
{0xDF,3,{0x93,0x65,0xF8}},
{0xCC,1,{0x32}}, // 33=4LANE; 32=3LANE
{0xB0,7,{0x10,0x13,0x06,0x00,0x64,0x06,0x01}},
{0xB2,2,{0x00,0x41}}, //VCOM
{0xB3,2,{0x00,0x41}}, //VCOM_R
{0xB5,2,{0x64,0x84}},
{0xB7,6,{0x00,0xBF,0x00,0x00,0xBF,0x00}},
{0xB9,4,{0x00,0x04,0x13,0x07}},
{0xBB,11,{0x02,0x01,0x24,0x00,0x2F,0x13,0x28,0x04,0xDD,0xDD,0xDD}},
{0xBC,2,{0x0F,0x04}},
{0xBE,2,{0x1E,0xF2}},
{0xC0,2,{0x26,0x03}},
{0xC1,2,{0x00,0x12}},
{0xC3,7,{0x04,0x02,0x02,0x66,0x01,0x80,0x80}},
{0xC4,9,{0x24,0xF8,0xB4,0x81,0x12,0x0F,0x16,0x00,0x00}},
{0xC8,38,{0x6C,0x4B,0x3B,0x2F,0x2D,0x1F,0x27,0x13,0x2F,0x30,0x32,0x53,0x45,0x4E,0x41,0x3D,0x39,0x0D,0x08,0x6C,0x4B,0x3B,0x2F,0x2D,0x1F,0x27,0x13,0x2F,0x30,0x32,0x53,0x45,0x4E,0x41,0x3D,0x39,0x0D,0x08}},
{0xD0,22,{0x1E,0x1F,0x57,0x58,0x44,0x46,0x48,0x4A,0x40,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x50,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}},
{0xD1,22,{0x1E,0x1F,0x57,0x58,0x45,0x47,0x49,0x4B,0x41,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x51,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}},
{0xD2,22,{0x1F,0x1E,0x17,0x18,0x0B,0x09,0x07,0x05,0x11,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x01,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}},
{0xD3,22,{0x1F,0x1E,0x17,0x18,0x0A,0x08,0x06,0x04,0x10,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}},
{0xD4,22,{0x00,0x00,0x00,0x04,0x0B,0x30,0x01,0x02,0x00,0x58,0x55,0xFF,0x30,0x03,0x04,0x00,0x58,0x73,0x0D,0x00,0x58,0x00}},
{0xD5,17,{0x10,0x08,0x08,0x08,0x30,0x00,0x00,0x00,0x00,0x00,0xBC,0x50,0x00,0x06,0x12,0x18,0x60}},
{0xDD,3,{0x2C,0xA3,0x00}},
{0xDE,1,{0x02}},
{0xB2,2,{0x32,0x1C}},
{0xB7,4,{0x1F,0x00,0x00,0x04}},
{0xC1,1,{0x11}},
{0xBB,6,{0x21,0x22,0x23,0x24,0x36,0x37}},
{0xC2,4,{0x20,0x38,0x1E,0x84}},
{0xDE,1,{0x00}},
{0x35,1,{0x00}},

{0x11, 0, {0x00}},
{REGFLAG_DELAY,120,{}},

{0x29, 0, {0x00}},
{REGFLAG_DELAY,50,{}},


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
	params->dsi.LANE_NUM				= LCM_THREE_LANE;	//LCM_TWO_LANE;//LCM_THREE_LANE;
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
	params->dsi.vertical_sync_active				= 3;
	params->dsi.vertical_backporch					= 14;
	params->dsi.vertical_frontporch					= 18;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 20;
	params->dsi.horizontal_frontporch				= 90;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	// Bit rate calculation
	params->dsi.PLL_CLOCK				= 314;
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

}

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xBF,	3,	{0x92, 0x61, 0xf3}},
	{REGFLAG_DELAY, 10, {}},

        // Sleep Mode On
        // {0xC3, 1, {0xFF}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	
	{0x28,0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{0x10,0, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	
};
static unsigned int lcm_compare_id(void)
{
	#if 1
		unsigned int id = 0;
        unsigned char buffer[2];
        unsigned int array[16];

        SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
        SET_RESET_PIN(0);
        MDELAY(10);
        SET_RESET_PIN(1);
        MDELAY(10);
    
        push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
    
        array[0] = 0x00023700;// read id return two byte,version and id
        dsi_set_cmdq(array, 1, 1);
        //id = read_reg(0xF4);
        read_reg_v2(0xDF, buffer, 2);
        //id = buffer[0]; //we only need ID
        id = (buffer[0] << 8) | buffer[1];
        
        #ifdef BUILD_LK
           printf("%s, id1 = 0x%08x\n", __func__, id); 
        #else
           printk("%s, id1 = 0x%08x\n", __func__, id);   
        #endif
	#endif		
        return 1;//(LCM_ID == id)?1:0;
}

static void lcm_init(void)
{
#ifdef BUILD_LK
	printf("jd9365_hd720_dsi_vdo_hsd_haifei_tgzn uboot %s\n", __func__);
#else
	printk("jd9365_hd720_dsi_vdo_hsd_haifei_tgzn kernel %s\n", __func__);
#endif
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}



static void lcm_suspend(void)
{

push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
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
	printf("uboot %s\n", __func__);
#else
	printk("kernel %s\n", __func__);	
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


LCM_DRIVER  jd9365_hd720_1520_dsi_vdo_hsd_qicai_bp_lcm_drv = 
{
    .name			= "jd9365_hd720_1520_dsi_vdo_hsd_qicai_bp",
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
