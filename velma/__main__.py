from . import __version__

import argparse
import sys

from typing import Final, Any

EXIT_SUCCESS: Final[int] = 0

CLI_ARGS: dict[tuple[str, ...] | str, dict[str, Any]] = {
    ("-p", "--profile"): dict(
        required=True,
        type=str,
        help="execute all rules from the given profile",
    ),
    ("-r", "--root"): dict(
        required=True,
        type=str,
        help="use the given directory as the vera root directory",
    ),
    "--version": dict(
        action="version", version=f"{__package__}, v{__version__}"
    ),
    "files": dict(
        nargs="*",
        help="list of files. alternatively they may be sent via stdin. ",
    ),
}


def main() -> int:
    parser = argparse.ArgumentParser()

    for flags, settings in CLI_ARGS.items():
        if isinstance(flags, str):
            flags = (flags,)

        parser.add_argument(*flags, **settings)

    args = parser.parse_args()

    profile = args.profile
    root = args.root
    files = args.files or sys.stdin.read()

    print(f"profile: {profile}, root: {root}")
    print(files)

    return EXIT_SUCCESS
