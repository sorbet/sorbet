# typed: strict
module Root::ImportsNested
  class Foo
    Root::Nested::SomeClass::Deeper.new
  end
end
