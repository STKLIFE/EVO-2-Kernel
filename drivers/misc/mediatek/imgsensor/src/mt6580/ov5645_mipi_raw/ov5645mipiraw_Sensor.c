/*****************************************************************************
 *
 * Filename:
 * ---------
 *     ov5645mipi_Sensor.c
 *
 * Project:
 * --------
 *     ALPS
 *
 * Description:
 * ------------
 *     Source code of Sensor driver
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "kd_camera_typedef.h"


//#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov5645mipiraw_Sensor.h"

/****************************Modify following Strings for debug****************************/
#define PFX "ov5645_camera_sensor"

//#define LOG_1 //LOG_INF("ov5645,MIPI 1LANE\n")
//#define LOG_2 //LOG_INF("preview 1470*1100@30fps,256Mbps/lane\n")
/****************************   Modify end    *******************************************/
//#define //LOG_INF(format, args...)	xlog_printk(ANDROID_//LOG_INFO   , PFX, "[%s] " format, __FUNCTION__, ##args)
//#define //LOG_INF(format, args...)    pr_debug(PFX "[%s] " format, __FUNCTION__, ##args)
static DEFINE_SPINLOCK(imgsensor_drv_lock);
//static kal_uint8  test_pattern_flag=0;

#define MIPI_SETTLEDELAY_AUTO     0
#define MIPI_SETTLEDELAY_MANNUAL  1

//#define BIRD_OV5645_H_MIRROR

static imgsensor_info_struct imgsensor_info = { 
	.sensor_id = OV5645MIPI_SENSOR_ID,//OV5645MIPI_SENSOR_ID
	
	.checksum_value = 0x1ec5153d,
	
	.pre = {
		.pclk = 84000000,				//record different mode's pclk
		.linelength = 2844,				//record different mode's linelength
		.framelength = 1968,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width = 2592,		//record different mode's width of grabwindow
		.grabwindow_height = 1944,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 14,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 150,	
	},
	.cap = {
		.pclk = 84000000,
		.linelength = 2844,
		.framelength = 1968,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2592,
		.grabwindow_height = 1944,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 150,
	},
	.cap1 = {
		.pclk = 84000000,
		.linelength = 2844,
		.framelength = 1968,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2592,
		.grabwindow_height = 1944,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 150,	
	},
	.normal_video = {
		.pclk = 56000000,
		.linelength = 1896,
		.framelength = 984,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 960,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 56000000,
		.linelength = 1896,
		.framelength = 984,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 960,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 300,
	},
	.slim_video = {
		.pclk = 56000000,
		.linelength = 1896,
		.framelength = 984,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 960,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 300,
	},
    .margin = 4,            //sensor framelength & shutter margin
	.min_shutter = 1,		//min shutter
	.max_frame_length = 0x7fff,//max framelength by sensor register's limitation
	.ae_shut_delay_frame = 0,	//shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2
    .ae_sensor_gain_delay_frame = 0,//sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2
	.ae_ispGain_delay_frame = 2,//isp gain delay frame for AE cycle
	.ihdr_support = 0,	  //1, support; 0,not support
	.ihdr_le_firstline = 0,  //1,le first ; 0, se first
	.sensor_mode_num = 5,	  //support sensor mode num
	
	.cap_delay_frame = 2,		//enter capture delay frame num
	.pre_delay_frame = 2,		//enter preview delay frame num
	.video_delay_frame = 2, 	//enter video delay frame num
	.hs_video_delay_frame = 2,	//enter high speed video  delay frame num
	.slim_video_delay_frame = 2,//enter slim video delay frame num
	
	.isp_driving_current = ISP_DRIVING_4MA, //mclk driving current
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,//sensor_interface_type
    .mipi_sensor_type = MIPI_OPHY_NCSI2, //0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2
    .mipi_settle_delay_mode = 0,//0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gb,//sensor output first pixel color	
	.mclk = 24,//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
	.mipi_lane_num = SENSOR_MIPI_2_LANE,//mipi lane num
	.i2c_addr_table = {0x78, 0xff},//record sensor support all write id addr, only supprt 4must end with 0xff
    .i2c_speed = 100, // i2c read/write speed
};


static imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,				//mirrorflip information
	.sensor_mode = IMGSENSOR_MODE_INIT, //IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video,High Speed Video, Slim Video
	.shutter = 0x3D0,					//current shutter
	.gain = 0x100,						//current gain
	.dummy_pixel = 0,					//current dummypixel
	.dummy_line = 0,					//current dummyline
    .current_fps = 300,  //full size current fps : 24fps for PIP, 30fps for Normal or ZSD
	.autoflicker_en = KAL_FALSE,  //auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker
	.test_pattern = KAL_FALSE,		//test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,//current scenario id
	.ihdr_en = 0, //sensor need support LE, SE with HDR feature
	.i2c_write_id = 0x78,
};


/* Sensor output window information */
static SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[5] =
{
{ 2592, 1944,		0,	  0, 2592, 1944, 2592, 1944,	 0,   0, 2592, 1944,  0,	 0, 2592,  1944}, // capture
{ 2592, 1944,		0,	  0, 2592, 1944, 2592, 1944,	 0,   0, 2592, 1944,  0,	 0, 2592,  1944}, // capture
{ 2592, 1944,		0,	  0, 2592, 1944, 2592, 1944,	 0,   0, 1296, 972,  8,	 6, 1280,  960}, // capture
{ 2592, 1944,		0,	  0, 2592, 1944, 2592, 1944,	 0,   0, 1296, 972,  8,	 6, 1280,  960}, // capture
{ 2592, 1944,		0,	  0, 2592, 1944, 2592, 1944,	 0,   0, 1296, 972,  8,	 6, 1280,  960}, // capture
}; // slim video


