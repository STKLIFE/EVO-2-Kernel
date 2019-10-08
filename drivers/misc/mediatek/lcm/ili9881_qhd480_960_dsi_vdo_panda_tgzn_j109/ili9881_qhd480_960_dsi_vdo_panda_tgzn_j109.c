#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (960)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0XFD
#define REGFLAG_END_OF_TABLE      							0xFE   // END OF REGISTERS MARKER
#define LCM_ID	(0x9881)

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
{0xFF,3,{0x98,0x81,0x05}},
{0xB2,1,{0x70}},
{0x03,1,{0x00}},
{0x04,1,{0x24}},
{0x30,1,{0xF7}},
{0x29,1,{0x00}},
{0x2A,1,{0x12}},
{0x38,1,{0xA8}},
{0x1A,1,{0x50}},
{0x52,1,{0x5F}},
{0x54,1,{0x28}},
{0x55,1,{0x25}},
{0x26,1,{0x02}},
{0x3D,1,{0xA1}},
{0x1B,1,{0x01}},
{0x37,1,{0x17}},
{0xFF,3,{0x98,0x81,0x02}},
{0x42,1,{0x2F}},    
{0x01,1,{0x50}},    
{0x15,1,{0x10}},     
{0x50,1,{0x25}},    
{0x57,1,{0x00}},
{0x58,1,{0x17}},
{0x59,1,{0x26}},
{0x5A,1,{0x14}},
{0x5B,1,{0x17}},
{0x5C,1,{0x29}},
{0x5D,1,{0x1D}},
{0x5E,1,{0x1F}},
{0x5F,1,{0x8B}},
{0x60,1,{0x1E}},
{0x61,1,{0x2A}},
{0x62,1,{0x78}},
{0x63,1,{0x19}},
{0x64,1,{0x17}},
{0x65,1,{0x4B}},
{0x66,1,{0x20}},
{0x67,1,{0x27}},
{0x68,1,{0x4A}},
{0x69,1,{0x5A}},
{0x6A,1,{0x25}},
{0x6B,1,{0x00}},
{0x6C,1,{0x17}},
{0x6D,1,{0x26}},
{0x6E,1,{0x14}},
{0x6F,1,{0x17}},
{0x70,1,{0x29}},
{0x71,1,{0x1D}},
{0x72,1,{0x1F}},
{0x73,1,{0x8B}},
{0x74,1,{0x1E}},
{0x75,1,{0x2A}},
{0x76,1,{0x78}},
{0x77,1,{0x19}},
{0x78,1,{0x17}},
{0x79,1,{0x4B}},
{0x7A,1,{0x20}},
{0x7B,1,{0x27}},
{0x7C,1,{0x4A}},
{0x7D,1,{0x5A}},
{0x7E,1,{0x25}},
{0xFF,3,{0x98,0x81,0x01}},
{0x01,1,{0x00}},
{0x02,1,{0x00}},
{0x03,1,{0x56}},
{0x04,1,{0x13}},
{0x05,1,{0x13}},
{0x06,1,{0x0a}},
{0x07,1,{0x05}},
{0x08,1,{0x05}},
{0x09,1,{0x1D}},
{0x0a,1,{0x01}},
{0x0b,1,{0x00}},
{0x0c,1,{0x3F}},
{0x0d,1,{0x29}},
{0x0e,1,{0x29}},
{0x0f,1,{0x1D}},
{0x10,1,{0x1D}},
{0x11,1,{0x00}},
{0x12,1,{0x00}},
{0x13,1,{0x08}},
{0x14,1,{0x08}},
{0x15,1,{0x00}},
{0x16,1,{0x00}},
{0x17,1,{0x00}},
{0x18,1,{0x00}},
{0x19,1,{0x00}},
{0x1a,1,{0x00}},
{0x1b,1,{0x00}},
{0x1c,1,{0x00}},
{0x1d,1,{0x00}},
{0x1e,1,{0x40}},
{0x1f,1,{0x88}},
{0x20,1,{0x08}},
{0x21,1,{0x01}},
{0x22,1,{0x00}},
{0x23,1,{0x00}},
{0x24,1,{0x00}},
{0x25,1,{0x00}},
{0x26,1,{0x00}},
{0x27,1,{0x00}},
{0x28,1,{0x33}},
{0x29,1,{0x03}},
{0x2a,1,{0x00}},
{0x2b,1,{0x00}},
{0x2c,1,{0x00}},
{0x2d,1,{0x00}},
{0x2e,1,{0x00}},
{0x2f,1,{0x00}},
{0x30,1,{0x00}},
{0x31,1,{0x00}},
{0x32,1,{0x00}},
{0x33,1,{0x00}},
{0x34,1,{0x00}},
{0x35,1,{0x00}},
{0x36,1,{0x00}},
{0x37,1,{0x00}},
{0x38,1,{0x00}},
{0x39,1,{0x0f}},
{0x3a,1,{0x2a}},
{0x3b,1,{0xc0}},
{0x3c,1,{0x00}},
{0x3d,1,{0x00}},
{0x3e,1,{0x00}},
{0x3f,1,{0x00}},
{0x40,1,{0x00}},
{0x41,1,{0xe0}},
{0x42,1,{0x40}},
{0x43,1,{0x0f}},
{0x44,1,{0x31}},
{0x45,1,{0xa8}},
{0x46,1,{0x00}},
{0x47,1,{0x08}},
{0x48,1,{0x00}},
{0x49,1,{0x00}},
{0x4a,1,{0x00}},
{0x4b,1,{0x00}},
{0x4c,1,{0xb2}},
{0x4d,1,{0x22}},
{0x4e,1,{0x01}},
{0x4f,1,{0xf7}},
{0x50,1,{0x29}},
{0x51,1,{0x72}},
{0x52,1,{0x25}},
{0x53,1,{0xb2}},
{0x54,1,{0x22}},
{0x55,1,{0x22}},
{0x56,1,{0x22}},
{0x57,1,{0xa2}},
{0x58,1,{0x22}},
{0x59,1,{0x01}},
{0x5a,1,{0xe6}},
{0x5b,1,{0x28}},
{0x5c,1,{0x62}},
{0x5d,1,{0x24}},
{0x5e,1,{0xa2}},
{0x5f,1,{0x22}},
{0x60,1,{0x22}},
{0x61,1,{0x22}},
{0x62,1,{0xee}},
{0x63,1,{0x02}},
{0x64,1,{0x0b}},
{0x65,1,{0x02}},
{0x66,1,{0x02}},
{0x67,1,{0x01}},
{0x68,1,{0x00}},
{0x69,1,{0x0f}},
{0x6a,1,{0x07}},
{0x6b,1,{0x55}},
{0x6c,1,{0x02}},
{0x6d,1,{0x02}},
{0x6e,1,{0x5b}},
{0x6f,1,{0x59}},
{0x70,1,{0x02}},
{0x71,1,{0x02}},
{0x72,1,{0x57}},
{0x73,1,{0x02}},
{0x74,1,{0x02}},
{0x75,1,{0x02}},
{0x76,1,{0x02}},
{0x77,1,{0x02}},
{0x78,1,{0x02}},                          
{0x79,1,{0x02}},
{0x7a,1,{0x0a}},
{0x7b,1,{0x02}},
{0x7c,1,{0x02}},
{0x7d,1,{0x01}},
{0x7e,1,{0x00}},
{0x7f,1,{0x0e}},
{0x80,1,{0x06}},
{0x81,1,{0x54}},
{0x82,1,{0x02}},
{0x83,1,{0x02}},
{0x84,1,{0x5a}},
{0x85,1,{0x58}},
{0x86,1,{0x02}},
{0x87,1,{0x02}},
{0x88,1,{0x56}},
{0x89,1,{0x02}},
{0x8a,1,{0x02}},
{0x8b,1,{0x02}},
{0x8c,1,{0x02}},
{0x8d,1,{0x02}},
{0x8e,1,{0x02}},                      
{0x8f,1,{0x44}},
{0x90,1,{0x44}},
{0xFF,3,{0x98,0x81,0x06}},
{0x01,1,{0x03}},
{0x2B,1,{0x0A}},
{0x04,1,{0x73}}, 
{0xC0,1,{0xCF}},
{0xC1,1,{0x2A}},
{0xFF,3,{0x98,0x81,0x0B}},
{0x70,1,{0x11}},
{0xFF,3,{0x98,0x81,0x00}},        

