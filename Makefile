#
# Makefile for zybo_freebsd_vga
#
PROJNM=zybo_freebsd_vga
SRCDIR=$(PROJNM).srcs
SOURCES= \
	$(SRCDIR)/constrs_1/zybopins.xdc		\
	$(SRCDIR)/sources_1/bd/design_1/design_1.bd	\
	$(SRCDIR)/sources_1/bd/design_1/hdl/design_1_wrapper.v \
	$(SRCDIR)/sources_1/zybo_vid_gizmo.v

ifndef XILINX_VIVADO
$(error XILINX_VIVADO must be set to point to Xilinx tools)
endif

VIVADO=$(XILINX_VIVADO)/bin/vivado

.PHONY: default project bitstream

default: project bitstream

PROJECT_FILE=$(PROJNM)/$(PROJNM).xpr

project: $(PROJECT_FILE)

$(PROJECT_FILE): 
	$(VIVADO) -mode batch -source project.tcl

BITFILE=$(PROJNM)/$(PROJNM).runs/impl_1/design_1_wrapper.bit

bitstream: $(BITFILE)

$(BITFILE): $(SOURCES) $(PROJECT_FILE)
	echo Building $(BITFILE) from sources
	$(VIVADO) -mode batch -source $(SRCDIR)/scripts_1/bitstream.tcl \
		-tclargs $(PROJNM)

