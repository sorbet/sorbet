# typed: true

module Opus::Foo
  # Definitions in unpackaged stray.rb must not populate package namespaces.
  Extra::X # error: Unable to resolve constant `Extra`
  Thing::FROM_STRAY # error: Unable to resolve constant `FROM_STRAY`
  ::Opus::FROM_STRAY # error: Unable to resolve constant `FROM_STRAY`

  # But they may define root constants and reopen payload classes, like unpackaged RBIs do.
  ::ROOT_VALUE
  ::RootClass
  ::String::FROM_STRAY
  ::String::QUALIFIED_FROM_STRAY

  # The real payload String must stay available.
  T.reveal_type("".length) # error: Revealed type: `Integer`
end
