import argparse
import re
import os
import pathlib
import subprocess
import sys

from typing import Any, Sequence

from . import __version__

CLI_ARGS: dict[Sequence[str], dict[str, Any]] = {
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


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()

    for flags, settings in CLI_ARGS.items():
        if isinstance(flags, str):
            flags = (flags,)

        parser.add_argument(*flags, **settings)

    return parser.parse_args()


def check_missing_subdirs(root_dir: str) -> tuple[str, ...]:
    root_subdirs = [
        entry
        for entry in os.listdir(root_dir)
        if os.path.isdir(os.path.join(root_dir, entry))
    ]

    return tuple(
        subdir
        for subdir in ("profiles", "rules")
        if subdir not in root_subdirs
    )


def load_profile(root: str, profile_name: str) -> tuple[set[str], int]:
    inval = set()

    if missing_dirs := check_missing_subdirs(root):
        print("Invalid root. Missing subfolder(s):", ", ".join(missing_dirs))
        return inval, os.EX_USAGE

    profile_path = os.path.join(root, "profiles", profile_name)
    profile = os.path.isfile(profile_path)
    if not profile:
        print(f"Profile description: '{profile_name}' do not exists")
        return inval, os.EX_IOERR

    content = pathlib.Path(profile_path).read_text()

    # emulating TCL because I dont want to parse it :<
    matched = re.search(r"set\s+rules\s+{((?:\s*[\w-]+\s*)*)}", content)
    if matched is None:
        print("missing ruleset list in profile:", profile_name)
        return inval, os.EX_USAGE

    ruleset = set(re.findall(r"[\w-]+", matched.groups()[0]))
    abs_ruleset = set()

    for rule in ruleset:
        script_filename = os.path.extsep.join((rule, "py"))
        script_path = os.path.join(root, "rules", script_filename)

        if not os.path.isfile(script_path):
            print("cannot open script:", rule)
            return inval, os.EX_IOERR

        abs_ruleset.add(script_path)
    return abs_ruleset, os.EX_OK


def main() -> int:
    args = parse_args()
    root = os.path.abspath(args.root)

    scripts, err = load_profile(root, args.profile)
    if err or not scripts:
        return err

    root = args.root
    files = args.files or sys.stdin.read()

    for script in scripts:
        subprocess.run((sys.executable, script), input=files.encode())
    return os.EX_OK


if __name__ == "__main__":
    sys.exit(main())
