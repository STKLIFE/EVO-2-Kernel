ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/gslx68x/
ifneq (,$(findstring config,$(CONFIG_GSLX68X_CONFIG)))
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/gslx68x/include/$(CONFIG_GSLX68X_CONFIG)/
else
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/gslx68x/
endif
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/hwmon/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps/inc

obj-y	+= gsl_point_id_20150706.o
obj-y	+= mtk_gslX68X.o

 
CTP_DEFINES := $(shell echo $(CONFIG_TOUCHSCREEN_CUSTOM_DEFINE_PARA) | tr a-z A-Z)
CTP_CONFIG_DEFINE := $(foreach n,$(CTP_DEFINES),-D$(n))

$(info $(CTP_DEFINES) )
$(info $(CTP_CONFIG_DEFINE) )

ccflags-y += $(CTP_CONFIG_DEFINE)