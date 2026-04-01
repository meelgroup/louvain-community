{
  description = "Louvain community detection library";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      inherit (nixpkgs) lib;
      systems = lib.intersectLists lib.systems.flakeExposed lib.platforms.linux;
      forAllSystems = lib.genAttrs systems;
      nixpkgsFor = forAllSystems (system: nixpkgs.legacyPackages.${system});

      louvain-community-package =
        { stdenv, cmake }:
        stdenv.mkDerivation {
          name = "louvain-community";
          nativeBuildInputs = [ cmake ];
          src = ./.;
        };
    in
    {
      devShells = forAllSystems (
        system:
        let
          pkgs = nixpkgsFor.${system};
        in
        {
          default = pkgs.mkShell {
            packages = [ pkgs.cmake ];
          };
        }
      );

      packages = forAllSystems (
        system:
        let
          louvain-community = nixpkgsFor.${system}.callPackage louvain-community-package { };
        in
        {
          inherit louvain-community;
          default = louvain-community;
        }
      );
    };
}
