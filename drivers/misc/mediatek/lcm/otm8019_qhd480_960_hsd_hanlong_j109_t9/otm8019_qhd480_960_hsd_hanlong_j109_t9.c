#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (480)
#define FRAME_HEIGHT (960)

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
 	
{0x00,1,{0x00}},//EnableAccessCommand2&SoftwareEXTCEnable
{0xff,3,{0x80,0x19,0x01}},

{0x00,1,{0x80}},//EnableAccessOriseCommand2
{0xff,2,{0x80,0x19}},
//-----------resolutionadjustment------------
{0x00,1,{0x90}},//PanelResolution:[00h=480x800/02h=480x854]
{0xb3,1,{0x02}},

{0x00,1,{0x92}},//InterfaceSelect:[05h=Pin_IM/45h=SW_IM]
{0xb3,1,{0x45}},


{0x00,1,{0xA2}},
{0xB3,4,{0x01,0xE0,0x03,0xC0}},//480x960
{0x00,1,{0xA7}},
{0xB3,1,{0x04}},//enableautoresolution


//-------------------Tcon&RTNsetting----------------------------
{0x00,1,{0x80}},//RTNsetting:linerateorFP/BPsetting
{0xc0,5,{0x00,0x58,0x00,0x15,0x15}},

{0x00,1,{0x90}},//MCLK_clrshift1~3number
{0xc0,6,{0x00,0x15,0x00,0x00,0x00,0x04}},

{0x00,1,{0xb4}},//PanelDrivingMode:[00h=columninversion/77h=1+2dot]
{0xc0,1,{0x00}},

//---------------powercontrol----------------
{0x00,1,{0x81}},//pump1max./min.autosetting
{0xc5,1,{0x66}},

{0x00,1,{0x82}},//pumpclampvoltage
{0xc5,1,{0xb0}},

{0x00,1,{0x90}},//PowerControl:4eh=3x/76h=+15/-10
{0xc5,3,{0x8e,0x76,0x06}},

//---------------GVDD&GVDDNoutputvoltage----------------
{0x00,1,{0x00}},//GVDD/NGVDD:[6Fh6Fh=+4.5V-4.5V]
{0xd8,2,{0x6F,0x6F}},

//---------------VCOMoutputvoltage----------------
{0x00,1,{0x00}},//VCOMvoltagesetting:3fh=-1.075V
{0xd9,1,{0x1E}},

{0x00,1,{0x81}},//FrameRatesetting
{0xc1,1,{0x33}},

{0x00,1,{0xA1}},//
{0xc1,1,{0x08}},

{0x00,1,{0x81}},//AdjustbiascurrentofsourceOPsetting:[84h=0.6uA]
{0xc4,1,{0x83}},

{0x00,1,{0x87}},//SourceEQ
{0xC4,1,{0x08}},

{0x00,1,{0xb1}},//VDD_18&LVDSVDDsetting
{0xc5,1,{0xa9}},


{0x00,1,{0x92}},//Pump45setting:[01h=+6/-6/00h=+8x/-6x]
{0xc5,1,{0x01}},

{0x00,1,{0x93}},//VRGHvoltage
{0xc5,1,{0x03}},

{0x00,1,{0x94}},//pumpclk&bias
{0xc5,3,{0x32,0x22,0x00}},

{0x00,1,{0x89}},
{0xC4,1,{0x08}},
{0x00,1,{0xA2}},//SourceTimingsetting:Sourcepulllowsetting
{0xC0,3,{0x00,0x00,0x00}},


{0x00,1,{0x80}},//GOAVSTSetting
{0xce,6,{0x85,0x03,0x0a,0x84,0x03,0x0a}},

{0x00,1,{0x90}},//GOAVENDSetting
{0xce,12,{0x33,0xC2,0x0A,0x33,0xC3,0x0A,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0x90}},//GOAVENDSetting
{0xce,12,{0x33,0xBE,0x1A,0x33,0xBF,0x1A,0x00,0x00,0x00,0x00,0x00,0x00}},//diamondmodify


{0x00,1,{0xa0}},//GOACLKA1Setting
{0xce,14,{0x38,0x03,0x83,0xB8,0x86,0x0a,0x00,0x38,0x02,0x83,0xB9,0x5B,0x0a,0x00}},

{0x00,1,{0xb0}},//GOACLKA3Setting
{0xce,14,{0x38,0x01,0x83,0xBA,0x86,0x0a,0x00,0x38,0x00,0x83,0xBB,0x85,0x0a,0x00}},

{0x00,1,{0xc0}},//GOACLKB1Setting
{0xce,14,{0x30,0x00,0x83,0xBC,0x86,0x0a,0x00,0x30,0x01,0x83,0xBD,0x85,0x0a,0x00}},

{0x00,1,{0xd0}},//GOACLKB3Setting
{0xce,14,{0x30,0x02,0x83,0xBE,0x86,0x0a,0x00,0x30,0x03,0x83,0xBF,0x85,0x0a,0x00}},


{0x00,1,{0xc0}},//GOAECLKSetting
{0xcf,10,{0x3d,0x3d,0x10,0x10,0x00,0x00,0x01,0x00,0x10,0x00}},//diamondmodify

//--------------------Poweron/offsequence-------------------------
{0x00,1,{0xc0}},//EnmodeH-byteofsig1~15:(pwrof_0/pwrof_1/norm/pwron_4)
{0xcb,15,{0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xd0}},//EnmodeH-byteofsig16~30:(pwrof_0/pwrof_1/norm/pwron_4)
{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x15,0x15,0x15}},

