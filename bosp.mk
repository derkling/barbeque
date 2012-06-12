
# Targets provided by this project
.PHONY: bbque clean_bbque

ifdef CONFIG_BBQUE_BUILD_DEBUG
  BUILD_TYPE := Debug
  BBQUE_CMAKE_OPTS += " -DBUILD_TYPE=Debug "
else
  BUILD_TYPE := Release
  BBQUE_CMAKE_OPTS += " -DBUILD_TYPE=Release "
endif

bbque: external
	@echo
	@echo "==== Building Barbeque RTRM ($(BUILD_TYPE)) ===="
	@[ -d barbeque/build/$(BUILD_TYPE) ] || \
		mkdir -p barbeque/build/$(BUILD_TYPE) || \
		exit 1
	@echo $(CMAKE_COMMON_OPTIONS)
	@cd barbeque/build/$(BUILD_TYPE) && \
		CXX=$(CXX) CFLAGS="--sysroot=$(PLATFORM_SYSROOT)" \
		cmake $(BBQUE_CMAKE_OPTS) $(CMAKE_COMMON_OPTIONS) || \
		exit 1
	@cd barbeque/build/$(BUILD_TYPE) && make -j$(CPUS) install || \
		exit 1

clean_bbque:
	@echo
	@echo "==== Clean-up Barbeque RTRM ($(BUILD_TYPE)) ===="
	@[ ! -f $(BUILD_DIR)/sbin/barbeque ] || \
		rm -f $(BUILD_DIR)/sbin/barbeque
	@rm -rf barbeque/build
	@rm -rf $(BUILD_DIR)/lib/bbque/*
	@echo

