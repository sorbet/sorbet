# typed: strict
# enable-packager: true

class Root < PackageSpec
  export Root::A
  export Root::Bare
  export Root::BarePrivate
  export Root::ModifierParent

  prelude!
end
