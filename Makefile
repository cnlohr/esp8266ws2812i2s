include user.cfg
-include esp82xx/common.mf
-include esp82xx/main.mf

SRCS += user/ws2812_i2s.c user/pattern.c

% :
	$(warning This is the empty rule. Something went wrong.)
	@true

ifndef TARGET
$(info Modules were not checked out... use git clone --recursive in the future. Pulling now.)
$(shell git submodule update --init --recursive)
endif

purge : clean
	@cd web && $(MAKE) $(MFLAGS) $(MAKEOVERRIDES) clean
	@cd wsend && $(MAKE) $(MFLAGS) $(MAKEOVERRIDES) clean
	$(RM) $(FW_FILE1) $(FW_FILE2) $(BIN_TARGET)
