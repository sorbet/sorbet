# frozen_string_literal: true
# typed: strict
# enable-packager: true
# allow-relaxed-packager-checks-for: SkipCheck::For

class Foo < PackageSpec
  visible_to Bar
  visible_to Quux
           # ^^^^ error: Unable to resolve constant `Quux`

  visible_to 'tests'
  visible_to 'a different string'
           # ^^^^^^^^^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant or

  visible_to Nested::*

  visible_to Nested::*(0)
           # ^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant or
           # ^^^^^^ error: Unable to resolve constant `Nested`
  visible_to Nested::*(x: 0)
           # ^^^^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant or
           # ^^^^^^ error: Unable to resolve constant `Nested`
  visible_to Nested::* {}
           # ^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant or
           # ^^^^^^^^^^^^ error: Invalid expression in package: `Block` not allowed
           # ^^^^^^ error: Unable to resolve constant `Nested`

  visible_to Nested::*::Blah
           # ^^^^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant
           # ^^^^^^ error: Unable to resolve constant `Nested`
           # ^^^^^^^^^^^^^^^ error: Dynamic constant references
end
