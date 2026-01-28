# frozen_string_literal: true
# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
end

class SecondPackage < PackageSpec; end # error: Invalid expression in package: `ClassDef` not allowed
#                     ^^^^^^^^^^^ error: Superclasses may only be set on constants in the package that owns them
#                     ^^^^^^^^^^^ error: Unable to resolve constant `PackageSpec`
