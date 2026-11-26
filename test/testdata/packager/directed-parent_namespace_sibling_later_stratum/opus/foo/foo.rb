# typed: true

module Opus::Foo
  X = Baz # error: `Opus::Foo::Baz` is not imported
end

T.reveal_type(Opus::Foo::X) # error: Revealed type: `T.untyped`
