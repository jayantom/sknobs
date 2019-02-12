import setuptools

extension = setuptools.Extension(
    name='sknobs',
    sources=['../c/sknobs.c', 'sknobs_ext.c'],
    include_dirs=['../c'],
    language='c',
    )

# See also setup.cfg for additional configuration.
setuptools.setup(
    ext_modules=[extension],
    )
