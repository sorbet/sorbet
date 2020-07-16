# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
end

class SecondPackage < PackageSpec; end # error: Package files can only declare one package
