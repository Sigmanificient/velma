.PHONY: help # Show this help message
help:
	@ cat $(firstword $(MAKEFILE_LIST)) \
		| grep -E "^.PHONY:" | cut -d ' ' -f 2,3- | sed 's/#/     		/g'

VENV_DIR = venv
PY = python

VBIN ?= $(VENV_DIR)/bin

.PHONY: build # build wheel
build: dist

$(VENV_DIR):
	$(PY) -m venv $(VENV_DIR)
	$(VBIN)/pip install .

dist: $(VENV_DIR)
	$(VBIN)/python -m pip install --upgrade -r dev.requirements.txt
	$(VBIN)/python -m build

.PHONY: setup # creates a virtual env, the install velma into it
setup: $(VENV_DIR)

.PHONY: resetup # remove virtual env, then setup
.NOTPARALLEL: resetup
resetup: mrproper setup

.PHONY: compiledb # create compile_commands.json for IDE support
compiledb:
	$(MAKE) -f standalone_shared_object.mk compiledb

BUILD_DIRS := build dist
BUILD_DIRS += velma.egg-info
BUILD_DIRS += result

# cleanin

.PHONY: clean # Remove all intermediate files, build dirs
clean:
	@ $(RM) -r $(BUILD_DIRS)

.PHONY: fclean # Remove compile files, and clean
fclean: clean
	@ $(RM) $(wildcard vera.cpython-*.so)

.PHONY: mrproper # Remove virtual env, and fclean
mrproper: fclean
	@ $(RM) -r $(VENV_DIR)

# pypi upload

VELMA_TAR = $(wildcard dist/velma-*.tar.gz)

.PHONY: pypi_test # Upload package to test.pypi.org
pypi_test: dist
	$(VBIN)/python -m twine upload --repository testpypi $(VELMA_TAR)

.PHONY: test_pip # attempt install from test.pypi.org, reset venv
test_pip: mrproper $(VENV_DIR)
	$(VBIN)/python -m pip install --index-url https://test.pypi.org/simple --no-deps velma

.PHONY: upload # Upload to pypi.org, warning: it the real thing!
upload: dist
	$(VBIN)/python -m twine check $(VELMA_TAR)
	$(VBIN)/python -m twine upload $(VELMA_TAR)

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
