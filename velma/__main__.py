from . import __version__

from typing import Final

EXIT_SUCCESS: Final[int] = 0


def main() -> int:
    print(f"{__package__}, v{__version__}")
    return EXIT_SUCCESS
