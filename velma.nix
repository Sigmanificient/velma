{ lib
, buildPythonPackage
, setuptools
}:

buildPythonPackage {
  pname = "velma";
  version = "2.2.1";
  pyproject = true;

  src = ./.;

  build-system = [ setuptools ];
  dependencies = [ ]; # for now

  pythonImportsCheck = [ "vera" ];

  meta = with lib; {
    description = "Modern Reimplementation of vera built with C internals";
    license = licenses.mit;
    homepage = "https://github.com/Sigmanificient/velma";
    maintainers = with maintainers; [ sigmanificient ];
  };
}
