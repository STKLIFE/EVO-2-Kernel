
# Linux driver folder
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/$(CONFIG_TOUCHSCREEN_MTK_FTS_DIRECTORY)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/hwmon/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/base
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/hwmon/include

obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_ex_fun.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_flash.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_gesture.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_core.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_esdcheck.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_ex_mode.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_proximity.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_i2c.o
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH)    +=  focaltech_point_report_check.o

obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH) +=     focaltech_test/
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH) +=     focaltech_flash/

CTP_DEFINES := $(shell echo $(CONFIG_TOUCHSCREEN_CUSTOM_DEFINE_PARA) | tr a-z A-Z)
CTP_CONFIG_DEFINE := $(foreach n,$(CTP_DEFINES),-D$(n))

$(info $(CTP_DEFINES) )
$(info $(CTP_CONFIG_DEFINE) )

ccflags-y += $(CTP_CONFIG_DEFINE)