{0x35,01,{0x00}},

{0x11, 0,{0x00}},

{REGFLAG_DELAY, 120, {}},

{0x29, 0,{0x00}},

{REGFLAG_DELAY, 50, {}},
	
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
	params->dsi.LANE_NUM				= LCM_TWO_LANE;	//LCM_TWO_LANE;//LCM_THREE_LANE;
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

	params->dsi.vertical_sync_active				= 4; //j2; //1;//2;
	params->dsi.vertical_backporch					= 16;  //j12 // from Q driver
	params->dsi.vertical_frontporch					= 20;  //j18; rom Q driver
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 20; //20; //j4; 5;//10; 
	params->dsi.horizontal_backporch				= 40; //50; //j20; //40; // from Q driver
	params->dsi.horizontal_frontporch				= 40; //50; //j24; //84;  // from Q driver
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	// Bit rate calculation
	params->dsi.PLL_CLOCK				= 212;

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
	unsigned int id=0;
	unsigned char buffer[3];
	unsigned int array[16];  

	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);	

	array[0]=0x00063902;
	array[1]=0x068198FF;
	dsi_set_cmdq(array, 2, 1);
	MDELAY(10); 

	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xF0, buffer, 1);   //98
	read_reg_v2(0xF1, buffer+1, 1); //81
     
	id = buffer[0]<<8 |buffer[1];
	
	 #ifdef BUILD_LK
           printf("%s, frank ILI9881 id = 0x%08x\n", __func__, id); 
        #else
           printk("%s, frank ILI9881 id = 0x%08x\n", __func__, id);   
        #endif
  return (id == LCM_ID)?1:0;




}

