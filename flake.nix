{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:
    let
      forAllSystems = function:
        nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed
          (system: function nixpkgs.legacyPackages.${system});
    in
    {
      formatter = forAllSystems (pkgs: pkgs.nixfmt-rfc-style);

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.system}.velma ];
          hardeningDisable = [ "all" ];
          packages = [ pkgs.bear ];

          env.NIX_CFLAGS_COMPILE = "-isystem ${pkgs.python311}/include/python3.11";
        };
      });

      packages = forAllSystems (pkgs:
        let
          pypkgs = pkgs.python311.pkgs;
        in
        {
          default = self.packages.${pkgs.system}.velma;
          velma = pypkgs.callPackage ./velma.nix { };
        });
    };
}
