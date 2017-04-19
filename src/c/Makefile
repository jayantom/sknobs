CFLAGS = -fPIC -Wall -Werror
INSTALL_LIB=lib64

default: libsknobs.a libsknobs.so

test: testsknobs
	./testsknobs

sknobs: sknobs.o

sknobs.o: sknobs.c sknobs.h

libsknobs.a: sknobs.o
	ar r $@ $^

libsknobs.so: sknobs.o
	$(CC) $(CFLAGS) -shared -o $@ $<

testsknobs: testsknobs.o libsknobs.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lsknobs

install: libsknobs.a libsknobs.so
	install -D sknobs.h $(PREFIX)/include/sknobs.h
	install -D libsknobs.a $(PREFIX)/$(ARCH)/$(INSTALL_LIB)/libsknobs.a
	install -D libsknobs.so $(PREFIX)/$(ARCH)/$(INSTALL_LIB)/libsknobs.so

clean:
	$(RM) testsknobs testsknobs.saved
	$(RM) *.o *.a *.so *.exe
	$(RM) \#* *~
