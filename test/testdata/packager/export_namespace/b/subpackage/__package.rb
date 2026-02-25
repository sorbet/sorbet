# typed: strict

class Root::B::Subpackage < PackageSpec
  import Root::B

  export Root::B::Subpackage::Example
end
