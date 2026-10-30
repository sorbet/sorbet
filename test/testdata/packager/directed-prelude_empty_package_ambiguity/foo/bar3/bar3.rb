# typed: true

module Opus::Foo
  module Bar3
    X = BadConstant  # error: Unable to resolve constant `BadConstant
    T.reveal_type(X) # error: Revealed type: `T.untyped`
    X.prelude!
  end
end
