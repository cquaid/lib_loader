SRC_DIR = src
SRC_FILES = ${SRC_DIR}/rtld.c ${SRC_DIR}/list.c ${SRC_DIR}/bintree.c
OBJECTS = rtld.o list.o bintree.o

LIB_NAME = lib_loader.so
INCLUDE = include

LDFLAGS = -shared -Wl,-soname,${LIB_NAME}
CFLAGS = -std=gnu99 -pedantic -Wall -fpic -I${INCLUDE}

ifdef DEBUG
CFLAGS += -g -DDEBUG
DEBUG_OPT= DEBUG=1
else
DEBUG_OPT=
endif

all: lib_loader test

lib_loader:
	@echo CC -c ${SRC_FILES}
	@${CC} ${CFLAGS} -c ${SRC_FILES}
	@echo CC -o ${LIB_NAME}
	@${CC} ${LDFLAGS} -o ${LIB_NAME} ${OBJECTS}
	@mkdir -p lib
	@mv ${LIB_NAME} lib/${LIB_NAME}
	@cp lib/${LIB_NAME} lib/lib${LIB_NAME}
test:
	@echo "building test objects"
	@cd test && make ${DEBUG_OPT}

clean:
	@echo "cleaning"
	@rm ${OBJECTS}
	@rm -rf lib
	@cd test && make clean

.PHONY: all lib_loader test clean