# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Prelude < PackageSpec
  prelude!

  import Application # error: `Prelude` may not `import` non-prelude package `Application`
end
