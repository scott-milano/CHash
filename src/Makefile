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
HEADER	= hash.h

# Optional executables
DOXYGEN ?= $(shell command -v /usr/local/bin/doxygen;)
DOXYGEN ?= $(shell command -v doxygen;)
CTAGS   ?= $(shell command -v ctags;)
CSCOPE  ?= $(shell command -v cscope;)
VALGRIND ?= $(shell command -v valgrind;)

# Build utilities
#CROSS_COMPILE	?= arm-none-linux-gnueabi-
CROSS_COMPILE   ?=
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RM = rm -rf

# Flags for archive
ARFLAGS = -rv

OBJDIR = obj
INCLUDE = ../include
INSTALL = ../lib
SOURCES = $(wildcard *.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))
LIBS += -lz -lpthread
ifeq ($(MAKECMDGOALS),debug)
CFLAGS += -Wall -g -Og
LDFLAGS += -g -pg -Og
else
CFLAGS += -Wall -O2
LDFLAGS += -O2
endif

all: $(ARCHIVE) $(PROGRAM) tags

debug: $(ARCHIVE) $(PROGRAM)


# Clean up output
clean:
	$(RM) $(OBJECTS) $(OBJECTS:.o=.d) $(ARCHIVE) $(PROGRAM) tags cscope.out
	rmdir $(OBJDIR)

$(PROGRAM): $(OBJECTS)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LOCALLIBS) $(LIBS)

$(ARCHIVE): $(OBJECTS)
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
	@[ -d $(INSTALL) ] || mkdir -p $(INSTALL)
	cp -f $(ARCHIVE) $(INSTALL)
	@[ -d $(INCLUDE) ] || mkdir -p $(INCLUDE)
	cp -f $(HEADER) $(INCLUDE)

test:
ifneq (,$(wildcard test.c))
	@$(CC) -g -DUNIT_TEST $(LDFLAGS) $(LOCALLIBS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest
	@./unittest
	@$(RM) ./unittest gmon.out

memtest:
ifneq (,$(VALGRIND))
	@$(CC) -g -DUNIT_TEST $(LDFLAGS) $(LOCALLIBS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest
	@./unittest && ./unittest
	@valgrind --leak-check=full --show-leak-kinds=all ./unittest
	@$(RM) ./unittest gmon.out
endif

timetest:
	$(CC) -g -Og -DTIMETEST -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t $(LDFLAGS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest && ./unittest
	$(CC) -g -Og -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t $(LDFLAGS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest && ./unittest
	$(CC) -g -Og -DLSEARCH -DUNIT_TEST -DMAXSIZE=5000 -DTK1=int64_t -DTV1=int64_t $(LDFLAGS) $(SOURCES) $(CFLAGS) $(LIBS) -o unittest && ./unittest
endif

# Optional build utilitties
tags: $(shell find . -name \*.c ) $(shell find . -name \*.h )
ifneq (,$(CSCOPE))
	@$(CSCOPE) -Rb
else
ifneq (,$(CTAGS))
	@$(CTAGS) -R
endif
endif

docs: $(shell find . -name \*.c ) $(shell find . -name \*.h )
ifneq (,$(DOXYGEN))
	@$(DOXYGEN)
endif

.PHONY: all debug clean force test memtest timetest
