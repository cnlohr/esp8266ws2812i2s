FW_FILE_1:=0x00000.bin
FW_FILE_2:=0x40000.bin
TARGET_OUT:=image.elf
all : $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)


SRCS:=driver/uart.c \
	common/mystuff.c \
	common/flash_rewriter.c \
	common/http.c \
	common/commonservices.c \
	common/http_custom.c \
	common/mdns.c \
	common/mfs.c \
	user/custom_commands.c \
	user/ws2812_i2s.c \
	user/user_main.c

SRC_PATH:=/usr/src
SDK_PATH:=$(SRC_PATH)/esp-open-sdk
GCC_FOLDER:=$(SDK_PATH)/xtensa-lx106-elf
ESPTOOL_PY:=$(SDK_PATH)/esptool/esptool.py
FW_TOOL:=$(SRC_PATH)/esptool-ck/esptool
SDK:=$(SDK_PATH)/esp_iot_sdk_v1.4.0
PORT:=/dev/ttyUSB0
#PORT:=/dev/ttyACM0

XTLIB:=$(SDK)/lib
XTGCCLIB:=$(GCC_FOLDER)/lib/gcc/xtensa-lx106-elf/4.8.2/libgcc.a
FOLDERPREFIX:=$(GCC_FOLDER)/bin
PREFIX:=$(FOLDERPREFIX)/xtensa-lx106-elf-
CC:=$(PREFIX)gcc

CFLAGS:=-mlongcalls -I$(SDK)/include -Imyclib -Iinclude -Iuser -Os -I$(SDK)/include/ -Icommon -DICACHE_FLASH

#	   \
#

LDFLAGS_CORE:=\
	-nostdlib \
	-Wl,--relax -Wl,--gc-sections \
	-L$(XTLIB) \
	-L$(XTGCCLIB) \
	$(SDK)/lib/liblwip.a \
	$(SDK)/lib/libssl.a \
	$(SDK)/lib/libupgrade.a \
	$(SDK)/lib/libnet80211.a \
	$(SDK)/lib/liblwip.a \
	$(SDK)/lib/libwpa.a \
	$(SDK)/lib/libnet80211.a \
	$(SDK)/lib/libphy.a \
	$(SDK)/lib/libmain.a \
	$(SDK)/lib/libpp.a \
	$(XTGCCLIB) \
	-T $(SDK)/ld/eagle.app.v6.ld

LINKFLAGS:= \
	$(LDFLAGS_CORE) \
	-B$(XTLIB)

#image.elf : $(OBJS)
#	$(PREFIX)ld $^ $(LDFLAGS) -o $@

$(TARGET_OUT) : $(SRCS)
	$(PREFIX)gcc $(CFLAGS) $^  -flto $(LINKFLAGS) -o $@



$(FW_FILE_1): $(TARGET_OUT)
	@echo "FW $@"
	$(FW_TOOL) -eo $(TARGET_OUT) -bo $@ -bs .text -bs .data -bs .rodata -bc -ec

$(FW_FILE_2): $(TARGET_OUT)
	@echo "FW $@"
	$(FW_TOOL) -eo $(TARGET_OUT) -es .irom0.text $@ -ec

burn : $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL_PY) --port $(PORT) write_flash 0x00000 0x00000.bin 0x40000 0x40000.bin)||(true)

#If you have space, MFS should live at 0x100000, if you don't it can also live at
#0x10000.  But, then it is limited to 180kB.  You might need to do this if you have a 512kB
#ESP variant.

burnweb : web/page.mpfs
	($(ESPTOOL_PY) --port $(PORT) write_flash 0x10000 web/page.mpfs)||(true)


IP?=192.168.4.1

netburn : image.elf $(FW_FILE_1) $(FW_FILE_2)
	web/execute_reflash $(IP) 0x00000.bin 0x40000.bin

clean :
	rm -rf user/*.o driver/*.o $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)


