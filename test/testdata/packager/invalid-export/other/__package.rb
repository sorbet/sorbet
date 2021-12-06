# frozen_string_literal: true
# typed: strict

class Other < PackageSpec
  # Ensure that the bad export in Foo::Bar don't create errors in a package that imports it.
  import Foo::Bar
end
