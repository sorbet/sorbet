# typed: true

module Foo
  class Range; end
  T.reveal_type(0..1) # error: Revealed type: `T::Range[Integer]`
  T.reveal_type(0...1) # error: Revealed type: `T::Range[Integer]`
end
