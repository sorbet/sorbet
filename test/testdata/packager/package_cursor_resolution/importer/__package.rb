# typed: strict
# enable-packager: true

class EmptyPackageImporter < PackageSpec
  import UnmistakablePackageCursorRoot::Nested::EmptyPackage
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ usage: cursor-namespace
end
