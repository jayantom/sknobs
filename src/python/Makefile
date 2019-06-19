default:
	python setup.py sdist bdist_wheel

# This maintains the legacy sknobs-global install target.
# You may want to skip this and use "pip install" to directly install the
# python library in to your system, user, or virtualenv python installation
# area.
# This is a non-universal wheel due to the C extension.  If you need to support
# both python 2 and 3, run twice in separate python2 and python3 environments.
install:
	pip install dist/*.whl --target $(PREFIX)/$(ARCH)/lib/python
	SKNOBS_HOME=$(PREFIX) ./testsknobs

# clean up
clean:
	$(RM) *~ *.pyc *.so
	$(RM) -r build dist
	$(RM) -r c
	$(RM) -r sknobs.egg-info

