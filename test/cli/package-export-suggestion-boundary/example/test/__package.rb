# frozen_string_literal: true
# typed: strict

class Test::Some::Example < PackageSpec
  test!

  import Some::Example

  export Test::Some::Example::A
end
