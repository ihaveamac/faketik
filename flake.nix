{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    devkitNix.url = "github:bandithedoge/devkitNix";
  };

  outputs = { self, nixpkgs, devkitNix, hax-nur }: {
    devShells.x86_64-linux.default = let
      pkgs = import nixpkgs { system = "x86_64-linux"; overlays = [ devkitNix.overlays.default ]; };
    in pkgs.mkShell {
      packages = with pkgs; [
        pkgs.devkitNix.devkitARM
      ];

      inherit (pkgs.devkitNix.devkitARM) shellHook;
    };

    packages.x86_64-linux = let
      pkgs = import nixpkgs { system = "x86_64-linux"; overlays = [ devkitNix.overlays.default ]; };
    in rec {
      faketik = pkgs.stdenvNoCC.mkDerivation rec {
        pname = "faketik";
        version = "0.3.0-unstable";
        src = builtins.path { path = ./.; name = pname; };

        preBuild = pkgs.devkitNix.devkitARM.shellHook;

        makeFlags = [ "TARGET=${pname}" ];

        installPhase = ''
          cp ${pname}.3dsx $out
        '';
      };
      default = faketik;
    };
  };
}
