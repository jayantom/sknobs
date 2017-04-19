SWIG = swig
DEBUG = -g
CFLAGS = -fPIC -I../c 
TCL_CFLAGS = -I/usr/include/tcl
INSTALL_LIB=lib64

default: sknobs.so

# TCL interface built as swig wrapper around C interface
sknobs_wrap.c: sknobs.i
	$(SWIG) -I../c -includeall -tcl $<

sknobs_wrap.o: sknobs_wrap.c
	$(CC) $(CFLAGS) $(TCL_CFLAGS) -c -o $@ $<

sknobs.so: sknobs_wrap.o
	gcc -shared $(CFLAGS) $^ ../c/libsknobs.a -o $@

install: sknobs.so
	install -D sknobs.so $(PREFIX)/$(ARCH)/$(INSTALL_LIB}/tcl/sknobs.so

test: sknobs.so
	./testsknobs +abc=5 +xyz=10

# clean up
clean:
	$(RM) *.o *.a *~ *.so
	$(RM) sknobs_wrap.*

