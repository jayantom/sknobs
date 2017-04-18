ARCH = $(shell uname -m)
PREFIX = $(CURDIR)/..
export PREFIX
export ARCH

clean: 
	$(MAKE) -C c clean
	$(MAKE) -C tcl clean
	#$(MAKE) -C python clean
	#$(MAKE) -C verilog clean
	#$(MAKE) -C perl clean

build: 
	echo $(ARCH)
	echo $(CURDIR)
	$(MAKE) -C c
	$(MAKE) -C tcl
	#$(MAKE) -C python
	#$(MAKE) -C verilog
	#$(MAKE) -C perl

installarch: 
	$(MAKE) -C c install
	$(MAKE) -C tcl install
	#$(MAKE) -C python install CFLAGS=
	#$(MAKE) -C verilog install
	#$(MAKE) -C perl install

install:
	$(MAKE) CFLAGS="-I../c -m64 -fPIC" ARCH=x86_64 INSTALL_LIB=lib64 clean build installarch
