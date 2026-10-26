{
  description = "Dev environment for building AromaCrashHandlerModule (Wii U Aroma module)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    devkitNix = {
      url = "github:bandithedoge/devkitNix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      devkitNix,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };

        devkitPPC = devkitNix.packages.${system}.devkitPPC;

        wums = pkgs.stdenvNoCC.mkDerivation {
          name = "wiiumodulesystem-20260418";
          src = pkgs.dockerTools.pullImage (
            builtins.fromJSON (builtins.readFile ./nix/wums.json)
          );
          phases = [ "buildPhase" ];
          buildPhase = ''
            tar -xf $src
            for archive in $(find . -maxdepth 1 -name '*.tar'); do
              tar -xf "$archive"
            done
            mkdir -p $out
            cp -r artifacts/wums $out/wums

          '';
        };

        devkitproRoot = pkgs.runCommand "devkitpro-root" { } ''
          mkdir -p $out
          for entry in ${devkitPPC}/opt/devkitpro/*; do
            ln -s "$entry" "$out/$(basename "$entry")"
          done
          ln -s ${wums}/wums $out/wums
        '';

        clangFormat = pkgs.clang-tools;


      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            devkitPPC
            pkgs.gcc
            pkgs.gnumake
            pkgs.python3
            (pkgs.python3.withPackages (p: [ p.pillow ]))
            pkgs.dejavu_fonts
            clangFormat
          ];

          env = {
            DEVKITPRO = "${devkitproRoot}";
            DEVKITPPC = "${devkitproRoot}/devkitPPC";
            WUT_ROOT = "${devkitproRoot}/wut";
            WUMS_ROOT = "${devkitproRoot}/wums";
            DEJAVU_MONO_PATH = "${pkgs.dejavu_fonts}/share/fonts/truetype/DejaVuSansMono.ttf";
          };

          shellHook = ''
            export PATH="$DEVKITPPC/bin:$DEVKITPRO/tools/bin:$PATH"
          '';
        };
      }
    );
}
