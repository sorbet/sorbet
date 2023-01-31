# frozen_string_literal: true
# typed: strict

class Other < PackageSpec
  import Foo::Bar
  import Foo::Bar::Baz
  import Typical
end
