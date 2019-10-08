
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/hwmon/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps/inc


obj-y	+=  icn87xx.o
obj-y	+=  flash.o

CTP_DEFINES := $(shell echo $(CONFIG_TOUCHSCREEN_CUSTOM_DEFINE_PARA) | tr a-z A-Z)
CTP_CONFIG_DEFINE := $(foreach n,$(CTP_DEFINES),-D$(n))

$(info $(CTP_DEFINES) )
$(info $(CTP_CONFIG_DEFINE) )

ccflags-y += $(CTP_CONFIG_DEFINE)