{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs =
    { self, nixpkgs }:
    let
      forAllSystems =
        function:
        nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed (
          system: function nixpkgs.legacyPackages.${system}
        );
    in
    {
      formatter = forAllSystems (pkgs: pkgs.nixfmt-rfc-style);

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.system}.velma ];
          hardeningDisable = [ "all" ];
          packages = with pkgs; [
            bear
            black
          ];

          env.NIX_CFLAGS_COMPILE =
            let
              velma = self.packages.${pkgs.system}.velma;
              py = {
                pkg = builtins.head velma.drvAttrs.nativeBuildInputs;
                version = pkgs.lib.versions.majorMinor py.pkg.version;
              };
            in
            "-isystem ${py.pkg}/include/python${py.version}";
        };

        vera-clang = pkgs.mkShell {
          packages = [
            (self.packages.${pkgs.system}.velma.overrideAttrs (prev: {
              propagatedBuildInputs = (prev.dependencies or [])
                ++ [ pkgs.python3Packages.libclang ];
              # required for banana-coding style checker
            }))
          ];
        };
      });

      packages = forAllSystems (
        pkgs:
        let
          pypkgs = pkgs.python312.pkgs;
        in
        {
          default = self.packages.${pkgs.system}.velma;
          velma = pypkgs.callPackage ./velma.nix { };
        }
      );
    };
}
