# typed: strict
# enable-packager: true

module Consumer
  ::RootQualifiedPackage::Thing # error: is not imported

  # A registry-only namespace must not count as resolving when it is the complete reference.
  UnmistakablePackageCursorRoot
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `UnmistakablePackageCursorRoot`

  # A package declaration does not define the corresponding runtime namespace.
  UnmistakablePackageCursorRoot::Nested::EmptyPackage
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `UnmistakablePackageCursorRoot`

  # Neither the root nor `Nested` exists as a runtime namespace, even though they exist in the package registry.
  UnmistakablePackageCursorRoot::Nested::EmptyPackage::Missing
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `UnmistakablePackageCursorRoot`
end
