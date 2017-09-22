from distutils.core import setup, Extension
setup(name='sknobs',
      version='1.0',
      ext_modules=[Extension('sknobs', ['../c/sknobs.c', 'sknobs_ext.c'], include_dirs=['../c'])],
      )
