#=============================================================================
# BYOVD Loader Makefile
# Builds the C port of the BYOVD exploitation chain for Ld9BoxSup.sys
#=============================================================================

# Compiler settings for MSVC (cl.exe)
# This Makefile is designed for Windows builds using MSVC
CC = cl.exe
LINK = link.exe

# Output directories
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)\obj
BIN_DIR = $(BUILD_DIR)\bin

# Output files
LIB_NAME = byovd.lib
TEST_NAME = byovd_test.exe

# Source files
SRCS = \
	nt_helpers.c \
	supdrv.c \
	throttlestop.c \
	crypto.c \
	deployer.c

# Object files
OBJS = $(SRCS:.c=.obj)
OBJS_PATH = $(addprefix $(OBJ_DIR)\,$(OBJS))

# Header files (for dependency tracking)
HDRS = \
	types.h \
	nt_defs.h \
	supdrv_types.h \
	supdrv.h \
	throttlestop.h \
	crypto.h \
	deployer.h

# Compiler flags
CFLAGS = /nologo /c /W4 /WX- /O2 /GS /Zi \
         /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" \
         /D "_CRT_SECURE_NO_WARNINGS" \
         /D "BYOVD_DEBUG_LOGGING" \
         /Fo"$(OBJ_DIR)\\" /Fd"$(OBJ_DIR)\\"

# Debug build flags
CFLAGS_DEBUG = /nologo /c /W4 /WX- /Od /GS /Zi /RTC1 \
               /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" \
               /D "_CRT_SECURE_NO_WARNINGS" \
               /D "BYOVD_DEBUG_LOGGING" \
               /Fo"$(OBJ_DIR)\\" /Fd"$(OBJ_DIR)\\"

# Linker flags
LDFLAGS = /nologo /MACHINE:X64
LIBS = kernel32.lib advapi32.lib

# Library flags
LIBFLAGS = /nologo /OUT:"$(BIN_DIR)\$(LIB_NAME)"

#=============================================================================
# Targets
#=============================================================================

.PHONY: all clean dirs release debug lib test

# Default target
all: release

# Create directories
dirs:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

# Release build
release: dirs lib

# Debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: dirs lib

# Build static library
lib: dirs $(OBJS_PATH)
	@echo Building static library...
	lib $(LIBFLAGS) $(OBJS_PATH)
	@echo Library built: $(BIN_DIR)\$(LIB_NAME)

# Build test executable (requires test_main.c)
test: lib
	@echo Building test executable...
	$(CC) $(CFLAGS) test_main.c
	$(LINK) $(LDFLAGS) /OUT:"$(BIN_DIR)\$(TEST_NAME)" \
		$(OBJ_DIR)\test_main.obj $(BIN_DIR)\$(LIB_NAME) $(LIBS)
	@echo Test built: $(BIN_DIR)\$(TEST_NAME)

# Object file rules
$(OBJ_DIR)\nt_helpers.obj: nt_helpers.c nt_defs.h types.h
	$(CC) $(CFLAGS) nt_helpers.c

$(OBJ_DIR)\supdrv.obj: supdrv.c supdrv.h supdrv_types.h nt_defs.h types.h
	$(CC) $(CFLAGS) supdrv.c

$(OBJ_DIR)\throttlestop.obj: throttlestop.c throttlestop.h nt_defs.h types.h
	$(CC) $(CFLAGS) throttlestop.c

$(OBJ_DIR)\crypto.obj: crypto.c crypto.h types.h
	$(CC) $(CFLAGS) crypto.c

$(OBJ_DIR)\deployer.obj: deployer.c deployer.h crypto.h supdrv_types.h nt_defs.h types.h
	$(CC) $(CFLAGS) deployer.c

# Clean build artifacts
clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@echo Clean complete.

#=============================================================================
# MinGW-w64 Alternative (for cross-compilation from Linux/macOS)
#=============================================================================
# Uncomment below for MinGW-w64 builds

# MINGW_CC = x86_64-w64-mingw32-gcc
# MINGW_AR = x86_64-w64-mingw32-ar
# MINGW_CFLAGS = -Wall -Wextra -O2 -DWIN32 -D_WINDOWS -DBYOVD_DEBUG_LOGGING
# MINGW_LDFLAGS = -lkernel32 -ladvapi32
#
# mingw: dirs
# 	$(MINGW_CC) $(MINGW_CFLAGS) -c nt_helpers.c -o $(OBJ_DIR)/nt_helpers.o
# 	$(MINGW_CC) $(MINGW_CFLAGS) -c supdrv.c -o $(OBJ_DIR)/supdrv.o
# 	$(MINGW_CC) $(MINGW_CFLAGS) -c throttlestop.c -o $(OBJ_DIR)/throttlestop.o
# 	$(MINGW_CC) $(MINGW_CFLAGS) -c crypto.c -o $(OBJ_DIR)/crypto.o
# 	$(MINGW_CC) $(MINGW_CFLAGS) -c deployer.c -o $(OBJ_DIR)/deployer.o
# 	$(MINGW_AR) rcs $(BIN_DIR)/libbyovd.a $(OBJ_DIR)/*.o
# 	@echo MinGW library built: $(BIN_DIR)/libbyovd.a

#=============================================================================
# Usage
#=============================================================================
#
# Build release library:
#   nmake
#   nmake release
#
# Build debug library:
#   nmake debug
#
# Build and run tests:
#   nmake test
#
# Clean build artifacts:
#   nmake clean
#
# Cross-compile with MinGW-w64:
#   make mingw
#
#=============================================================================
