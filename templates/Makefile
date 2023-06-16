TARGET=ciel
EXECUTABLE=$(TARGET).elf

#GIT_VERSION := $(shell git describe --tags)

CUBE=submodules/STM32CubeF7_Drivers
HALS=$(CUBE)/STM32F7xx_HAL_Driver/Src
# USBD=submodules/STM32CubeF7/Middlewares/ST/STM32_USB_Device_Library
# WRLIB=submodules/wrLib
# WRMATH=submodules/wrLib/math
# WRDSP=submodules/wrLib/dsp
# FGLIB=../fglib
#LUAS=submodules/lua/src
#BOOTLOADER=submodules/dfu-stm32f7
BUILD_DIR := build
PRJ_DIR=ciel/templates

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump

BIN = $(TARGET).bin

DEFS = -DSTM32F7XX -DARM_MATH_CM7 -DHSE_VALUE=8000000
DEFS += -DSTM32F767xx -DUSE_HAL_DRIVER
STARTUP = $(CUBE)/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f767xx.s

MCFLAGS = -mcpu=cortex-m7 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

STM32_INCLUDES = \
	-I$(CUBE)/CMSIS/Device/ST/STM32F7xx/Include/ \
	-I$(CUBE)/CMSIS/Include/ \
	-I$(CUBE)/STM32F7xx_HAL_Driver/Inc/ \
	-I/usr/local/include/ \
# 	-Iusbd/ \
# 	-I$(USBD)/Class/CDC/Inc/ \
# 	-I$(USBD)/Core/Inc/ \
# 	-I$(CARP_DIR)/core/ \
# 	-I$(WRLIB)/math/ \
# 	-I$(WRLIB)/dsp/ \

OPTIMIZE       = -O3

CFLAGS += -std=c99
CFLAGS += -Wall
CFLAGS += -Wno-unused-function -Wno-unused-variable
CFLAGS += $(MCFLAGS)
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEFS) -I. -I./ $(STM32_INCLUDES)
CFLAGS += -fsingle-precision-constant
#CFLAGS += -Wdouble-promotion
# CFLAGS += -DLUA_32BITS -DLUA_COMPAT_5_2
CFLAGS += -fno-common
CFLAGS += -DVERSION=\"$(GIT_VERSION)\"
CFLAGS += -ffunction-sections -fdata-sections # provides majority of LTO binary size reduction
CFLAGS += -Wno-maybe-uninitialized

# release: if (=1), disable all debug prints
R ?= 0
ifeq ($(R), 1)
	CFLAGS += -DRELEASE
	#CFLAGS += -flto # broken in debug mode. provides a small LTO binary size reduction
else
	CFLAGS += -DDEBUG
endif

LDFLAGS = -Wl,-T,STM32F767ZIT.ld,-flto,-gc-sections
#LDFLAGS = -Wl,-T,STM32F767ZIT.ld
LIBS = -lm -lc -lnosys

SRC += main.c \
	stm32f7xx_it.c \
	system.c \
	oldlib/dout.c \
	oldlib/gpio.c \

SRC += system_stm32f7xx.c \
	$(HALS)/stm32f7xx_hal.c \
	$(HALS)/stm32f7xx_hal_cortex.c \
	$(HALS)/stm32f7xx_hal_gpio.c \
	$(HALS)/stm32f7xx_hal_pwr.c \
	$(HALS)/stm32f7xx_hal_pwr_ex.c \
	$(HALS)/stm32f7xx_hal_rcc.c \
	$(HALS)/stm32f7xx_hal_rcc_ex.c \
# 	$(HALS)/stm32f7xx_hal_adc.c \
# 	$(HALS)/stm32f7xx_hal_dac.c \
# 	$(HALS)/stm32f7xx_hal_dma.c \
# 	$(HALS)/stm32f7xx_hal_i2c.c \
# 	$(HALS)/stm32f7xx_hal_ltdc.c \
# 	$(HALS)/stm32f7xx_hal_pcd.c \
# 	$(HALS)/stm32f7xx_hal_pcd_ex.c \
# 	$(HALS)/stm32f7xx_hal_sai.c \
# 	$(HALS)/stm32f7xx_hal_tim.c \
# 	$(HALS)/stm32f7xx_hal_tim_ex.c \
# 	$(HALS)/stm32f7xx_hal_uart.c \
# 	$(HALS)/stm32f7xx_hal_usart.c \
# 	$(HALS)/stm32f7xx_ll_usb.c \
#	$(HALS)/stm32f7xx_hal_flash.c \
#	$(HALS)/stm32f7xx_hal_flash_ex.c \
#	$(HALS)/stm32f7xx_hal_i2s.c \
#	$(HALS)/stm32f7xx_hal_dma2d.c \
#	$(HALS)/stm32f7xx_hal_rng.c \
#	$(HALS)/stm32f7xx_hal_spi.c \

# SRC += \
# 	$(USBD)/Core/Src/usbd_core.c \
# 	$(USBD)/Core/Src/usbd_ctlreq.c \
# 	$(USBD)/Core/Src/usbd_ioreq.c \
# 	$(USBD)/Class/CDC/Src/usbd_cdc.c \

# SRC += \
# 	$(WRMATH)/str_buffer.c \
# 	$(WRMATH)/wrQueue.c \

# SRC += \
# 	$(WRDSP)/wrFilter.c \

# SRC += \
# 	$(FGLIB)/system.c \
# 	$(FGLIB)/din.c \
# 	$(FGLIB)/dout.c \
# 	$(FGLIB)/gpio.c \
# 	$(FGLIB)/i2c.c \
# 	$(FGLIB)/timers.c \
# 	$(FGLIB)/usart.c \
# 	$(FGLIB)/uart.c \
# 	$(FGLIB)/adc.c \
# 	$(FGLIB)/dac.c \
# 	$(FGLIB)/mux.c \
# 	$(FGLIB)/tty.c \
# 	$(FGLIB)/param.c \


