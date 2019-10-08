#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (480)
#define FRAME_HEIGHT (854)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0XFFF   // END OF REGISTERS MARKER


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
 	
{0x00,1,{0x00}},
{0xFF,3,{0x80,0x09,0x01}},
{0x00,1,{0x80}},
{0xFF,2,{0x80,0x09}},
{0x00,3,{0xD2,0xD2,0xD2}}, 
{0xB0,1,{0x01}},
{0x00,1,{0x80}}, 
{0xC4,1,{0x30}}, //????
{REGFLAG_DELAY, 10, {}},									 
{0x00,1,{0x8A}}, 
{0xC4,1,{0x40}}, //????
{REGFLAG_DELAY, 10, {}},
{0x00,1,{0x00}},
{0xD8,2,{0x87,0x87}},
{0x00,1,{0xB1}},
{0xC5,1,{0xA9}},
{0x00,1,{0x00}},
{0xD9,1,{0x20}},
{0x00,1,{0x90}},
{0xC5,3,{0x96,0xA7,0x01}},
{0x00,1,{0x81}},
{0xC1,1,{0x66}},
{0x00,1,{0xA1}},
{0xC1,3,{0x08,0x02,0x1B}},
{0x00,1,{0x81}},
{0xC4,1,{0x83}},
{0x00,1,{0x90}},
{0xB3,1,{0x02}},
{0x00,1,{0x92}},
{0xB3,1,{0x45}},
{0x00,1,{0xA7}},
{0xB3,1,{0x00}},
{0x00,1,{0xB4}},
{0xC0,1,{0x10}},
{0x00,1,{0x00}},
{0x36,1,{0x00}},
{0x00,1,{0x90}},
{0xC0,6,{0x00,0x44,0x00,0x00,0x00,0x03}},
{0x00,1,{0xA6}},
{0xC1,3,{0x00,0x00,0x00}},
{0x00,1,{0x80}},
{0xCE,6,{0x87,0x03,0x00,0x86,0x03,0x00}},
{0x00,1,{0x90}},
{0xCE,6,{0x33,0x54,0x00,0x33,0x55,0x00}},
{0x00,1,{0xA0}},
{0xCE,14,{0x38,0x03,0x03,0x58,0x00,0x00,0x00,0x38,0x02,0x03,0x59,0x00,0x00,0x00}},
{0x00,1,{0xB0}},
	{0xCE,14,{0x38,0x01,0x03,0x5A,0x00,0x00,0x00,0x38,0x00,0x03,0x5B,0x00,0x00,0x00}},																		 
																																							 
	{0x00,1,{0xC0}},																																		 
	{0xCE,14,{0x30,0x00,0x03,0x5C,0x00,0x00,0x00,0x30,0x01,0x03,0x5D,0x00,0x00,0x00}},																		 
																																							 
	{0x00,1,{0xD0}},																																		 
	{0xCE,14,{0x38,0x05,0x03,0x5E,0x00,0x00,0x00,0x38,0x04,0x03,0x5F,0x00,0x00,0x00}},																		 
																																							 
	{0x00,1,{0xC0}},																																		 
	{0xCF,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x09}},																							 
																																							 
	{0x00,1,{0xC0}},																																		 
	{0xCB,15,{0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00}}, 																 
																																							 
	{0x00,1,{0xD0}},																																		 
	{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04}}, 																 
																																							 
	{0x00,1,{0xE0}},																																		 
	{0xCB,10,{0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},																							 
																																							 
	{0x00,1,{0x80}},																																		 
	{0xCC,10,{0x00,0x26,0x25,0x02,0x06,0x00,0x00,0x0A,0x0E,0x0C}},																							 
																																							 
	{0x00,1,{0x90}},																																		 

	{0xCC,15,{0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x01,0x05}}, 																 
																																							 
	{0x00,1,{0xA0}},																																		 
	{0xCC,15,{0x00,0x00,0x09,0x0D,0x0B,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 																 
																																							 
	{0x00,1,{0xB0}},																																		 
	{0xCC,10,{0x00,0x25,0x26,0x05,0x01,0x00,0x00,0x0D,0x09,0x0B}},																							 
																																							 
	{0x00,1,{0xC0}},																																		 
	{0xCC,15,{0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x26,0x06,0x02}}, 																 
																																							 
	{0x00,1,{0xD0}},																																		 
	{0xCC,15,{0x00,0x00,0x0E,0x0A,0x0C,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 																 
{0x00,1,{0x00}},
{0xE1,16,{0x06,0x07,0x0A,0x0A,0x04,0x16,0x0F,0x0F,0x00,0x04,0x03,0x07,0x0E,0x21,0x1D,0x0B}},
{0x00,1,{0x00}},
{0xE2,16,{0x06,0x07,0x0A,0x0A,0x04,0x17,0x0F,0x0F,0x00,0x04,0x02,0x07,0x0F,0x22,0x1E,0x0B}},
{0x00,1,{0xA0}},
{0xC1,1,{0xEA}},
{0x00,1,{0xA6}},
{0xC1,3,{0x01,0x00,0x00}},
{0x00,1,{0xC6}},
{0xB0,1,{0x03}},
{0x00,1,{0x81}}, 
{0xC5,1,{0x66}}, //Auto pump 2x
{0x00,1,{0xB6}}, 
{0xF5,1,{0x06}}, //VGL_REG timing setting.
{0x00,1,{0x8B}}, 
{0xB0,1,{0x40}}, //ULPM
{0x00,1,{0xB1}}, 
{0xB0,1,{0x80}}, //MIPI RC delay setting
{0x00,1,{0x00}},
{0xFF,3,{0xFF,0xFF,0xFF}},
{0x21,1,{0x00}},
	
																																																																								

	{0x11, 1 ,{0x00}}, 
	{REGFLAG_DELAY, 120, {}},//150

	{0x29, 1 ,{0x00}}, 
	{REGFLAG_DELAY, 10, {}},//200

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



		params->dsi.vertical_sync_active				= 1;
		params->dsi.vertical_backporch					= 16;
		params->dsi.vertical_frontporch					= 15;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 4;
		params->dsi.horizontal_backporch				= 44;
		params->dsi.horizontal_frontporch				= 46;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    params->dsi.PLL_CLOCK = 190;
   
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
    printf("uboot otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03 lcm_init\n");
	#else
	printk("kernel otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03 lcm_init\n");
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
	printf("uboot otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03 %s\n", __func__);
#else
	printk("kernel otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03 %s\n", __func__);
#endif
	lcm_init();
}



static unsigned int lcm_compare_id(void)
{
	unsigned int data_array[16];
	char  buffer[5];
	char  id_high=0;
	char  id_low=0;
	int   id=0;
	
    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);


		data_array[0]=0x00043902;
		data_array[1]=0x010980ff;
		dsi_set_cmdq(data_array, 2, 1);

		data_array[0]=0x00023902;
		data_array[1]=0x00008000;
		dsi_set_cmdq(data_array, 2, 1);
		
		data_array[0]=0x00033902;
		data_array[1]=0x000980ff;
		dsi_set_cmdq(data_array, 2, 1);
	
	data_array[0]=0x00023902;
	data_array[1]=0x00000200;
	dsi_set_cmdq(data_array, 2, 1);
	read_reg_v2(0xd2,buffer, 1);
	id_high = buffer[0];       //bob.chen read 0x80

	data_array[0]=0x00023902;
	data_array[1]=0x00000300;
	dsi_set_cmdq(data_array, 2, 1);
	read_reg_v2(0xd2,buffer, 1);
	id_low  = buffer[0];         

	id      = (id_high<<8) + id_low; 
    
	#ifdef BUILD_LK
		printf("uboot otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03 %s id_high=0x%x,id_low=0x%x,id=0x%x\n", __func__, id_high,id_low,id);
	#else
		printk("kernel otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03 %s id_high=0x%x,id_low=0x%x,id=0x%x\n", __func__, id_high,id_low,id);
	#endif

	
    
    return (0x8009 == id)?1:0;

}

LCM_DRIVER  otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03_lcm_drv = 
{
    .name			= "otm8018b_fwvga_dsi_vdo_hanlong_cpt_youshi_s9b_k03",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
    .compare_id     = lcm_compare_id,        
};

