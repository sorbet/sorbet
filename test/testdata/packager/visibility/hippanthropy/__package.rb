# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Hippanthropy < PackageSpec
  # this import will be okay because `Foo` includes `visible_to "tests"`
  test_import Foo
end
