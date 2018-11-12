{ pkgs ? import <nixpkgs> {}, lib ? pkgs.lib }:

pkgs.stdenv.mkDerivation rec {
  name = "ip2unix-${version}";

  version = let
    regex = " *project *\\([^)]*[ ,]+version *: *'([^']*)'.*";
    contents = builtins.readFile ./meson.build;
  in builtins.head (builtins.match regex contents);

  src = lib.cleanSource ./.;

  nativeBuildInputs = [
    pkgs.meson pkgs.ninja pkgs.pkgconfig pkgs.asciidoc pkgs.libxslt.bin
    pkgs.docbook_xml_dtd_45 pkgs.docbook_xsl pkgs.libxml2.bin pkgs.docbook5
    pkgs.python3Packages.pytest pkgs.python3Packages.pytest-timeout
  ];
  buildInputs = [ pkgs.libyamlcpp pkgs.systemd ];

  doCheck = true;

  doInstallCheck = true;
  installCheckPhase = ''
    found=0
    for man in "$out/share/man/man1"/ip2unix.1*; do
      test -s "$man" && found=1
    done
    if [ $found -ne 1 ]; then
      echo "ERROR: Manual page hasn't been generated." >&2
      exit 1
    fi
  '';
}
