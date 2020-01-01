# -------------------------------------------------
# Copyright 2019 Scott Milano
# -------------------------------------------------
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

PROGRAM =
ARCHIVE	= libhash.a
HEADER	= hash.h dbg.h

# Optional executables
DOXYGEN ?= $(shell command -v /usr/local/bin/doxygen;)
ifeq (,$(DOXYGEN))
DOXYGEN = $(shell command -v doxygen;)
endif
CTAGS   ?= $(shell command -v ctags;)
CSCOPE  ?= $(shell command -v cscope;)
VALGRIND ?= $(shell command -v valgrind;)

# Build utilities
#CROSS_COMPILE	?= arm-none-linux-gnueabi-
CROSS_COMPILE   ?=
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RM = rm -rf

# Support cross compile, this becomes the extension for cross compiles
MARCH=$(shell echo | $(CC) -v -E - 2>&1 | grep cc1 | sed -e 's/.*march=//' | cut -d\  -f1)

# Flags for archive
ARFLAGS = -rv

# Place all build output in this folder
BUILDDIR = $(abspath build)

# Select install folders based on "TOP" from parent make file
TOP ?= $(abspath ..)
INSTALL = $(TOP)
INCINSTALL = $(INSTALL)/include

# Change output/install location on cross compile
ifeq (,$(CROSS_COMPILE))
ifneq (,$(PROGRAM))
PROGRAMOUT = $(BUILDDIR)/$(PROGRAM)
endif
BININSTALL = $(INSTALL)/bin
LIBINSTALL = $(INSTALL)/lib
else
ifneq (,$(PROGRAM))
PROGRAMOUT = $(BUILDDIR)/$(MARCH)/$(PROGRAM)
endif
BININSTALL = $(INSTALL)/bin_$(MARCH)
LIBINSTALL = $(INSTALL)/lib_$(MARCH)
endif

# Set output location for archive if it exists
ifneq (,$(ARCHIVE))
ARCHIVEOUT = $(BUILDDIR)/$(MARCH)/$(ARCHIVE)
endif

CFLAGS = -I$(INCINSTALL)
LDFLAGS = -L$(LIBINSTALL)
LIBS += -lpthread
ifeq ($(MAKECMDGOALS),all)
TARGET=release
CFLAGS += -Wall -O2
LDFLAGS += -O2 -lpthread
else ifeq ($(MAKECMDGOALS),timetest)
CFLAGS += -Wall -O2
LDFLAGS += -O2 -lpthread
else
TARGET=debug
CFLAGS += -Wall -g
LDFLAGS += -lpthread -g
ifneq ($(MARCH),armv5te)
CFLAGS += -Og
LDFLAGS += -Og
endif
endif

# Build files
OBJDIR = $(BUILDDIR)/$(MARCH)/$(TARGET)
SOURCES = $(wildcard *.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))

all: $(ARCHIVEOUT) $(PROGRAMOUT) tags

debug: $(ARCHIVEOUT) $(PROGRAMOUT)


# Clean up output
clean:
	-$(RM) $(OBJECTS) $(OBJECTS:.o=.d) $(ARCHIVEOUT) $(PROGRAMOUT)
	-$(RM) tags cscope.out ../docs/ $(BUILDDIR)/

$(PROGRAMOUT): $(OBJECTS)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LOCALLIBS) $(LIBS)

$(ARCHIVEOUT): $(OBJECTS)
	@echo 'Building target: $@ from $(OBJECTS)'
	@echo 'Invoking: AR Archiver'
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) -o $@ $(OBJECTS)

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJECTS:.o=.d)
endif

$(OBJDIR)/%.o:  %.c
	@$(RM) $@
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -MMD -MP -o $@ $<

install: all
ifneq (,$(PROGRAM))
	@[ -d $(BININSTALL) ] || mkdir -p $(BININSTALL)
	cp -f $(PROGRAMOUT) $(BININSTALL)
endif
ifneq (,$(ARCHIVE))
	@[ -d $(LIBINSTALL) ] || mkdir -p $(LIBINSTALL)
	cp -f $(ARCHIVEOUT) $(LIBINSTALL)
endif
ifneq (,$(HEADER))
	@[ -d $(INCINSTALL) ] || mkdir -p $(INCINSTALL)
	cp -f $(HEADER) $(INCINSTALL)
endif

test:
ifneq (,$(wildcard test.c))
	@$(CC) -g -DUNIT_TEST $(LDFLAGS) $(LOCALLIBS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest
	@./unittest
	@$(RM) ./unittest gmon.out
endif

memtest:
ifneq (,$(wildcard test.c))
ifneq (,$(VALGRIND))
	@$(CC) -g -DUNIT_TEST -DMEM_TEST $(LDFLAGS) $(LOCALLIBS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest
	@valgrind --leak-check=full --show-leak-kinds=all ./unittest
	@$(RM) ./unittest gmon.out
endif
endif

timetest:
ifneq (,$(wildcard test.c))
	$(CC) -DTIMETEST -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t $(LDFLAGS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest && ./unittest
	$(CC) -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t $(LDFLAGS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest && ./unittest
	$(CC) -DLSEARCH -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t $(LDFLAGS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest && ./unittest
	@$(RM) ./unittest gmon.out
endif

# Optional build utilitties
tags:
ifneq (,$(CSCOPE))
	@$(CSCOPE) -Rb
else
ifneq (,$(CTAGS))
	@$(CTAGS) -R
endif
endif

docs:
ifneq (,$(DOXYGEN))
	$(DOXYGEN)
endif

.PHONY: all debug clean force test memtest timetest docs tags
