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

#define FRAME_WIDTH  									480
#define FRAME_HEIGHT 									854

#define REGFLAG_DELAY             							0XFD
#define REGFLAG_END_OF_TABLE      							0xFE 


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


static struct LCM_setting_table lcm_initialization_setting[] = 
{
  {0x11,0,{}},                  
    {REGFLAG_DELAY, 200, {}},    
    
    {0xFF,5,{0x77, 0x01, 0x00, 0x00, 0x10}},
    {0xC0,2,{0xE9, 0x03}},
    {0xC1,2,{0x0C, 0x02}},
    {0xC2,52,{0x10, 0x08}},
    {0xCC,1,{0x18}},//add
    {0xB0,16,{0x00,0x06,0x12,0x16,0x1C,0x0D,0x15,0x09,0x09,0x27,0x04,0x0E,0x07,0x15,0x16,0x18}},
    {0xB1,16,{0x00,0x06,0x10,0x13,0x18,0x0B,0x16,0x09,0x09,0x2C,0x0C,0x1A,0x1B,0x1E,0x24,0x18}},
    {0xFF,5,{0x77, 0x01, 0x00, 0x00, 0x11}},
    {0xB0,1,{0x4D}},
    {0xB1,1,{0x5f}},  // 51
    {0xB2,1,{0x07}},
    {0xB3,1,{0x80}},
    {0xB5,1,{0x47}},
    {0xB7,1,{0x85}},
    {0xB8,1,{0x21}},
    {0xB9,1,{0x10}},//add 33
    {0xC1,1,{0x78}},
    {0xC2,1,{0x78}},
    {0xD0,1,{0x88}},
    {REGFLAG_DELAY, 100, {}},
    {0xE0,3,{0x00, 0x00, 0x02}},
    {0xE1,11,{0x04,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
    {0xE2,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0xE3,4,{0x00, 0x00, 0x33, 0x00}},
    {0xE4,2,{0x22, 0x00}},
    {0xE5,16,{0x04,0x5C,0xA0,0xA0,0x06,0x5C,0xA0,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0xE6,4,{0x00, 0x00, 0x33, 0x00}},
    {0xE7,2,{0x22, 0x00}},
    {0xE8,16,{0x05,0x5C,0xA0,0xA0,0x07,0x5C,0xA0,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0xEB,7,{0x02,0x00,0x40,0x40,0x00,0x00,0x00}},
    {0xEC,2,{0x00, 0x00}},
    {0xED,16,{0xFA,0x45,0x0B,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xB0,0x54,0xAF}},
	{0xFF,5,{0x77, 0x01, 0x00, 0x00, 0x13}},
	{0xE6,2,{0x16, 0x7c}},
    {0xFF,5,{0x77, 0x01, 0x00, 0x00, 0x00}},
    {REGFLAG_DELAY, 10, {}},
    {0x29,0,{}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}

};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;
    
    for(i = 0; i < count; i++) 
    {
        unsigned cmd;
        cmd = table[i].cmd;
        
        switch (cmd) 
        {
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
    
    #if (LCM_DSI_CMD_MODE)
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
    params->dsi.packet_size=256;
    
    // Video mode setting		
    params->dsi.intermediat_buffer_num = 2;
    
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    
    params->dsi.vertical_sync_active				= 4;
    params->dsi.vertical_backporch					= 18;
    params->dsi.vertical_frontporch					= 18;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 
    
    params->dsi.horizontal_sync_active				= 10;
    params->dsi.horizontal_backporch				= 50;
    params->dsi.horizontal_frontporch				= 50;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    
    params->dsi.PLL_CLOCK				= 204;
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);
    
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    
    #ifdef BUILD_LK
    printf("st7701s_fwvga_dsi_vdo_boe_huajing_kxd_wzyh50 %s\n", __func__);
    #else
    printk("st7701s_fwvga_dsi_vdo_boe_huajing_kxd_wzyh50 %s\n", __func__);
    #endif

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
}



static unsigned int lcm_compare_id(void)
{
    int array[4];
    char buffer[5];
    char id_high=0;
    char id_low=0;
    int id=0;
    
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(50);

    array[0] = 0x00053700;
    dsi_set_cmdq(array, 1, 1);
    
    read_reg_v2(0xa1, buffer, 5);
    id_high = buffer[0];
    id_low = buffer[1];
    id = (id_high<<8) | id_low;
    
    #ifdef BUILD_LK
    printf("st7701s_fwvga_dsi_vdo_boe_huajing_kxd_wzyh50 %s,buffer[0]=0x%x,buffer[1]=0x%x,id = 0x%x\n", __func__,buffer[0],buffer[1],id);
    #else
    printk("st7701s_fwvga_dsi_vdo_boe_huajing_kxd_wzyh50 %s,buffer[0]=0x%x,buffer[1]=0x%x,id = 0x%x\n", __func__,buffer[0],buffer[1],id);
    #endif
    
    return (0x8802 == id)?1:0;
}


LCM_DRIVER st7701s_fwvga_dsi_vdo_boe_huajing_kxd_wzyh50_lcm_drv =
{
    .name			= "st7701s_fwvga_dsi_vdo_boe_huajing_kxd_wzyh50",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
};
