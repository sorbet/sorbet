# typed: true

module ::MyPackage
  class Foo # error: File belongs to package `MyPackage::Prelude` but defines a constant that does not match this namespace
  end
end
