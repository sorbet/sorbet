# frozen_string_literal: true
# typed: strict

class RBI < PackageSpec
  prelude!

  export RBI::Foo
  export RBI::Bar
end
