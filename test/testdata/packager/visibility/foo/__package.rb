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
           #         ^       error: Method `*` does not exist on `T.class_of(Nested)`
  visible_to Nested::*(x: 0)
           # ^^^^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant or
           #         ^       error: Method `*` does not exist on `T.class_of(Nested)`
  visible_to Nested::* {}
           # ^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant or
           # ^^^^^^^^^^^^ error: Invalid expression in package: `Block` not allowed
           #         ^       error: Method `*` does not exist on `T.class_of(Nested)`

  visible_to Nested::*::Blah
           # ^^^^^^^^^^^^^^^ error: Argument to `visible_to` must be a constant
           # ^^^^^^^^^^^^^^^ error: Dynamic constant references
           #         ^       error: Method `*` does not exist on `T.class_of(Nested)`
end
