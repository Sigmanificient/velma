.PHONY: help # Show this help message
help:
	@ cat $(firstword $(MAKEFILE_LIST)) \
		| grep -E "^.PHONY:"            \
		| cut -d ' ' -f 2,3-            \
		| sed 's/#/     		/g'

.PHONY: compiledb # create compile_commands.json for IDE support
compiledb:
	$(MAKE) -f standalone_shared_object.mk compiledb

BUILD_DIRS := build dist
BUILD_DIRS += velma.egg-info
BUILD_DIRS += result

VENV_DIR = venv

.PHONY: clean # Remove all intermediate files, build dirs
clean:
	@ $(RM) -r $(BUILD_DIRS)

.PHONY: fclean # Remove compile files, and clean
fclean: clean
	@ $(RM) $(wildcard vera.cpython-*.so)

.PHONY: mrproper # Remove virtual env, and fclean
mrproper: fclean
	@ $(RM) -r $(VENV_DIR)

# logging utilities

ifneq ($(shell command -v tput),)
  ifneq ($(shell tput colors),0)

mk-color = \e[$(strip $1)m

C_BEGIN := \033[A
C_RESET := $(call mk-color, 00)

C_RED := $(call mk-color, 31)
C_GREEN := $(call mk-color, 32)
C_YELLOW := $(call mk-color, 33)
C_BLUE := $(call mk-color, 34)
C_PURPLE := $(call mk-color, 35)
C_CYAN := $(call mk-color, 36)

  endif
endif

NOW = $(shell date +%s%3N)

STIME := $(call NOW)
TIME_NS = $(shell expr `date +%s%3N` - $(STIME))
TIME_MS = $(shell expr $(call TIME_NS))

BOXIFY = "[$(C_BLUE)$(1)$(C_RESET)] $(2)"

ifneq ($(shell command -v printf),)
  LOG_TIME = printf $(call BOXIFY, %6s , %b\n) "$(call TIME_MS)"
else
  LOG_TIME = echo -e $(call BOXIFY, $(call TIME_MS) ,)
endif
