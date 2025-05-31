import argparse
import importlib.util
import os
import pathlib
import re
import sys
import traceback

from typing import Any, Sequence

import vera

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
    ("-d", "--no-duplicate"): dict(
        action="store_true",
        help=(
            "do not duplicate messages if a single rule is violated many times"
            " in a single line of code"
        ),
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
    inval: set[str] = set()

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


def exec_from_abspath(script: str) -> bool:
    script_dir = os.path.dirname(os.path.abspath(script))
    sys.path.insert(0, script_dir)

    try:
        spec = importlib.util.spec_from_file_location("__main__", script)
        if spec is None or spec.loader is None:
            return False

        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return True

    except Exception:
        traceback.print_exc()
        return False

    finally:
        sys.path.pop(0)


def main() -> int:
    args = parse_args()
    root = os.path.abspath(args.root)

    scripts, err = load_profile(root, args.profile)
    if err or not scripts:
        return err

    root = args.root
    files: list[str] = args.files or sys.stdin.read().splitlines()

    vera._register_sources(files)
    vera._set_duplication_policy(args.no_duplicate)
    for script in scripts:
        exec_from_abspath(script)

    return os.EX_OK


if __name__ == "__main__":
    sys.exit(main())
