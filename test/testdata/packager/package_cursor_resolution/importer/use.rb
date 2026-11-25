# typed: strict
# enable-packager: true

module EmptyPackageImporter
  # Importing an empty package still does not define its runtime namespace.
  UnmistakablePackageCursorRoot::Nested::EmptyPackage
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `EmptyPackage`
end
