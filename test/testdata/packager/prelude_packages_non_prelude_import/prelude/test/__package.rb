# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Test::Prelude < PackageSpec
  test!
  prelude_package

  import Prelude
  import Application # error: Prelude package `Test::Prelude` may not `import` non-prelude package `Application`
end
