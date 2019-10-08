#ifndef BUILD_LK
#include <linux/string.h>
#include<linux/kernel.h>
#endif

#include "lcm_drv.h"


//#define LCM_DEBUG
#ifdef LCM_DEBUG
#define TAG "[rm68191_qhd_dsi_vdo_longyu_boe55_s6_jhm_m088]"
#ifdef BUILD_LK

#define LCM_LOG(format, args...) printf(TAG":"format,##args)
#else
#define LCM_LOG(format, args...) printk(KERN_EMERG TAG":"format,##args)
#endif
#else
#define LCM_LOG(format, args...)
#endif

//#define LCM_ID_RM68191       0x1189

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)


#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif
static unsigned int lcm_compare_id(void);

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    				(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 					(lcm_util.udelay(n))
#define MDELAY(n) 					(lcm_util.mdelay(n))

#define LCM_ID     (0x8191)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)			lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg						lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)          		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)       

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x03}},

{0x90, 9,{0x03, 0x14, 0x09, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00}},

{0x91, 9,{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

{0x92, 11,{0x40, 0x0B, 0x0C, 0x0D, 0x0E, 0x00, 0x38, 0x00, 0x10, 0x03, 0x04}},

{0x94, 8,{0x00, 0x08, 0x0B, 0x03, 0xD2, 0x03, 0xD3, 0x0C}},

{0x95, 16,{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

{0x99, 2,{0x00, 0x00}},

{0x9A, 11,{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

{0x9B, 6,{0x01, 0x38, 0x00, 0x00, 0x00, 0x00}},

{0x9C, 2,{0x00, 0x00}},

{0x9D, 8,{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00}},

{0x9E, 2,{0x00, 0x00}},

{0xA0, 10,{0x9F, 0x1F, 0x08, 0x1F, 0x0A, 0x1F, 0x00, 0x1F, 0x14, 0x1F}},

{0xA1, 10,{0x15, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},

{0xA2, 10,{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},

{0xA4, 10,{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},

{0xA5, 10,{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x15}},

{0xA6, 10,{0x1F, 0x14, 0x1F, 0x01, 0x1F, 0x0B, 0x1F, 0x09, 0x1F, 0x1F}},

{0xA7, 10,{0x1F, 0x1F, 0x0B, 0x1F, 0x09, 0x1F, 0x01, 0x1F, 0x15, 0x1F}},

{0xA8, 10,{0x14, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},

{0xA9, 10,{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},

{0xAB, 10,{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},

{0xAC, 10,{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x14}},

{0xAD, 10,{0x1F, 0x15, 0x1F, 0x00, 0x1F, 0x08, 0x1F, 0x0A, 0x1F, 0x1F}},

{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x00}},

{0xBC, 3,{0x00, 0x00, 0x00}},

{0xB8, 4,{0x01, 0xAF, 0x8F, 0x8F}},

{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x01}},

{0xD1, 16,{0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},

{0xD2, 16,{0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},

{0xD3, 16,{0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},

{0xD4, 4,{0x03, 0xE5, 0x03, 0xFF}},

{0xD5, 16,{0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},

{0xD6, 16,{0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},

{0xD7, 16,{0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},

{0xD8, 4,{0x03, 0xE5, 0x03, 0xFF}},

{0xD9, 16,{0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},

{0xDD, 16,{0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},

{0xDE, 16,{0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},

{0xDF, 4,{0x03, 0xE5, 0x03, 0xFF}},

{0xE0, 16,{0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},

{0xE1, 16,{0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},

{0xE2, 16,{0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},

{0xE3, 4,{0x03, 0xE5, 0x03, 0xFF}},

{0xE4, 16,{0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},

{0xE5, 16,{0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},

{0xE6, 16,{0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},

{0xE7, 4,{0x03, 0xE5, 0x03, 0xFF}},

{0xE8, 16,{0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},

{0xE9, 16,{0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},

{0xEA, 16,{0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},

{0xEB, 4,{0x03, 0xE5, 0x03, 0xFF}},

{0xB0, 3,{0x05, 0x05, 0x05}},

{0xB1, 3,{0x05, 0x05, 0x05}},

{0xB3, 3,{0x10, 0x10, 0x10}},

{0xB4, 3,{0x06, 0x06, 0x06}},

{0xB6, 3,{0x44, 0x44, 0x44}},

{0xB7, 3,{0x34, 0x34, 0x34}},

{0xB8, 3,{0x34, 0x34, 0x34}},

{0xB9, 3,{0x24, 0x24, 0x24}},

{0xBA, 3,{0x24, 0x24, 0x24}},

{0xBC, 3,{0x00, 0x70, 0x00}},

{0xBD, 3,{0x00, 0x70, 0x00}},

{0xBE, 1,{0x40}},

{0x35, 1,{0x00}},

{0x11, 0,{0x00}},

{REGFLAG_DELAY, 120, {}},

{0x29, 0,{0x00}},

{REGFLAG_DELAY, 120, {}},

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
		
        unsigned int cmd;
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



		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 14;
		params->dsi.vertical_frontporch					= 16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 8;
		params->dsi.horizontal_backporch				= 16;
		params->dsi.horizontal_frontporch				= 24;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    params->dsi.PLL_CLOCK = 215;
   
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
		printf("lk rm68191_qhd_dsi_vdo_longyu_boe55_s6_jhm_m088 lcm_init\n\n");
    #else
		printk("kernel rm68191_qhd_dsi_vdo_longyu_boe55_s6_jhm_m088 lcm_init\n\n");
    #endif	
}
static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xF0, 5,{0x55,0xAA,0x52,0x08,0x01}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
//	{0xC3, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

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
//OTM8018B_PRINT(" =====bob.chen lcm_resume====== %s, %d \n", __FUNCTION__, __LINE__);
    lcm_init();
}



static unsigned int lcm_compare_id(void)
{

	
	unsigned int id = 0;
	unsigned char buffer[2];
	unsigned int array[16];

    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(100);

	push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
//	id = read_reg(0xF4);
	read_reg_v2(0xc5, buffer, 2);
	//id = (buffer[1]<<8)|buffer[0]; //we only need ID
	id = (buffer[0]<<8)|buffer[1];
	
	 #ifdef BUILD_LK
		printf("%s,   lk rm68191 id = 0x%08x\n", __func__, id);
    #else
		printk("%s,  kernel rm68191 id = 0x%08x\n", __func__, id);
    #endif
   
    return (LCM_ID == id)?1:0;

}


LCM_DRIVER rm68191_qhd_dsi_vdo_longyu_boe55_s6_jhm_m088_lcm_drv = 
{
	.name		= "rm68191_qhd_dsi_vdo_longyu_boe55_s6_jhm_m088",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
    .compare_id     = lcm_compare_id,        
};


