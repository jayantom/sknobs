
On an x86_64 machine:
make install PREFIX=../../

On an arm64 machine: UNIMPLEMENTED

# On x86_32 machine: UNSUPPORTED

Notes: 
Binary files are automatically placed in PREFIX/`uname -m`/. Everything is installed "in place" as siblings of src,
i.e., create "include", "lib", "<mach>/lib", etc..

