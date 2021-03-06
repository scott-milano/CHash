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

# Find utilities
CTAGS   ?= $(shell command -v ctags;)
CSCOPE  ?= $(shell command -v cscope;)

# Find Makefiles
MKFLS += $(subst Makefile,,$(shell find */ -name Makefile ))

TOP ?= $(abspath .)
export TOP

all debug install docs test memtest timetest: tags
	@$(foreach folder,$(MKFLS), $(MAKE) -C $(folder) $@ || exit;)

clean:
	@$(foreach folder,$(MKFLS), $(MAKE) -C $(folder) $@ || exit;)
	@rm -rf tags cscope.out bin lib include

# Optional build utilitties
tags:
	@$(foreach folder,$(MKFLS), $(MAKE) -C $(folder) $@ || exit;)
ifneq (,$(CSCOPE))
	@$(CSCOPE) -Rb
else
ifneq (,$(CTAGS))
	@$(CTAGS) -R
endif
endif

.PHONY: all debug clean force test memtest timetest docs
