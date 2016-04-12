#
# Copyright 2014, NICTA
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(NICTA_BSD)
#

lib-dirs:=libs

all: custom

-include .config
include tools/common/project.mk

custom: custom_server common
	@echo "[STAGE] ${IMAGE_ROOT}/custom-image"
	cp -f ${STAGE_BASE}/bin/custom_server ${IMAGE_ROOT}/custom-image

simulate-ia32:
	qemu-system-i386 \
		-m 512 -nographic -kernel images/kernel-ia32-pc99 \
		-initrd images/custom-image

# Help
.PHONY: help
help:
	@echo "RefOS - Reference multi-server OS on seL4."
	@echo ""
	@echo " make help            - Show this help test."
	@echo " make menuconfig      - Select build configuration via menus."
	@echo " make silentoldconfig - Update configuration with the defaults of any"
	@echo "                        newly introduced settings."
	@echo " make                 - Build everything with the current configuration."
	@echo " make custom          - Build without re-generating RPC stub code."
	@echo ""
