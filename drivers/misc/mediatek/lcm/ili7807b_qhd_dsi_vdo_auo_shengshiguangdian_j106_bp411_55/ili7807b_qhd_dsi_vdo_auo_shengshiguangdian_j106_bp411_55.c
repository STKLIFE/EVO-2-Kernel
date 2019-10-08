#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (540)
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


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

       
struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,3,{0x78,07,01}},
{0x0A,1,{0x01}},       //01:2lanes; 02:3lanes; 03:4lanes
{0x42,1,{0x11}},	
{0x43,1,{0xA3}},
{0x44,1,{0x9E}},
{0x45,1,{0x19}},
{0x46,1,{0x28}},
{0x4A,1,{0x12}},
{0x4B,1,{0x12}},
{0x50,1,{0x5A}},
{0x51,1,{0x5A}},
{0xA2,1,{0x01}},
{0xA3,1,{0x16}}, 
{0xFF,3,{0x78,07,01}},
{0x22,1,{0x06}},
{0x36,1,{0x00}},
{0x63,1,{0x08}},
{0x64,1,{0x10}},
{0x6C,1,{0x45}},
{0x6D,1,{0x00}},
{0xFF,3,{0x78,07,05}},
{0xB0,1,{0x40}},
{0xFF,3,{0x78,07,06}},
{0x00,1,{0x43}},  
{0x01,1,{0x12}},  
{0x02,1,{0x21}}, 
{0x03,1,{0x21}},  
{0x04,1,{0x03}},  
{0x05,1,{0x12}},  
{0x06,1,{0x04}},    
{0x07,1,{0x05}},    
{0x08,1,{0x83}},  
{0x09,1,{0x02}},  
{0x0A,1,{0x30}},   	
{0x0B,1,{0x10}},    
{0x0C,1,{0x04}},   
{0x0D,1,{0x04}},    
{0x0E,1,{0x00}},   
{0x0F,1,{0x00}},   	
{0x10,1,{0x00}},
{0x11,1,{0x00}},
{0x12,1,{0x00}},
{0x13,1,{0x00}},
{0x14,1,{0x84}},    
{0x15,1,{0x84}},    
{0x31,1,{0x08}},  
{0x32,1,{0x01}},  
{0x33,1,{0x00}},  
{0x34,1,{0x11}},  
{0x35,1,{0x13}},  
{0x36,1,{0x26}},  
{0x37,1,{0x22}},  
{0x38,1,{0x02}},  
{0x39,1,{0x0C}},  
{0x3A,1,{0x02}}, 
{0x3B,1,{0x02}},  
{0x3C,1,{0x02}},  
{0x3D,1,{0x02}},  
{0x3E,1,{0x28}},  
{0x3F,1,{0x29}},  
{0x40,1,{0x2A}},  
{0x41,1,{0x08}},  
{0x42,1,{0x01}},  
{0x43,1,{0x00}},  
{0x44,1,{0x10}},  
{0x45,1,{0x12}},  
{0x46,1,{0x26}},  
{0x47,1,{0x22}},  
{0x48,1,{0x02}},  
{0x49,1,{0x0C}},  
{0x4A,1,{0x02}},  
{0x4B,1,{0x02}},  
{0x4C,1,{0x02}},  
{0x4D,1,{0x02}},  
{0x4E,1,{0x28}},  
{0x4F,1,{0x29}},  
{0x50,1,{0x2A}},  
{0x61,1,{0x0C}},  
{0x62,1,{0x01}},  
{0x63,1,{0x00}},  
{0x64,1,{0x12}},  
{0x65,1,{0x10}},  
{0x66,1,{0x26}},  
{0x67,1,{0x22}}, 
{0x68,1,{0x02}},  
{0x69,1,{0x08}},  
{0x6A,1,{0x02}},  
{0x6B,1,{0x02}},  
{0x6C,1,{0x02}},  
{0x6D,1,{0x02}},  
{0x6E,1,{0x28}},  
{0x6F,1,{0x29}},  
{0x70,1,{0x2A}},  
{0x71,1,{0x0C}},  
{0x72,1,{0x01}},  
{0x73,1,{0x00}},  
{0x74,1,{0x13}},  
{0x75,1,{0x11}},  
{0x76,1,{0x26}},  
{0x77,1,{0x22}},  
{0x78,1,{0x02}},  
{0x79,1,{0x08}},  
{0x7A,1,{0x02}},  
{0x7B,1,{0x02}},  
{0x7C,1,{0x02}},  
{0x7D,1,{0x02}},  
{0x7E,1,{0x28}},  
{0x7F,1,{0x29}},  
{0x80,1,{0x2A}},  
{0xD0,1,{0x01}},
{0xD1,1,{0x01}},
{0xD2,1,{0x10}}, 
{0xD3,1,{0x00}},
{0xD4,1,{0x00}},
{0xD5,1,{0x00}},
{0xD6,1,{0x00}},
{0xD7,1,{0x00}},
{0xD8,1,{0x00}},
{0xD9,1,{0x00}},
{0xDA,1,{0x00}},
{0xDB,1,{0x47}},
{0xDC,1,{0x02}},  
{0xDD,1,{0x55}},
{0x96,1,{0x80}},
{0x97,1,{0x33}},  
{0xA0,1,{0x08}},  
{0xA1,1,{0x00}},
{0xA2,1,{0x04}},
{0xA3,1,{0x0F}},  
{0xA7,1,{0x00}},
{0xA6,1,{0x32}},
{0xA7,1,{0x00}},  
{0xE5,1,{0x80}},
{0xB2,1,{0x08}},  
{0xB3,1,{0x08}},  
{0xFF,3,{0x78,07,00}},