// Gain Index
#define MaxGainIndex (97)
/*
static kal_uint16 sensorGainMapping[MaxGainIndex][2] ={
{ 64 ,0  },   
{ 68 ,12 },   
{ 71 ,23 },   
{ 74 ,33 },   
{ 77 ,42 },   
{ 81 ,52 },   
{ 84 ,59 },   
{ 87 ,66 },   
{ 90 ,73 },   
{ 93 ,79 },   
{ 96 ,85 },   
{ 100,91 },   
{ 103,96 },   
{ 106,101},   
{ 109,105},   
{ 113,110},   
{ 116,114},   
{ 120,118},   
{ 122,121},   
{ 125,125},   
{ 128,128},   
{ 132,131},   
{ 135,134},   
{ 138,137},
{ 141,139},
{ 144,142},   
{ 148,145},   
{ 151,147},   
{ 153,149}, 
{ 157,151},
{ 160,153},      
{ 164,156},   
{ 168,158},   
{ 169,159},   
{ 173,161},   
{ 176,163},   
{ 180,165}, 
{ 182,166},   
{ 187,168},
{ 189,169},
{ 193,171},
{ 196,172},
{ 200,174},
{ 203,175}, 
{ 205,176},
{ 208,177}, 
{ 213,179}, 
{ 216,180},  
{ 219,181},   
{ 222,182},
{ 225,183},  
{ 228,184},   
{ 232,185},
{ 235,186},
{ 238,187},
{ 241,188},
{ 245,189},
{ 249,190},
{ 253,191},
{ 256,192}, 
{ 260,193},
{ 265,194},
{ 269,195},
{ 274,196},   
{ 278,197},
{ 283,198},
{ 288,199},
{ 293,200},
{ 298,201},   
{ 304,202},   
{ 310,203},
{ 315,204},
{ 322,205},   
{ 328,206},   
{ 335,207},   
{ 342,208},   
{ 349,209},   
{ 357,210},   
{ 365,211},   
{ 373,212}, 
{ 381,213},
{ 400,215},      
{ 420,217},   
{ 432,218},   
{ 443,219},      
{ 468,221},   
{ 482,222},   
{ 497,223},   
{ 512,224},
{ 529,225}, 	 
{ 546,226},   
{ 566,227},   
{ 585,228}, 	 
{ 607,229},   
{ 631,230},   
{ 656,231},   
{ 683,232}
};
*/
static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };
	iReadRegI2C(pu_send_cmd, 2, (u8*)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}

static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}
/*
static kal_uint16 read_cmos_sensor_8(kal_uint16 addr)
{
	kal_uint16 get_byte=0;
    kdSetI2CSpeed(imgsensor_info.i2c_speed); // Add this func to set i2c speed by each sensor
    char pu_send_cmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    iReadRegI2C(pu_send_cmd , 2, (u8*)&get_byte,1,imgsensor.i2c_write_id);
    return get_byte;
}
*/
  /*
static void write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para)
{
	char pu_send_cmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}   
   */
static void set_dummy(void)
{
    //LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line, imgsensor.dummy_pixel);
    /* you can set dummy by imgsensor.dummy_line and imgsensor.dummy_pixel, or you can set dummy by imgsensor.frame_length and imgsensor.line_length */
    write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
    write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
    write_cmos_sensor(0x380c, imgsensor.line_length >> 8);
    write_cmos_sensor(0x380d, imgsensor.line_length & 0xFF);

}    /*    set_dummy  */


