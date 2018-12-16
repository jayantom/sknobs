default:
	python3 setup.py sdist

install:
	python3 setup.py install --home $(PREFIX)/$(ARCH) \
                             --install-lib=$(PREFIX)/$(ARCH)/lib/python3
	SKNOBS_HOME=$(PREFIX) ./testsknobs

# clean up
clean:
	$(RM) MANIFEST
	$(RM) *~ *.pyc *.so
	$(RM) -r build dist
	$(RM) -r c

