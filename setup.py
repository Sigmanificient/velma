import os
from typing import Iterator

from setuptools import Extension, setup


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


vera = Extension(
    "velma",
    sources=list(find_sources("vera")),
    include_dirs=["vera"],
    language="c",
)

if __name__ == "__main__":
    setup(ext_modules=[vera])
