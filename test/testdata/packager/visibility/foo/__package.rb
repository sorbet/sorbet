# frozen_string_literal: true
# typed: strict
# enable-packager: true
# skip-package-import-visibility-check-for: SkipCheck::For

class Foo < PackageSpec
  visible_to Bar
  visible_to Quux
           # ^^^^ error: Unable to resolve constant `Quux`

  visible_to 'tests'
  visible_to 'a different string'
           # ^^^^^^^^^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant or
end