# lua tests
# LTESTS = $(wildcard tests/*.lua) \


# recipes!
all: $(TARGET).hex $(BIN)


### pre-process only files

### i2c descriptors
##II_SRCD = lua/ii
##II_SRC = $(wildcard $(II_SRCD)/*.lua)
##II_TARGET = $(addprefix $(BUILD_DIR)/ii_, $(notdir $(II_SRC)))
##
##$(II_TARGET): util/ii_lua_module.lua
##
##$(BUILD_DIR)/ii_%.lua: $(II_SRCD)/%.lua util/ii_lua_module.lua | $(BUILD_DIR)
##	@lua util/ii_lua_module.lua $< $@
##	@echo lua $@
##
##$(BUILD_DIR)/iihelp.lua: $(II_SRC) util/ii_lua_help.lua | $(BUILD_DIR)
##	@lua util/ii_lua_help.lua $(II_SRCD) $@
##	@echo lua $@
##
##$(BUILD_DIR)/ii_c_layer.h: $(II_SRC) util/ii_c_layer.lua | $(BUILD_DIR)
##	@lua util/ii_c_layer.lua $(II_SRCD) $@
##	@echo lua $@
##
##$(BUILD_DIR)/ii_lualink.h: $(II_SRC) util/ii_lualinker.lua | $(BUILD_DIR)
##	@lua util/ii_lualinker.lua $(II_SRCD) $@
##	@echo lua $@
##
##
##### destination sources
##
### lua srcs: these get converted to bytecode strings wrapped in c-headers
##LUA_SRC  = $(wildcard lua/*.lua)
##LUA_SRC += $(BUILD_DIR)/iihelp.lua
##LUA_SRC += $(II_TARGET)
##
##LUA_PP = $(LUA_SRC:%.lua=%.lua.h)
##LUA_PP: $(LUA_SRC)
##
##LUACORE_OBJS=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
##		lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
##		ltm.o lundump.o lvm.o lzio.o
##LUALIB_OBJS=	lauxlib.o lbaselib.o lbitlib.o lcorolib.o ldblib.o liolib.o \
##		lmathlib.o loslib.o lstrlib.o ltablib.o lutf8lib.o loadlib.o linit.o
##

# build the objects from c source
OBJDIR = .
OBJS = $(SRC:%.c=$(OBJDIR)/%.o)
#OBJS += $(addprefix $(LUAS)/,$(LUACORE_OBJS) $(LUALIB_OBJS) )
OBJS += Startup.o

# specific objects that require built dependencies (ii)
##$(OBJDIR)/lib/lualink.o: $(LUA_PP) $(BUILD_DIR)/ii_lualink.h
##$(OBJDIR)/lib/ii.o: $(BUILD_DIR)/ii_c_layer.h

# generate the build directory
##$(BUILD_DIR):
##	@echo build_dir $(BUILD_DIR)/
##	@mkdir -p $(BUILD_DIR)


# C dependencies echoed into Makefile
DEP = $(OBJS:.o=.d)  # one dependency file for each source


# OS dependent size printing
UNAME := $(shell uname)

GETSIZE = stat

ifeq ($(UNAME), Darwin)
	GETSIZE = stat -x
endif


.PHONY: tests
tests:
	@for t in $(LTESTS); do \
		lua $$t; \
	done

# include all DEP files in the makefile
# will rebuild elements if dependent C headers are changed
# FIXME: currently causes compiler warning due to missing .lua.h files
-include $(DEP)

$(TARGET).hex: $(EXECUTABLE)
	@$(CP) -O ihex $^ $@

$(EXECUTABLE): $(OBJS)
	@$(LD) -g $(MCFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	@echo "linked:       $@"
	@$(OBJDUMP) --disassemble $@ > $@.lst
	@echo "disassembly:  $@.lst"

$(BIN): $(EXECUTABLE)
	@$(CP) -O binary $< $@
	@echo "binary:       $@"
	@$(OBJDUMP) -x --syms $< > $(addsuffix .dmp, $(basename $<))
	@echo "symbol table: $@.dmp"
	@echo "Release: "$(R)
	@$(GETSIZE) $(BIN) | grep 'Size'
	@echo "        ^ must be less than 384kB (384,000)"
	# 512kb -64kb(bootloader) -128kb(scripts)

flash: $(BIN)
	st-flash --reset write $(BIN) 0x08000000

dfu: $(BIN)
	sudo dfu-util -a 0 -s 0x08000000 -R -D $(BIN) -d ,0483:df11

%.o: %.c
	@$(CC) -ggdb $(CFLAGS) -c $< -o $@
	@echo $@

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.s: %.c
	@$(CC) -ggdb $(CFLAGS) -S $< -o $@

# out/main.c: main.carp
# 	carp build.carp --no-core

# %.lua.h: %.lua util/l2h.lua
# 	@luac -p $<
# 	@echo l2h $< "->" $@
# 	@lua util/l2h.lua $<

Startup.o: $(STARTUP)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo $@

erase:
	st-flash erase

.PHONY: clean
clean:
	@rm -rf Startup.lst $(TARGET).elf.lst $(OBJS) $(AUTOGEN) \
	$(TARGET).bin  $(TARGET).out  $(TARGET).hex \
	$(TARGET).map  $(TARGET).dmp  $(EXECUTABLE) $(DEP) \
	$(TARGET)-$(GIT_VERSION)/  *.zip \
# 	$(BUILD_DIR) lua/*.lua.h out/main.c \

splint:
	splint -I. -I./ $(STM32_INCLUDES) *.c
