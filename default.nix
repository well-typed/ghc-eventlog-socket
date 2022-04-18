{ compiler ? "ghc8107" }:

let
  sources = import ./nix/sources.nix;
  pkgs = import sources.nixpkgs { };

  gitignore = pkgs.nix-gitignore.gitignoreSourcePure [ ./.gitignore ];

  myHaskellPackages = pkgs.haskell.packages.${compiler}.override {
    overrides = hself: hsuper: {
      "eventlog-socket" = hself.callCabal2nix "eventlog-socket" (gitignore ./.) { };
    };
  };

  shell = myHaskellPackages.shellFor {
    packages = p: [ p."eventlog-socket" ];
    buildInputs = [
      myHaskellPackages.haskell-language-server
      pkgs.haskellPackages.cabal-install
      pkgs.haskellPackages.ghcid
      pkgs.haskellPackages.ormolu
      pkgs.haskellPackages.hlint
      pkgs.haskellPackages.hasktags
      pkgs.niv
      pkgs.nixpkgs-fmt
      pkgs.pkgconfig
      pkgs.zlib
      pkgs.netcat
      pkgs.shellcheck
    ];
    withHoogle = true;
  };

  exe = pkgs.haskell.lib.justStaticExecutables (myHaskellPackages."eventlog-socket");

  docker = pkgs.dockerTools.buildImage {
    name = "eventlog-socket";
    config.Cmd = [ "${exe}/bin/eventlog-socket" ];
  };
in {
  inherit shell;
  inherit exe;
  inherit docker;
  inherit myHaskellPackages;
  "eventlog-socket" = myHaskellPackages."eventlog-socket";
}
