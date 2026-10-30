# frozen_string_literal: true
# typed: strict

class Other < PackageSpec
  test_import Foo::Bar
  test_import Typical
end
