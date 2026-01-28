# frozen_string_literal: true
# typed: strict

class RBI < PackageSpec
  prelude_package

  export RBI::Foo
  export RBI::Bar
end
