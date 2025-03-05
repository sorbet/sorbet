# frozen_string_literal: true
# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
end

class SecondPackage < PackageSpec; end # error: Package files can only declare one package
#                     ^^^^^^^^^^^ error: Unable to resolve constant `PackageSpec`
