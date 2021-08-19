# typed: strict

class Root::Nested < PackageSpec
  strict_exports true
  export Root::Nested::SomeClass::Deeper
  export Root::Nested::OtherClass::Deep2
end
