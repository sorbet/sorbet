# typed: true

module Opus::Foo
  module Bar2
    X = Baz          # error: `Opus::Foo::Baz` is not imported
    T.reveal_type(X) # error: Revealed type: `T.untyped`
    X.prelude!
  end
end
