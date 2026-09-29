# frozen_string_literal: true
# typed: strict
# allow-relaxed-packager-checks-for: RelaxedPrelude

class RelaxedPrelude < PackageSpec
  prelude!

  import Application # error: `RelaxedPrelude` may not `import` non-prelude package `Application`
end
