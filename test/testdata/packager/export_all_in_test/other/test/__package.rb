# frozen_string_literal: true
# typed: strict

class Test::Other < PackageSpec
  test!

  import Other
  import Foo::Bar
  import Test::Foo::Bar
  import Typical
  import Test::Typical
end
