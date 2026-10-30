# typed: true

module Opus::Foo
  X = Baz          # error: `Opus::Foo::Baz` is not imported
  T.reveal_type(X) # error: Revealed type: `T.untyped`
end
