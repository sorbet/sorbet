# typed: true

module Opus::Foo
  # Definitions in unpackaged stray.rb must not populate any of these namespaces.
  Extra::X # error: Unable to resolve constant `Extra`
  Thing::FROM_STRAY # error: Unable to resolve constant `FROM_STRAY`
  ::Opus::FROM_STRAY # error: Unable to resolve constant `FROM_STRAY`
  ::ROOT_VALUE # error: Unable to resolve constant `ROOT_VALUE`
  ::RootClass # error: Unable to resolve constant `RootClass`
  ::String::FROM_STRAY # error: Unable to resolve constant `FROM_STRAY`

  # The real payload String must stay available.
  T.reveal_type("".length) # error: Revealed type: `Integer`
end
