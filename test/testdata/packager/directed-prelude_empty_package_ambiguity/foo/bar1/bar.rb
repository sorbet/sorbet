# typed: true

module Opus::Foo
  module Bar1
    X = Baz
    T.reveal_type(X) # error: Revealed type: `T.class_of(Baz)`
    X.prelude!       # error: Method `prelude!` does not exist
  end
end
