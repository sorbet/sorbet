# typed: strict
# enable-packager: true

class Root < PackageSpec
  #   ^^^^ def: rootpkg

  export Root::A
end