static kal_uint32 return_sensor_id(void)
{
	write_cmos_sensor(0x3103,0x11);
	write_cmos_sensor(0x3008,0x82);//soft reset
	mDELAY(10);
    return ((read_cmos_sensor(0x300a) << 8) | read_cmos_sensor(0x300b));//chip id is 0x5645
}
static void set_max_framerate(UINT16 framerate,kal_bool min_framelength_en)
{
//	kal_int16 dummy_line;
	kal_uint32 frame_length = imgsensor.frame_length;
	//unsigned long flags;
	//LOG_INF("framerate = %d, min framelength should enable? \n", framerate,min_framelength_en);   
	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length)?frame_length:imgsensor.min_frame_length;
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	//dummy_line = frame_length - imgsensor.min_frame_length;
	//if (dummy_line < 0)
	//	imgsensor.dummy_line = 0;
	//else
	//	imgsensor.dummy_line = dummy_line;
	//imgsensor.frame_length = frame_length + imgsensor.dummy_line;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
	{
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */

static void write_shutter(kal_uint16 shutter)
{
    kal_uint16 realtime_fps = 0;

    /* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
    /* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */

    // OV Recommend Solution
    // if shutter bigger than frame_length, should extend frame length first
    spin_lock(&imgsensor_drv_lock);
    if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
        imgsensor.frame_length = shutter + imgsensor_info.margin;
    else
        imgsensor.frame_length = imgsensor.min_frame_length;
    if (imgsensor.frame_length > imgsensor_info.max_frame_length)
        imgsensor.frame_length = imgsensor_info.max_frame_length;
    spin_unlock(&imgsensor_drv_lock);
    shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
    shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

    // Framelength should be an even number
    shutter = (shutter >> 1) << 1;
    imgsensor.frame_length = (imgsensor.frame_length >> 1) << 1;

    if (imgsensor.autoflicker_en) {
        realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;

        if(realtime_fps >= 297 && realtime_fps <= 305)
            set_max_framerate(296,0);
        else if(realtime_fps >= 147 && realtime_fps <= 150)
            set_max_framerate(146,0);
        else {
            // Extend frame length
            write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
            write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
        }
    } else {
        // Extend frame length
        write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
        write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
    }

    // Update Shutter
    write_cmos_sensor(0x3502, (shutter << 4) & 0xFF);
    write_cmos_sensor(0x3501, (shutter >> 4) & 0xFF);
    write_cmos_sensor(0x3500, (shutter >> 12) & 0x0F);

    printk("shutter =%d, framelength =%d\n", shutter, imgsensor.frame_length);
}    /*    set_shutter */


/*************************************************************************
* FUNCTION
*    set_shutter
*
* DESCRIPTION
*    This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*    iShutter : exposured lines
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
    unsigned long flags;
    spin_lock_irqsave(&imgsensor_drv_lock, flags);
    imgsensor.shutter = shutter;
    spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

    write_shutter(shutter);
}

static kal_uint16 gain2reg(const kal_uint16 gain)
	{
		kal_uint16 reg_gain = 0x0000;
	
		reg_gain = ((gain / BASEGAIN) << 4) + ((gain % BASEGAIN) * 16 / BASEGAIN);
		reg_gain = reg_gain & 0xFFFF;
	
		return (kal_uint16)reg_gain;
	}
/*************************************************************************
* FUNCTION
*	set_gain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*	iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
    kal_uint16 reg_gain;

    /* 0x350A[0:1], 0x350B[0:7] AGC real gain */
    /* [0:3] = N meams N /16 X    */
    /* [4:9] = M meams M X         */
    /* Total gain = M + N /16 X   */

    //
    if (gain < BASEGAIN || gain > 32 * BASEGAIN) {
        printk("Error gain setting");

        if (gain < BASEGAIN)
            gain = BASEGAIN;
        else if (gain > 32 * BASEGAIN)
            gain = 32 * BASEGAIN;
    }

    reg_gain = gain2reg(gain);
    spin_lock(&imgsensor_drv_lock);
    imgsensor.gain = reg_gain;
    spin_unlock(&imgsensor_drv_lock);
    printk("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

    write_cmos_sensor(0x350a, reg_gain >> 8);
    write_cmos_sensor(0x350b, reg_gain & 0xFF);

    return gain;
}	/*	set_gain  */
/*
static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
    //LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n",le,se,gain);
    if (imgsensor.ihdr_en) {

        spin_lock(&imgsensor_drv_lock);
        if (le > imgsensor.min_frame_length - imgsensor_info.margin)
            imgsensor.frame_length = le + imgsensor_info.margin;
        else
            imgsensor.frame_length = imgsensor.min_frame_length;
        if (imgsensor.frame_length > imgsensor_info.max_frame_length)
            imgsensor.frame_length = imgsensor_info.max_frame_length;
        spin_unlock(&imgsensor_drv_lock);
        if (le < imgsensor_info.min_shutter) le = imgsensor_info.min_shutter;
        if (se < imgsensor_info.min_shutter) se = imgsensor_info.min_shutter;


        // Extend frame length first
        write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
        write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);

        write_cmos_sensor(0x3502, (le << 4) & 0xFF);
        write_cmos_sensor(0x3501, (le >> 4) & 0xFF);
        write_cmos_sensor(0x3500, (le >> 12) & 0x0F);

        write_cmos_sensor(0x3508, (se << 4) & 0xFF);
        write_cmos_sensor(0x3507, (se >> 4) & 0xFF);
        write_cmos_sensor(0x3506, (se >> 12) & 0x0F);

        set_gain(gain);
    }

}

static void set_mirror_flip(kal_uint8 image_mirror)
{
#if 0
//LOG_INF("image_mirror = %d\n", image_mirror);


	kal_uint8  iTemp; 
	
	iTemp = read_cmos_sensor(0x0101);
	iTemp&= ~0x03; //Clear the mirror and flip bits.
	switch (image_mirror) 
	{
		case IMAGE_NORMAL:
			write_cmos_sensor_8(0x0101, iTemp);    //Set normal
			break;
		case IMAGE_H_MIRROR:
            write_cmos_sensor_8(0x0101, iTemp | 0x01); //Set mirror
			break;
		case IMAGE_V_MIRROR:
            write_cmos_sensor_8(0x0101, iTemp | 0x02); //Set flip
			break;
		case IMAGE_HV_MIRROR:
            write_cmos_sensor_8(0x0101, iTemp | 0x03); //Set mirror and flip
			break;
		default:
			printk("Error image_mirror setting\n");
	}
#endif
}
*/
/*************************************************************************
* FUNCTION
*	night_mode
*
* DESCRIPTION
*	This function night mode of sensor.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/ 
}	/*	night_mode	*/

static void sensor_init(void)
{
	printk("OV5645 sensor_init 20180804");
}	/*	sensor_init  */


static void preview_setting(void)
{
	printk("OV5645 preview_setting 20180804");
	write_cmos_sensor(0x3103,0x11);
	write_cmos_sensor(0x3008,0x82);
	mDELAY(50);
	write_cmos_sensor(0x3008,0x42);
	write_cmos_sensor(0x3103,0x03);
	write_cmos_sensor(0x3503,0x07);
	write_cmos_sensor(0x3002,0x1c);
	write_cmos_sensor(0x3006,0xc3);
	write_cmos_sensor(0x300e,0x45);
	write_cmos_sensor(0x3017,0x40);
	write_cmos_sensor(0x3018,0x00);
	write_cmos_sensor(0x302e,0x0b);
	write_cmos_sensor(0x3037,0x13);
	write_cmos_sensor(0x3108,0x01);
	write_cmos_sensor(0x3611,0x06);
	write_cmos_sensor(0x3612,0xab);
	write_cmos_sensor(0x3614,0x50);
	write_cmos_sensor(0x3618,0x04);
	write_cmos_sensor(0x3034,0x1a);
	write_cmos_sensor(0x3035,0x12);
	write_cmos_sensor(0x3036,0x69);
	write_cmos_sensor(0x3500,0x00);
	write_cmos_sensor(0x3501,0x01);
	write_cmos_sensor(0x3502,0x00);
	write_cmos_sensor(0x350a,0x00);
	write_cmos_sensor(0x350b,0x3f);
	write_cmos_sensor(0x3600,0x08);
	write_cmos_sensor(0x3601,0x33);
	write_cmos_sensor(0x3620,0x33);
	write_cmos_sensor(0x3621,0xe0);
	write_cmos_sensor(0x3622,0x01);
	write_cmos_sensor(0x3630,0x2d);
	write_cmos_sensor(0x3631,0x00);
	write_cmos_sensor(0x3632,0x32);
	write_cmos_sensor(0x3633,0x52);
	write_cmos_sensor(0x3634,0x70);
	write_cmos_sensor(0x3635,0x13);
	write_cmos_sensor(0x3636,0x03);
	write_cmos_sensor(0x3702,0x6e);
	write_cmos_sensor(0x3703,0x52);
	write_cmos_sensor(0x3704,0xa0);
	write_cmos_sensor(0x3705,0x33);
	write_cmos_sensor(0x3708,0x63);
	write_cmos_sensor(0x3709,0x12);
	write_cmos_sensor(0x370b,0x61);
	write_cmos_sensor(0x370c,0xc0);
	write_cmos_sensor(0x370f,0x10);
	write_cmos_sensor(0x3715,0x08);
	write_cmos_sensor(0x3717,0x01);
	write_cmos_sensor(0x371b,0x20);
	write_cmos_sensor(0x3731,0x22);
	write_cmos_sensor(0x3739,0x70);
	write_cmos_sensor(0x3901,0x0a);
	write_cmos_sensor(0x3905,0x02);
	write_cmos_sensor(0x3906,0x10);
	write_cmos_sensor(0x3719,0x86);
	write_cmos_sensor(0x3800,0x00);
	write_cmos_sensor(0x3801,0x00);
	write_cmos_sensor(0x3802,0x00);
	write_cmos_sensor(0x3803,0x00);
	write_cmos_sensor(0x3804,0x0a);
	write_cmos_sensor(0x3805,0x3f);
	write_cmos_sensor(0x3806,0x07);
	write_cmos_sensor(0x3807,0x9f);
	write_cmos_sensor(0x3808,0x0a);
	write_cmos_sensor(0x3809,0x20);
	write_cmos_sensor(0x380a,0x07);
	write_cmos_sensor(0x380b,0x98);
	write_cmos_sensor(0x380c,0x0b);
	write_cmos_sensor(0x380d,0x1c);
	write_cmos_sensor(0x380e,0x07);
	write_cmos_sensor(0x380f,0xb0);
	write_cmos_sensor(0x3810,0x00);
	write_cmos_sensor(0x3811,0x10);
	write_cmos_sensor(0x3812,0x00);
	write_cmos_sensor(0x3813,0x06);
	write_cmos_sensor(0x3814,0x11);
	write_cmos_sensor(0x3815,0x11);
	write_cmos_sensor(0x3820,0x46);
	write_cmos_sensor(0x3821,0x00);
	write_cmos_sensor(0x3824,0x01);
	write_cmos_sensor(0x3826,0x03);
	write_cmos_sensor(0x3828,0x08);
	write_cmos_sensor(0x3a02,0x07);
	write_cmos_sensor(0x3a03,0xb0);
	write_cmos_sensor(0x3a08,0x01);
	write_cmos_sensor(0x3a09,0x27);
	write_cmos_sensor(0x3a0a,0x00);
	write_cmos_sensor(0x3a0b,0xf6);
	write_cmos_sensor(0x3a0e,0x06);
	write_cmos_sensor(0x3a0d,0x08);
	write_cmos_sensor(0x3a14,0x07);
	write_cmos_sensor(0x3a15,0xb0);
	write_cmos_sensor(0x3a18,0x00);
	write_cmos_sensor(0x3a19,0xf8);
	write_cmos_sensor(0x3c01,0x34);
	write_cmos_sensor(0x3c04,0x28);
	write_cmos_sensor(0x3c05,0x98);
	write_cmos_sensor(0x3c07,0x07);
	write_cmos_sensor(0x3c09,0xc2);
	write_cmos_sensor(0x3c0a,0x9c);
	write_cmos_sensor(0x3c0b,0x40);
	write_cmos_sensor(0x3c01,0x34);
	write_cmos_sensor(0x4001,0x02);
	write_cmos_sensor(0x4004,0x06);
	write_cmos_sensor(0x4005,0x18);
	write_cmos_sensor(0x4050,0x6e);
	write_cmos_sensor(0x4051,0x8f);
	write_cmos_sensor(0x4300,0xf8);
	write_cmos_sensor(0x4514,0x00);
	write_cmos_sensor(0x4520,0xb0);
	write_cmos_sensor(0x460b,0x37);
	write_cmos_sensor(0x460c,0x20);
	write_cmos_sensor(0x4818,0x01);
	write_cmos_sensor(0x481d,0xf0);
	write_cmos_sensor(0x481f,0x50);
	write_cmos_sensor(0x4823,0x70);
	write_cmos_sensor(0x4831,0x14);
	write_cmos_sensor(0x4837,0x16);
	write_cmos_sensor(0x5000,0x06);
	write_cmos_sensor(0x5001,0x00);
	write_cmos_sensor(0x5003,0x00);
	write_cmos_sensor(0x5005,0x00);
	write_cmos_sensor(0x501d,0x00);
	write_cmos_sensor(0x501f,0x03);
	write_cmos_sensor(0x503d,0x00);
	write_cmos_sensor(0x505c,0x30);
	write_cmos_sensor(0x5181,0x59);
	write_cmos_sensor(0x5183,0x00);
	write_cmos_sensor(0x5191,0xf0);
	write_cmos_sensor(0x5192,0x03);
	write_cmos_sensor(0x5684,0x10);
	write_cmos_sensor(0x5685,0xa0);
	write_cmos_sensor(0x5686,0x0c);
	write_cmos_sensor(0x5687,0x78);
	write_cmos_sensor(0x5a00,0x08);
	write_cmos_sensor(0x5a21,0x00);
	write_cmos_sensor(0x5a24,0x00);
	//write_cmos_sensor(0x3503,0x03);
	write_cmos_sensor(0x3008,0x02);
	mDELAY(100);//delay 30ms
}	/*	preview_setting  */

static void capture_setting(void)
{
	preview_setting();
}

static void normal_video_setting(void)
{
printk("OV5645 normal_video_setting 20180804");
write_cmos_sensor(0x3103,0x11);
write_cmos_sensor(0x3008,0x82);
mDELAY(50);
write_cmos_sensor(0x3008,0x42);
write_cmos_sensor(0x3103,0x03);
write_cmos_sensor(0x3503,0x07);
write_cmos_sensor(0x3002,0x1c);
write_cmos_sensor(0x3006,0xc3);
write_cmos_sensor(0x300e,0x45);
write_cmos_sensor(0x3017,0x40);
write_cmos_sensor(0x3018,0x00);
write_cmos_sensor(0x302e,0x0b);
write_cmos_sensor(0x3037,0x13);
write_cmos_sensor(0x3108,0x01);
write_cmos_sensor(0x3611,0x06);
write_cmos_sensor(0x3612,0xab);
write_cmos_sensor(0x3614,0x50);
write_cmos_sensor(0x3618,0x04);
write_cmos_sensor(0x3034,0x1a);
write_cmos_sensor(0x3035,0x12);
write_cmos_sensor(0x3036,0x46);
write_cmos_sensor(0x3500,0x00);
write_cmos_sensor(0x3501,0x01);
write_cmos_sensor(0x3502,0x00);
write_cmos_sensor(0x350a,0x00);
write_cmos_sensor(0x350b,0x3f);
write_cmos_sensor(0x3600,0x09);
write_cmos_sensor(0x3601,0x43);
write_cmos_sensor(0x3620,0x33);
write_cmos_sensor(0x3621,0xe0);
write_cmos_sensor(0x3622,0x01);
write_cmos_sensor(0x3630,0x2d);
write_cmos_sensor(0x3631,0x00);
write_cmos_sensor(0x3632,0x32);
write_cmos_sensor(0x3633,0x52);
write_cmos_sensor(0x3634,0x70);
write_cmos_sensor(0x3635,0x13);
write_cmos_sensor(0x3636,0x03);
write_cmos_sensor(0x3702,0x6e);
write_cmos_sensor(0x3703,0x52);
write_cmos_sensor(0x3704,0xa0);
write_cmos_sensor(0x3705,0x33);
write_cmos_sensor(0x3708,0x66);
write_cmos_sensor(0x3709,0x12);
write_cmos_sensor(0x370b,0x61);
write_cmos_sensor(0x370c,0xc3);
write_cmos_sensor(0x370f,0x10);
write_cmos_sensor(0x3715,0x08);
write_cmos_sensor(0x3717,0x01);
write_cmos_sensor(0x371b,0x20);
write_cmos_sensor(0x3731,0x22);
write_cmos_sensor(0x3739,0x70);
write_cmos_sensor(0x3901,0x0a);
write_cmos_sensor(0x3905,0x02);
write_cmos_sensor(0x3906,0x10);
write_cmos_sensor(0x3719,0x86);
write_cmos_sensor(0x3800,0x00);
write_cmos_sensor(0x3801,0x00);
write_cmos_sensor(0x3802,0x00);
write_cmos_sensor(0x3803,0x06);
write_cmos_sensor(0x3804,0x0a);
write_cmos_sensor(0x3805,0x3f);
write_cmos_sensor(0x3806,0x07);
write_cmos_sensor(0x3807,0x9d);
write_cmos_sensor(0x3808,0x05);
write_cmos_sensor(0x3809,0x00);
write_cmos_sensor(0x380a,0x03);
write_cmos_sensor(0x380b,0xc0);
write_cmos_sensor(0x380c,0x07);
write_cmos_sensor(0x380d,0x68);
write_cmos_sensor(0x380e,0x03);
write_cmos_sensor(0x380f,0xd8);
write_cmos_sensor(0x3810,0x00);
write_cmos_sensor(0x3811,0x10);
write_cmos_sensor(0x3812,0x00);
write_cmos_sensor(0x3813,0x06);
write_cmos_sensor(0x3814,0x31);
write_cmos_sensor(0x3815,0x31);
write_cmos_sensor(0x3820,0x47);
write_cmos_sensor(0x3821,0x01);
write_cmos_sensor(0x3824,0x01);
write_cmos_sensor(0x3826,0x03);
write_cmos_sensor(0x3828,0x08);
write_cmos_sensor(0x3a02,0x03);
write_cmos_sensor(0x3a03,0xd8);
write_cmos_sensor(0x3a08,0x01);
write_cmos_sensor(0x3a09,0xf8);
write_cmos_sensor(0x3a0a,0x01);
write_cmos_sensor(0x3a0b,0xa4);
write_cmos_sensor(0x3a0e,0x02);
write_cmos_sensor(0x3a0d,0x02);
write_cmos_sensor(0x3a14,0x03);
write_cmos_sensor(0x3a15,0xd8);
write_cmos_sensor(0x3a18,0x00);
write_cmos_sensor(0x3a19,0xf8);
write_cmos_sensor(0x3c01,0x34);
write_cmos_sensor(0x3c04,0x28);
write_cmos_sensor(0x3c05,0x98);
write_cmos_sensor(0x3c07,0x07);
write_cmos_sensor(0x3c09,0xc2);
write_cmos_sensor(0x3c0a,0x9c);
write_cmos_sensor(0x3c0b,0x40);
write_cmos_sensor(0x3c01,0x34);
write_cmos_sensor(0x4001,0x02);
write_cmos_sensor(0x4004,0x02);
write_cmos_sensor(0x4005,0x18);
write_cmos_sensor(0x4050,0x6e);
write_cmos_sensor(0x4051,0x8f);
write_cmos_sensor(0x4300,0xf8);
write_cmos_sensor(0x4514,0xbb);
write_cmos_sensor(0x4520,0xb0);
write_cmos_sensor(0x460b,0x37);
write_cmos_sensor(0x460c,0x20);
write_cmos_sensor(0x4818,0x01);
write_cmos_sensor(0x481d,0xf0);
write_cmos_sensor(0x481f,0x50);
write_cmos_sensor(0x4823,0x70);
write_cmos_sensor(0x4831,0x14);
write_cmos_sensor(0x4837,0x21);
write_cmos_sensor(0x5000,0x06);
write_cmos_sensor(0x5001,0x00);
write_cmos_sensor(0x5003,0x00);
write_cmos_sensor(0x5005,0x00);
write_cmos_sensor(0x501d,0x00);
write_cmos_sensor(0x501f,0x03);
write_cmos_sensor(0x503d,0x00);
write_cmos_sensor(0x505c,0x30);
write_cmos_sensor(0x5181,0x59);
write_cmos_sensor(0x5183,0x00);
write_cmos_sensor(0x5191,0xf0);
write_cmos_sensor(0x5192,0x03);
write_cmos_sensor(0x5684,0x10);
write_cmos_sensor(0x5685,0xa0);
write_cmos_sensor(0x5686,0x0c);
write_cmos_sensor(0x5687,0x78);
write_cmos_sensor(0x5a00,0x08);
write_cmos_sensor(0x5a21,0x00);
write_cmos_sensor(0x5a24,0x00);
write_cmos_sensor(0x3008,0x02);
write_cmos_sensor(0x3503,0x00);
mDELAY(100);

}
static void hs_video_setting(void)
{
	normal_video_setting();
}

static void slim_video_setting(void)
{
	normal_video_setting();
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
    printk("enable: %d\n", enable);

    // 0x503D[8]: 1 enable,  0 disable
    // 0x503D[1:0]; 00 Color bar, 01 Random Data, 10 Square
    if(enable)
        write_cmos_sensor(0x503D, 0x80);
    else
        write_cmos_sensor(0x503D, 0x00);

    spin_lock(&imgsensor_drv_lock);
    imgsensor.test_pattern = enable;
    spin_unlock(&imgsensor_drv_lock);
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*	get_imgsensor_id
*
* DESCRIPTION
*	This function get the sensor ID 
*
* PARAMETERS
*	*sensorID : return the sensor ID 
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id) 
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	//sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
            *sensor_id = return_sensor_id();
			if (*sensor_id == imgsensor_info.sensor_id) {	
				printk("ov5645 i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,*sensor_id);	  
				return ERROR_NONE;
			}	
            printk("ov5645--------Read sensor id fail, write id: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id,*sensor_id);
			retry--;
		} while(retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		// if Sensor ID is not correct, Must set *sensor_id to 0xFFFFFFFF 
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*	open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
	//const kal_uint8 i2c_addr[] = {IMGSENSOR_WRITE_ID_1, IMGSENSOR_WRITE_ID_2};
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0; 
	//LOG_1;	
	//LOG_2;
	
	//sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
            sensor_id = return_sensor_id();
			if (sensor_id == imgsensor_info.sensor_id) {
				printk("ov5645back i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);	  
				break;
			}	
            printk("ov5645back Read sensor id fail, write id: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
			retry--;
		} while(retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}		 
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;
	
	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en= KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.shutter = 0x3D0;
	imgsensor.gain = 0x100;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = KAL_FALSE;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}	/*	open  */



/*************************************************************************
* FUNCTION
*	close
*
* DESCRIPTION
*	
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	//LOG_INF("E\n");

	/*No Need to implement this function*/ 
	
	return ERROR_NONE;
}	/*	close  */


/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	//imgsensor.video_mode = KAL_FALSE;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength; 
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();
	return ERROR_NONE;
}	/*	preview   */

/*************************************************************************
* FUNCTION
*	capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                          MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    //LOG_INF("E\n");
	
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
    if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {//PIP capture: 24fps for less than 13M, 20fps for 16M,15fps for 20M
        imgsensor.pclk = imgsensor_info.cap1.pclk;
        imgsensor.line_length = imgsensor_info.cap1.linelength;
        imgsensor.frame_length = imgsensor_info.cap1.framelength;
        imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
        imgsensor.autoflicker_en = KAL_FALSE;
    } else {
        if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
            //LOG_INF("Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",imgsensor.current_fps,imgsensor_info.cap.max_framerate/10);
        imgsensor.pclk = imgsensor_info.cap.pclk;
        imgsensor.line_length = imgsensor_info.cap.linelength;
        imgsensor.frame_length = imgsensor_info.cap.framelength;
        imgsensor.min_frame_length = imgsensor_info.cap.framelength;
        imgsensor.autoflicker_en = KAL_FALSE;
    }
    spin_unlock(&imgsensor_drv_lock);
    capture_setting();
	if(imgsensor.test_pattern == KAL_TRUE)
	{
		set_test_pattern_mode(TRUE);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.test_pattern = KAL_FALSE;
		spin_unlock(&imgsensor_drv_lock);
	}
    return ERROR_NONE;
}    /* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//LOG_INF("E\n");
	
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;  
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting(); 	
	return ERROR_NONE;
}	/*	normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//LOG_INF("E\n");
	
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength; 
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//LOG_INF("E\n");
	
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength; 
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	return ERROR_NONE;
}	/*	slim_video	 */



    /*add by xunhu andy andy20160720 at 22:51 end*/
static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	//LOG_INF("E\n");
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;
	
	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;		

	
	sensor_resolution->SensorHighSpeedVideoWidth	 = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight	 = imgsensor_info.hs_video.grabwindow_height;
	
	sensor_resolution->SensorSlimVideoWidth	 = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight	 = imgsensor_info.slim_video.grabwindow_height;
	return ERROR_NONE;
}	/*	get_resolution	*/

