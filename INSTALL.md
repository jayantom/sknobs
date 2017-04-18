# Binary files automatically placed in PREFIX/`uname -m`/
cd src
# Place in everything "in place" as siblings of src (i.e., create "include",
# "lib", "<mach>/lib", etc.).
# On x86_64 machine:
make install64 PREFIX=../..
# On x86_32 machine:
make install32 PREFIX=../..
