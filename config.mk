
ifdef DEBUG
DEBUG_OPT = DEBUG=1
DEBUG_FLAGS = -g -DDEBUG
WARNINGS = -Wall -Wextra -Wconversion -Werror
else
DEBUG_OPT=
DEBUG_FLAGS=
WARNINGS = -Wall
endif

RTLD_LIB ?= rtld
RTLD_LIB_STATIC = lib$(RTLD_LIB).a
RTLD_LIB_DYNAMIC = lib$(RTLD_LIB).so

RTLD_CFLAGS = -std=gnu99 -pedantic $(WARNINGS) $(DEBUG_FLAGS)
RTLD_LDFLAGS = -shared -Wl,-soname,$(RTLD_LIB)
