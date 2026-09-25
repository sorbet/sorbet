# typed: strict
# enable-packager: true
# enable-package-directed: true

class A < PackageSpec
  import B

  export B::Thing
# ^^^^^^^^^^^^^^^ error: Cannot export `B::Thing` because it is owned by another package
# ^^^^^^^^^^^^^^^ error: Cannot export `B::Thing` because it is owned by another package
end
