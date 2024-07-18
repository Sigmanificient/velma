from setuptools import setup, Extension

module = Extension(
    'vera',
    sources=['vera.c', 'token_utils.c']
)

setup(
    ext_modules=[module]
)
