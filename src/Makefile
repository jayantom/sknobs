ARCH = $(shell uname -m)
PREFIX = $(CURDIR)/..
export PREFIX
export ARCH
# Define language targets on command line.
# E.g., make install64 SKNOBS_TCL=0 SKNOBS_PYTHON=0
export SKNOBS_C = 1
export SKNOBS_PERL = 1
export SKNOBS_PYTHON = 1
export SKNOBS_TCL = 1
export SKNOBS_VERILOG = 1

clean: 
	if [ "$(SKNOBS_C)"       -eq "1" ]; then $(MAKE) -C c clean; fi
	if [ "$(SKNOBS_TCL)"     -eq "1" ]; then $(MAKE) -C tcl clean; fi
	if [ "$(SKNOBS_PYTHON)"  -eq "1" ]; then $(MAKE) -C python clean; fi
	if [ "$(SKNOBS_VERILOG)" -eq "1" ]; then $(MAKE) -C verilog clean; fi
	if [ "$(SKNOBS_PERL)"    -eq "1" ]; then $(MAKE) -C perl clean; fi

build: 
	@echo ARCH=$(ARCH)
	@echo CURDIR=$(CURDIR)
	if [ "$(SKNOBS_C)"       -eq "1" ]; then $(MAKE) -C c; fi
	if [ "$(SKNOBS_TCL)"     -eq "1" ]; then $(MAKE) -C tcl; fi
	if [ "$(SKNOBS_PYTHON)"  -eq "1" ]; then $(MAKE) -C python; fi
	if [ "$(SKNOBS_VERILOG)" -eq "1" ]; then $(MAKE) -C verilog; fi
	if [ "$(SKNOBS_PERL)"    -eq "1" ]; then $(MAKE) -C perl; fi

installarch: 
	if [ "$(SKNOBS_C)"       -eq "1" ]; then $(MAKE) -C c install; fi
	if [ "$(SKNOBS_TCL)"     -eq "1" ]; then $(MAKE) -C tcl install; fi
	if [ "$(SKNOBS_PYTHON)"  -eq "1" ]; then $(MAKE) -C python install CFLAGS=; fi
	if [ "$(SKNOBS_VERILOG)" -eq "1" ]; then $(MAKE) -C verilog install; fi
	if [ "$(SKNOBS_PERL)"    -eq "1" ]; then $(MAKE) -C perl install; fi

install:
	$(MAKE) CFLAGS="-I../c -m64 -fPIC" ARCH=x86_64 INSTALL_LIB=lib64 clean build installarch
