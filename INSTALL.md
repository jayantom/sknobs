
On an x86_64 machine:
make install PREFIX=../../

On an arm64 machine: UNIMPLEMENTED

# On x86_32 machine: UNSUPPORTED

Notes: 
Binary files are automatically placed in PREFIX/`uname -m`/. Everything is installed "in place" as siblings of src,
i.e., create "include", "lib", "<mach>/lib", etc..

If you wish particular languages not to be built, add one of more of the following to the make command:
* `SKNOBS_C=0`
* `SKNOBS_PERL=0`
* `SKNOBS_PYTHON=0`
* `SKNOBS_TCL=0`
* `SKNOBS_VERILOG=0`

System Package Dependencies (RPM package names)
* perl-devel
* python3
* python3-devel
* python3-setuptools

Python Package Dependencies.  It is expected that you are running in an
appropriate virtal environment for the version of python for which you wish to
build.
* pip install wheel