static kal_uint32 get_info(MSDK_SCENARIO_ID_ENUM scenario_id,
					  MSDK_SENSOR_INFO_STRUCT *sensor_info,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//LOG_INF("scenario_id = %d\n", scenario_id);



	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;
    sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
    sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
    sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
    sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
    sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;
	
	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame; 		 /* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;	/* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;	
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	
	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num; 
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */
	
	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  // 0 is default 1x
	sensor_info->SensorHightSampling = 0;	// 0 is default 1x 
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;		
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			sensor_info->SensorGrabStartX = imgsensor_info.cap.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.cap.mipi_data_lp2hs_settle_dc; 

			break;	 
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			
			sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;
	   
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc; 

			break;	  
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:			
			sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc; 

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc; 

            break;
		default:			
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;		
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}
	
	return ERROR_NONE;
}	/*	get_info  */


static kal_uint32 control(MSDK_SCENARIO_ID_ENUM scenario_id, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			preview(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			capture(image_window, sensor_config_data);
			break;	
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			normal_video(image_window, sensor_config_data);
			break;	  
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			hs_video(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			slim_video(image_window, sensor_config_data);
			break;	  
		default:
			//LOG_INF("Error ScenarioId setting");
			preview(image_window, sensor_config_data);
			return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}	/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	//LOG_INF("framerate = %d\n ", framerate);
	// SetVideoMode Function should fix framerate
	if (framerate == 0)
		// Dynamic frame rate
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps,1);
	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	//LOG_INF("enable = %d, framerate = %d \n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) //enable auto flicker	  
		imgsensor.autoflicker_en = KAL_TRUE;
	else //Cancel Auto flick
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate) 
{
	kal_uint32 frame_length;
	
	//LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength):0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
            //set_dummy();
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if(framerate == 0)
				return ERROR_NONE;
			frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength):0;			
			imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
            //set_dummy();
			break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        	  if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
                frame_length = imgsensor_info.cap1.pclk / framerate * 10 / imgsensor_info.cap1.linelength;
                spin_lock(&imgsensor_drv_lock);
		            imgsensor.dummy_line = (frame_length > imgsensor_info.cap1.framelength) ? (frame_length - imgsensor_info.cap1.framelength) : 0;
		            imgsensor.frame_length = imgsensor_info.cap1.framelength + imgsensor.dummy_line;
		            imgsensor.min_frame_length = imgsensor.frame_length;
		            spin_unlock(&imgsensor_drv_lock);
            } else {
        		    if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
                    //LOG_INF("Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",framerate,imgsensor_info.cap.max_framerate/10);
//                frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
//                spin_lock(&imgsensor_drv_lock);
//		            imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
//		            imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
//		            imgsensor.min_frame_length = imgsensor.frame_length;
		            spin_unlock(&imgsensor_drv_lock);
            }
            //set_dummy();
			break;	
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
            //set_dummy();
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength): 0;
			imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
            //set_dummy();
			break;		
		default:  //coding with  preview scenario by default
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
            //set_dummy();
			//LOG_INF("error scenario_id = %d, we use preview scenario \n", scenario_id);
			break;
	}	
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate) 
{
	//LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*framerate = imgsensor_info.pre.max_framerate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*framerate = imgsensor_info.normal_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*framerate = imgsensor_info.cap.max_framerate;
			break;		
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*framerate = imgsensor_info.hs_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO: 
			*framerate = imgsensor_info.slim_video.max_framerate;
			break;
		default:
			break;
	}

    return ERROR_NONE;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
							 UINT8 *feature_para,UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16=(UINT16 *) feature_para;
	UINT16 *feature_data_16=(UINT16 *) feature_para;
	UINT32 *feature_return_para_32=(UINT32 *) feature_para;
	UINT32 *feature_data_32=(UINT32 *) feature_para;
	unsigned long long *feature_data=(unsigned long long *) feature_para;
