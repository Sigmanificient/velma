from setuptools import setup, Extension

module = Extension(
    'vera',
    sources=['vera.c', 'token_utils.c']
)

setup(
    name='vera',
    version='1.0',
    description='Banana vera++',
    ext_modules=[module]
)
