default:
	python setup.py sdist

install:
	python setup.py install --home $(PREFIX)/$(ARCH) \
                            --install-lib=$(PREFIX)/$(ARCH)/lib/python

	SKNOBS_HOME=$(PREFIX) ./testsknobs

# clean up
clean:
	$(RM) MANIFEST
	$(RM) *~ *.pyc *.so
	$(RM) -r build dist
	$(RM) -r c

