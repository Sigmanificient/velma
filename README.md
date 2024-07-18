# velma

Velma is the reimplementation of [vera](https://github.com/verateam/vera)
 python-API built with C internals.

## main differences

- Velma suppports python-based plugins while vera support both lua and python.
- It provides a python package built with setuptools and C extension rather than relying on `<boost/python.hpp>`. This avoid relying on the system python environment, and provide support for virtual env.
- Tokenization is done with C in mind rather than C++.

## Installation

Start by [cloning this repository](https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository).

### pip based

> [!NOTE]
> It is recommended to use python virtual env's rather than doing a system-wide install.
> See [Creation of virtual environments](https://docs.python.org/3/library/venv.html) for more information

```bash
# (in repository's root)
pip install .
```

### Using nix

The repository flake provides `velma` nix derivation (`buildPythonPackage`).

```
nix build github:Sigmanificient/velma
```

or creating a python env:

```nix
let
  # assuming `velma-flake.url = "github:Sigmanificient/velma"` in flake inputs
  velma = velma-flake.outputs.${system}.velma;
  selectPythonPackages = ps: [ velma ... ];
in 
(pkgs.python311.withPackages (ps: selectPythonPackages)
```

## Development

To see available commands in the Makefile, use the following:

```bash
make help
```

### Building a standalone shared object

If you need to build a standlone `shared object`:

```bash
make -f standalone_shared_object.mk
```

### IDE support (via compile_commands.json)

```bash
make compiledb
```