//    unsigned long long *feature_return_para=(unsigned long long *) feature_para;

	SENSOR_WINSIZE_INFO_STRUCT *wininfo;	
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data=(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;
 
	//LOG_INF("feature_id = %d\n", feature_id);
	switch (feature_id) {
		case SENSOR_FEATURE_GET_PERIOD:
			*feature_return_para_16++ = imgsensor.line_length;
			*feature_return_para_16 = imgsensor.frame_length;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:	 
			//LOG_INF("feature_Control imgsensor.pclk = %d,imgsensor.current_fps = %d\n", imgsensor.pclk,imgsensor.current_fps);
			*feature_return_para_32 = imgsensor.pclk;
			*feature_para_len=4;
			break;		   
		case SENSOR_FEATURE_SET_ESHUTTER:
			set_shutter(*feature_data);
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			night_mode((BOOL) *feature_data);
			break;
		case SENSOR_FEATURE_SET_GAIN:		
			set_gain((UINT16) *feature_data);
			break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
			break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
			break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*feature_return_para_32=LENS_DRIVER_ID_DO_NOT_CARE;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
			set_video_mode(*feature_data);
			break; 
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			get_imgsensor_id(feature_return_para_32); 
			break; 
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			set_auto_flicker_mode((BOOL)*feature_data_16,*(feature_data_16+1));
			break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			set_max_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*feature_data, *(feature_data+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			get_default_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*feature_data, (MUINT32 *)(uintptr_t)(*(feature_data+1)));
			break;
		case SENSOR_FEATURE_SET_TEST_PATTERN:
			set_test_pattern_mode((BOOL)*feature_data);
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: //for factory mode auto testing			 
			*feature_return_para_32 = imgsensor_info.checksum_value;
			*feature_para_len=4;							 
			break;				
		case SENSOR_FEATURE_SET_FRAMERATE:
			//LOG_INF("current fps :%d\n",  (UINT32)*feature_data);
			spin_lock(&imgsensor_drv_lock);
			imgsensor.current_fps = *feature_data;
			spin_unlock(&imgsensor_drv_lock);		
			break;
		case SENSOR_FEATURE_SET_HDR:
			//LOG_INF("ihdr enable :%d\n", (BOOL)*feature_data);
			spin_lock(&imgsensor_drv_lock);
			imgsensor.ihdr_en = (bool)*feature_data;
			spin_unlock(&imgsensor_drv_lock);		
			break;
		case SENSOR_FEATURE_GET_CROP_INFO:
			//LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n", (UINT32)*feature_data);
			wininfo = (SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));
		
			switch (*feature_data_32) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[1],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;	  
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[2],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[3],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[4],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				default:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[0],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
					break;
			}
            break;
		case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
			//LOG_INF("SENSOR_SET_SENSOR_IHDR is no support");
			//LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",(UINT16)*feature_data_32,(UINT16)*(feature_data_32+1),(UINT16)*(feature_data_32+2)); 
			//ihdr_write_shutter_gain((UINT16)*feature_data_32,(UINT16)*(feature_data_32+1),(UINT16)*(feature_data_32+2));	
			break;
		default:
			break;
	}
  
	return ERROR_NONE;
}	/*	feature_control()  */

static SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 OV5645MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&sensor_func;
	return ERROR_NONE;
}	/*	OV5693_MIPI_RAW_SensorInit	*/
