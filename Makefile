include user.cfg
include esp82xx/common.mf
include esp82xx/main.mf

SRCS += user/ws2812_i2s.c

#Useful git commands

init_submodule :
	git submodule update --init --recursive

