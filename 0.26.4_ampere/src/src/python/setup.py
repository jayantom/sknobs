from distutils.core import setup, Extension
setup(name='sknobs',
      version='1.0',
      author='Warren Stapleton',
      author_email='warren@sknobs.sourceforce.com',
      url='sknobs.sourceforge.com',
      ext_modules=[Extension('sknobs', ['../c/sknobs.c', 'sknobs_ext.c'], include_dirs=['../c', '/tools/FOSS/python/2.6.4/x86_64/include/python2.6'], library_dirs=['/tools/FOSS/python/2.6.4/x86_64/lib'])],
      )
