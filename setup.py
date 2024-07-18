import os

from setuptools import setup, Extension
from typing import Iterator


def find_sources(basepath: str) -> Iterator[str]:
    files = (
        (current_dir, file)
        for current_dir, _, files in os.walk(basepath)
        for file in files
    )

    for current_dir, file in files:
        *_, ext = os.path.splitext(file)
        if ext != f"{os.path.extsep}c":
            continue

        fullpath = os.path.join(current_dir, file)
        print(f"Registering ${fullpath} to sources")
        yield fullpath


module = Extension(
    'vera',
    sources=list(find_sources("src/velma")),
    include_dirs=["velma"],
    language="c"
)

setup(
    ext_modules=[module]
)
