# typed: strict
# enable-packager: true

class Root < PackageSpec
  export Root::A
  export Root::ModifierParent

  prelude!
end
