{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    devkitNix.url = "github:bandithedoge/devkitNix";
  };

  outputs = { self, nixpkgs, devkitNix }: let
    pkgs = import nixpkgs { system = "x86_64-linux"; overlays = [ devkitNix.overlays.default ]; };
  in {
    devShells.x86_64-linux.default = pkgs.mkShell.overide { stdenv = pkgs.devkitNix.stdenvARM; } {};

    packages.x86_64-linux = rec {
      faketik = pkgs.devkitNix.stdenvARM.mkDerivation rec {
        pname = "faketik";
        version = "1.1.2-unstable";
        src = builtins.path { path = ./.; name = pname; };

        makeFlags = [ "TARGET=${pname}" ];

        installPhase = ''
          mkdir $out
          cp ${pname}.3dsx $out
        '';
      };
      default = faketik;
    };
  };
}