static void lcm_init(void)
{
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);
 	#ifdef BUILD_LK
           printf("%s, frank ILI9881 lcm_init\n", __func__); 
        #else
           printk("%s, frank ILI9881 lcm_init\n", __func__);   
        #endif
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
/*
		int array[4];	
		char buffer[3];	
		//char id_high=0;	
		//char id_low=0;	
		int id=0;

		SET_RESET_PIN(1);
		MDELAY(20);
		SET_RESET_PIN(0);	
		MDELAY(20);	
		SET_RESET_PIN(1);	
		MDELAY(100);

		array[0] = 0x00043902;
		array[1] = 0x018189FF;
		dsi_set_cmdq(array, 2, 1);

		array[0] = 0x00013700; //return byte number	
		dsi_set_cmdq(array, 1, 1);	
		read_reg_v2(0x00, &buffer[0], 1);  //0x98

		array[0] = 0x00013700; //return byte number	
		dsi_set_cmdq(array, 1, 1);	
		read_reg_v2(0x01, &buffer[1], 1);  //0x81	
		id = (buffer[0]<<8) | buffer[1];
    
        #ifdef BUILD_LK
           printf("%s, frank ILI9881 id = 0x%08x\n", __func__, id); 
        #else
           printk("%s, frank ILI9881 id = 0x%08x\n", __func__, id);   
        #endif
*/		

	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
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


LCM_DRIVER  ili9881_qhd480_960_dsi_vdo_panda_tgzn_j109_lcm_drv = 
{
    .name			= "ili9881_qhd480_960_dsi_vdo_panda_tgzn_j109",
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