{0x00,1,{0xe0}},//EnmodeH-byteofsig31~40:(pwrof_0/pwrof_1/norm/pwron_4)
{0xcb,10,{0x15,0x15,0x15,0x15,0x15,0x15,0x00,0x00,0x00,0x00}},

{0x00,1,{0xf0}},
{0xcb,10,{0x00,0xFF,0x0F,0x00,0x00,0x0F,0xFF,0x00,0xFF,0xFF}},

//--------------------------U2D------------------------------------
{0x00,1,{0x80}},
{0xcc,10,{0x26,0x25,0x21,0x22,0x10,0x0E,0x0C,0x0A,0x02,0x06}},

{0x00,1,{0x90}},//11-25
{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xa0}},//Registersettingforsignal26~40selectionwithu2dmode
{0xcc,15,{0x00,0x05,0x01,0x09,0x0B,0x0D,0x0F,0x22,0x21,0x25,0x26,0x00,0x00,0x00,0x00}},

//--------------------------D2U------------------------------------
{0x00,1,{0xb0}},
{0xcc,10,{0x25,0x26,0x21,0x22,0x09,0x0B,0x0D,0x0F,0x05,0x01}},

{0x00,1,{0xc0}},//11-25
{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xd0}},//Registersettingforsignal26~40selectionwithu2dmode
{0xcc,15,{0x00,0x02,0x06,0x10,0x0E,0x0C,0x0A,0x22,0x21,0x26,0x25,0x00,0x00,0x00,0x00}},



//Gamma-----------------------------------------------
{0x00,1,{0x00}},//GammaCorrectionCharacteristicsSetting(2.2+)
{0xe1,20,{0x01,0x03,0x0a,0x17,0x2c,0x40,0x50,0x83,0x72,0x88,0x7f,0x6e,0x87,0x72,0x78,0x71,0x69,0x60,0x55,0x0a}},

{0x00,1,{0x00}},//GammaCorrectionCharacteristicsSetting(2.2-)
{0xe2,20,{0x01,0x03,0x0a,0x17,0x2c,0x40,0x50,0x83,0x72,0x88,0x7f,0x6e,0x87,0x72,0x78,0x71,0x69,0x60,0x55,0x0a}},


{0x00,1,{0x90}},//MCLK_clrshift1~3number
{0xC0,6,{0x00,0x0E,0x00,0x00,0x00,0x04}},

{0x00,1,{0x80}},//Sourceoutputlevelsfornon-displayareasetting:[30h=GND]
{0xc4,1,{0x30}},

{0x00,1,{0x80}},//framerate
{0xc1,3,{0x03,0x44,0x84}},

{0x00,1,{0x98}},//autoTCONp-shiftdisable
{0xc0,1,{0x00}},

{0x00,1,{0xa9}},//TCONp-shift
{0xc0,1,{0x0a}},

{0x00,1,{0xb0}},//TCONvideo
{0xc1,6,{0x20,0x00,0x00,0x00,0x02,0x01}},

{0x00,1,{0xe1}},//TCONvideo
{0xc0,2,{0x40,0x30}},

{0x00,1,{0xa0}},//VideoModeSetting
{0xc1,1,{0xe8}},

{0x00,1,{0x90}},//commandfial
{0xb6,2,{0xb4,0x5a}},

{0x00,1,{0x83}},//VGH&VGLenable
{0xc5,1,{0x01}},

{0x00,1,{0xc4}},//EMIissue
{0xc5,2,{0x00,0x00}},

{0x00,1,{0xba}},//EMIissueDCDCoff
{0xf5,2,{0x00,0x00}},

{0x00,1,{0x00}},//ESDenable
{0xfb,1,{0x01}},

{0x00,1,{0x80}},//DisableAccessOriseCommand2
{0xff,2,{0xff,0xff}},

{0x00,1,{0x00}},//DisableAccessCommand2&SoftwareEXTCEnable
{0xff,3,{0xff,0xff,0xff}},

{0x11,1,{0x00}},
{REGFLAG_DELAY,150,{}},
{0x29,1,{0x00}},
{REGFLAG_DELAY,50,{}},

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
    printf("uboot otm8018b_fwvga_dsi_vdo_cmi_xingxiang_w71m lcm_init\n");
	#else
	printk("kernel otm8018b_fwvga_dsi_vdo_cmi_xingxiang_w71m lcm_init\n");
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
	printf("uboot otm8018b_fwvga_dsi_vdo_cmi_xingxiang_w71m %s\n", __func__);
#else
	printk("kernel otm8018b_fwvga_dsi_vdo_cmi_xingxiang_w71m %s\n", __func__);
#endif
	lcm_init();
}



static unsigned int lcm_compare_id(void)
{
//	unsigned int data_array[16];
	//int   array[4];
	char  buffer[5];
	char  id_high=0;
	char  id_low=0;
	int   id=0;
	
    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(10);
	SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);



//	dsi_set_cmdq(data_array, 2, 1);
	read_reg_v2(0xa1,buffer, 4);
	id_low  = buffer[3];         
	id_high = buffer[2];

	id=(id_high<<8) + id_low; 
    
    #ifdef BUILD_LK
       printf("bob.chen: otm8019_qhd480_960_hsd_hanlong_j109_t9 %s, id = 0x%08x\n", __func__, id);
    #else
        printk("bob.chen: otm8019_qhd480_960_hsd_hanlong_j109_t9 %s, id = 0x%08x\n", __func__, id);
    #endif
	
    
    
    return (0x8019 == id)?1:0;

}

LCM_DRIVER  otm8019_qhd480_960_hsd_hanlong_j109_t9_lcm_drv = 
{
    .name			= "otm8019_qhd480_960_hsd_hanlong_j109_t9",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
    .compare_id     = lcm_compare_id,        
};

