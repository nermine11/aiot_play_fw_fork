.PHONY: all list-projects

SEGGER_DIR ?= /opt/segger
BUILD_CONFIG ?= Debug
SEGGER_PROJECT ?= aiot_play_fw.emProject

PROJECTS ?= 02drv_triangle

.PHONY: $(PROJECTS)

all: $(PROJECTS)

$(PROJECTS):
	@echo "\e[1mBuilding project $@\e[0m"
	"$(SEGGER_DIR)/bin/emBuild" $(SEGGER_PROJECT) -project $@ -config $(BUILD_CONFIG) -rebuild -verbose
	@echo "\e[1mDone\e[0m\n"

list-projects:
	@echo "\e[1mAvailable projects:\e[0m"
	@echo $(PROJECTS) | tr ' ' '\n'

clean:
	"$(SEGGER_DIR)/bin/emBuild" $(SEGGER_PROJECT) -config $(BUILD_CONFIG) -clean

