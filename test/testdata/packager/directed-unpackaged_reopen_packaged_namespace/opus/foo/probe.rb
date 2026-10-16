# typed: true

module Opus::Foo
  # These all currently resolve to definitions in unpackaged stray.rb.
  Extra::X
  Thing::FROM_STRAY
  ::Opus::FROM_STRAY
  ::ROOT_VALUE
  ::RootClass
  ::String::FROM_STRAY

  # The real payload String must stay available.
  T.reveal_type("".length) # error: Revealed type: `Integer`
end
