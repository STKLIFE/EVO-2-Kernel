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


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};


static struct LCM_setting_table lcm_initialization_setting[] = 
{
  	
	//---------------------------------------Bank0 Setting-------------------------------------------------//
	//------------------------------------Display Control setting----------------------------------------------//
	{0x11, 1,{0x00}},
	{REGFLAG_DELAY, 120, {}},
	//---------------------------------------Bank0 Setting-------------------------------------------------//
	//---------------------------------Display Control setting-------------------------------------------//
{0xFF,5,{0x77,0x01,0x00,0x00,0x10}},

{0xC0,2,{0x77,0x00}},

{0xC1,2,{0x0E,0x0C}},

{0xC2,2,{0x01,0x03}},

{0xC6,1,{0x07}},

{0xCC,1,{0x18}},

{0xB0,16,{0x00,0x05,0x0C,0x12,0x1A,0x0C,0x07,0x09,0x08,0x1C,0x05,0x12,0x0F,0x0E,0x12,0x14}},

{0xB1,16,{0x00,0x05,0x0C,0x10,0x14,0x08,0x08,0x09,0x09,0x1F,0x0A,0x17,0x13,0x14,0x1A,0x1D}},

{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},

{0xB0,1,{0x4D}},

{0xB1,1,{0x51}},

{0xB2,1,{0x87}},

{0xB3,1,{0x80}},

{0xB5,1,{0x44}},

{0xB7,1,{0x89}},

{0xB8,1,{0x21}},

{0xB9,2,{0x00,0x13}},

{0xC1,1,{0x78}},

{0xC2,1,{0x78}},

{0xD0,1,{0x88}},

{0xE0,3,{0x00,0xA0,0x02}},

{0xE1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},

{0xE2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},

{0xE3,4,{0x00,0x00,0x33,0x33}},

{0xE4,2,{0x44,0x44}},

{0xE5,16,{0x09,0xD4,0xAA,0x8C,0x0B,0xD4,0xAA,0x8C,0x05,0xD4,0xAA,0x8C,0x07,0xD4,0xAA,0x8C}},

{0xE6,4,{0x00,0x00,0x33,0x33}},

{0xE7,2,{0x44,0x44}},

{0xE8,16,{0x08,0xD4,0xAA,0x8C,0x0A,0xD4,0xAA,0x8C,0x04,0xD4,0xAA,0x8C,0x06,0xD4,0xAA,0x8C}},

{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},

{0xEB,7,{0x02,0x02,0x4E,0x4E,0x44,0x00,0x10}},

{0xEC,2,{0x3C,0x01}},

{0xED,16,{0x05,0x47,0x61,0xFF,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xFF,0x16,0x74,0x50}},
	{0x29,1, {0x00}},
	{REGFLAG_DELAY, 80, {}},
	
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};

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
    
    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;
    
    // Video mode setting		
    params->dsi.intermediat_buffer_num = 2;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    
    params->dsi.vertical_sync_active				= 4;
    params->dsi.vertical_backporch					= 20;
    params->dsi.vertical_frontporch					= 18;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 80;
		params->dsi.horizontal_frontporch				= 80;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    
    params->dsi.PLL_CLOCK				= 204;
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
    printf("st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 lcm_init\n");
	#else
	printk("st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 lcm_init\n");
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
	printf("uboot st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 %s\n", __func__);
#else
	printk("kernel st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 %s\n", __func__);
#endif
	lcm_init();
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
    printf("st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 test adc_vol is %d\n",adc_vol);
#else
    printk("st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 test adc_vol is %d\n",adc_vol);
#endif
    if(adc_vol>=0 && adc_vol<=100)
        return 1;
    else if(adc_vol>100)
        return 2;
//	else if(adc_vol>100)
//		return 3;
	else
		return -1;
}


static unsigned int lcm_compare_id(void)
{
	int array[16];
	char buffer[4];
	//char buffer1[4];
	char  id_high=0;
	char  id_low=0;
	int   id=0;
	int id_adc = 0;
	
    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    SET_RESET_PIN(0);
	MDELAY(25);
	SET_RESET_PIN(1);
	MDELAY(50);  

    array[0] = 0x00053700;
    dsi_set_cmdq(array, 1, 1);
    
    read_reg_v2(0xa1, buffer, 3);
    id_adc = adc_read_vol();
    id_high = buffer[0];
    id_low = buffer[1];
    id = (id_high<<8) | id_low;
    
    #ifdef BUILD_LK
    printf("st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 %s,buffer[0]=0x%x,buffer[1]=0x%x,id = 0x%x\n", __func__,buffer[0],buffer[1],id);
    #else
    printk("st7701s_qhd480_960_dsi_vdo_ctc_qihong_lianyong_r52 %s,buffer[0]=0x%x,buffer[1]=0x%x,id = 0x%x\n", __func__,buffer[0],buffer[1],id);
    #endif
    
	if(0x8802 == id&&id_adc == 2)
	{
		return 1;
	}
	return 0;
    //return (0x8802 == id)?1:0;
}


LCM_DRIVER st7701s_qhd480_960_dsi_vdo_ctc_qihong_ruiyang_3506_lcm_drv =
{
    .name			= "st7701s_qhd480_960_dsi_vdo_ctc_qihong_ruiyang_3506",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
    .compare_id     = lcm_compare_id,        
};