{0x35, 1,{0x00}},

{0x11, 0,{0x00}},                                  
{REGFLAG_DELAY, 200, {}}, 
                         
{0x29, 0,{0x00}},                                  
{REGFLAG_DELAY, 200, {}},                          
                                                   
{REGFLAG_END_OF_TABLE, 0x00, {}}                   
};

/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
        {0x11, 1, {0x00}},
        {REGFLAG_DELAY, 120, {}},
        {0x29, 1, {0x00}},
        {REGFLAG_DELAY, 20, {}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/
static struct LCM_setting_table lcm_sleep_In_setting[] = {
        {0x28, 1, {0x00}},
        {REGFLAG_DELAY, 50, {}},
        {0x10, 1, {0x00}},
        {REGFLAG_DELAY, 120, {}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};


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

		// enable tearing-free
    params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
    
    params->dsi.mode   = SYNC_PULSE_VDO_MODE;
    
    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_TWO_LANE;
    
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format	= LCM_DSI_FORMAT_RGB888;
    
    // Video mode setting		
    params->dsi.intermediat_buffer_num = 2;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    
    params->dsi.word_count=FRAME_WIDTH*3;
    params->dsi.vertical_active_line=FRAME_HEIGHT;		//jessen //854; //800



		params->dsi.vertical_sync_active				= 4;
		params->dsi.vertical_backporch					= 16;
		params->dsi.vertical_frontporch					= 16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 8;
		params->dsi.horizontal_backporch				= 64;
		params->dsi.horizontal_frontporch				= 64;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    params->dsi.PLL_CLOCK = 208;
   
}


static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);//1
    SET_RESET_PIN(1);
    MDELAY(150);//10

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	#ifdef BUILD_LK
    printf("ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55 lcm_init\n");
	#else
	printk("ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55 lcm_init\n");
	#endif
}


static void lcm_suspend(void)
{
	
	
    push_table(lcm_sleep_In_setting, sizeof(lcm_sleep_In_setting) / sizeof(struct LCM_setting_table), 1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(60);
}


static void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("uboot ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55 %s\n", __func__);
#else
	printk("kernel ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55 %s\n", __func__);
#endif
	lcm_init();
}



static unsigned int lcm_compare_id(void)
{
	int array[16];
	char buffer[4];
	char buffer1[4];
	char  id_high=0;
	char  id_low=0;
	int   id=0;
	
    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    SET_RESET_PIN(0);
	MDELAY(25);
	SET_RESET_PIN(1);
	MDELAY(50);  

	array[0]=0x00063902;
	array[1]=0x010778ff;
    dsi_set_cmdq(array, 2, 1);
 
    array[0] = 0x00043700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x00, buffer, 4);
	id_high = buffer[1];
	
	array[0] = 0x00043700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x01, buffer1, 4);
	
	id_low = buffer1[1];
	
	id      = (id_high<<8) | id_low; 
	
	#ifdef BUILD_LK
		printf("uboot ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55 %s id_high=0x%x,id_low=0x%x\n", __func__, id_high,id_low);
	#else
		printk("kernel ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55 %s id_high=0x%x,id_low=0x%x\n", __func__, id_high,id_low);
	#endif
	
	return (0x7807 == id)?1:0;

}


LCM_DRIVER  ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55_lcm_drv = 
{
    .name			= "ili7807b_qhd_dsi_vdo_auo_shengshiguangdian_j106_bp411_55",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
    .compare_id     = lcm_compare_id,        
};

