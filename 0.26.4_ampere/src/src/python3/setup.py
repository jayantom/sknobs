from distutils.core import setup, Extension
setup(name='sknobs',
      version='1.0',
      author='Warren Stapleton',
      author_email='warren@sknobs.sourceforce.com',
      url='sknobs.sourceforge.com',
      ext_modules=[Extension('sknobs', ['../c/sknobs.c', 'sknobs_ext.c'], include_dirs=['../c', '/projects/svdc/veloce/tools/python/3.6.5/x86_64/include/python3.6.5'], library_dirs=['/projects/svdc/veloce/tools/python/3.6.5/x86_64/lib'])]
      )
