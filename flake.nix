{
  description = "ghc-eventlog-socket";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/master";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachSystem flake-utils.lib.allSystems (system: let
      pkgs = import nixpkgs {
        inherit system;
      };
      hsPkgs = pkgs.haskell.packages.ghc944;
      hsenv = hsPkgs.ghcWithPackages (p: [ p.random ]);
      shell = pkgs.mkShell {
        buildInputs = [
          hsenv
          pkgs.alejandra # nix expression formatter
          pkgs.cabal-install
          pkgs.zlib
        ];
      };
    in {
      devShells.default = shell;
    });
}
