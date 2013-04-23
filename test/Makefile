ifdef DEBUG
DEBUG_OPT= DEBUG=1
else
DEBUG_OPT=
endif

all:
	@cd zlib && make ${DEBUG_OPT}
	@cd lib_test && make ${DEBUG_OPT}
	@cd malloc_hook && make ${DEBUG_OPT}

clean:
	@cd zlib && make clean
	@cd lib_test && make clean
	@cd malloc_hook && make clean

.PHONY: all clean
