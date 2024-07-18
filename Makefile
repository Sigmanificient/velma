.PHONY: help # Show this help message
help:
	@ cat $(firstword $(MAKEFILE_LIST)) \
		| grep -E "^.PHONY:"            \
		| cut -d ' ' -f 2,3-            \
		| sed 's/#/     		/g'

.PHONY: compiledb # create compile_commands.json for IDE support
compiledb:
	$(MAKE) -f standalone_shared_object.mk compiledb
