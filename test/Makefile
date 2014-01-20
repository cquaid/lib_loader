ifdef DEBUG
DEBUG_OPT= DEBUG=1
else
DEBUG_OPT=
endif

all:
	@$(MAKE) -C libc $(DEBUG_OPT)
	@$(MAKE) -C zlib $(DEBUG_OPT)
	@$(MAKE) -C lib_test $(DEBUG_OPT)
	@$(MAKE) -C malloc_hook $(DEBUG_OPT)

.PHONY: clean
clean:
	@$(MAKE) -C libc clean
	@$(MAKE) -C zlib clean
	@$(MAKE) -C lib_test clean
	@$(MAKE) -C malloc_hook clean
