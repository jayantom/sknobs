ARCH = $(shell uname -m)
PREFIX = $(CURDIR)/..
export PREFIX
export ARCH

default: 
	echo $(ARCH)
	echo $(CURDIR)
	$(MAKE) -C c
	$(MAKE) -C tcl
	$(MAKE) -C python
	$(MAKE) -C verilog
	$(MAKE) -C perl

clean: 
	$(MAKE) -C c clean
	$(MAKE) -C tcl clean
	$(MAKE) -C python clean
	$(MAKE) -C verilog clean
	$(MAKE) -C perl clean

install:
	$(MAKE) install32
	$(MAKE) install64

install32:
	$(MAKE) CFLAGS="-I../c -m32 -fPIC" ARCH=i686 clean default installarch

install64:
	$(MAKE) CFLAGS="-I../c -m64 -fPIC" ARCH=x86_64 INSTALL_LIB=lib64 clean default installarch

defaultc:
	echo $(ARCH)
	echo $(CURDIR)
	$(MAKE) -C c

install64c:
	$(MAKE) CFLAGS="-I../c -m64 -fPIC" ARCH=x86_64 INSTALL_LIB=lib64 clean defaultc installarch


installarch: 
	$(MAKE) -C c install
	$(MAKE) -C tcl install
	$(MAKE) -C python install CFLAGS=
	$(MAKE) -C verilog install
	$(MAKE) -C perl install
