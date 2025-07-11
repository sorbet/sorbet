# frozen_string_literal: true
# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
end

class SecondPackage < PackageSpec; end # error: Invalid expression in package: `ClassDef` not allowed
#                     ^^^^^^^^^^^ error: Unable to resolve constant `PackageSpec`
