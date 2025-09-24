# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Prelude < PackageSpec
  prelude_package

  import Application # error: `Prelude` may not `import` non-prelude package `Application`
  test_import Application # error: `Prelude` may not `test_import` non-prelude package `Application`

  export Prelude::A
end
