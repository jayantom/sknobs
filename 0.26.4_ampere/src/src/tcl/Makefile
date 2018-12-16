SWIG = swig
DEBUG = -g
CFLAGS = -fPIC -I../c  -I/tools/FOSS/tcl/8.6.8/x86_64/include

default: sknobs.so
	#./testsknobs +abc=5 +xyz=10

# python interface built as swig wrapper around C interface
sknobs_wrap.c: sknobs.i
	$(SWIG) -I../c -I/tools/FOSS/tcl/8.6.8/x86_64/include -includeall -tcl $<

sknobs_wrap.o: sknobs_wrap.c

sknobs.so: sknobs_wrap.o
	gcc -shared $(CFLAGS) $^ ../c/libsknobs.a -o $@

install: sknobs.so
	install -D sknobs.so $(PREFIX)/$(ARCH)/lib/tcl/sknobs.so

# clean up
clean:
	$(RM) *.o *.a *~ *.so
	$(RM) sknobs_wrap.*

