# typed: strict
# enable-packager: true
# disable-fast-path: true
# ^ The "Only modules can be `include`d." error, unfortunately, doesn't surface the same errors on the fast path.
# This is a restriction in other tests.

class A < PackageSpec
  import 123
       # ^^^ error: Argument to `import` must be a constant
       # ^^^ error: Expected `T.class_of(PackageSpec)`
  import "hello"
       # ^^^^^^^ error: Argument to `import` must be a constant
       # ^^^^^^^ error: Expected `T.class_of(PackageSpec)`
  import method
       # ^^^^^^ error: Argument to `import` must be a constant
       # ^^^^^^ error: Expected `T.class_of(PackageSpec)`
       # ^^^^^^ error: Not enough arguments
  import REFERENCE # error: Cannot find package `REFERENCE`
       # ^^^^^^^^^ error: Unable to resolve constant `REFERENCE`
  # Despite the above, this import should work.
  import B
  # But this one will fail; no duplicate imports allowed.
  import B # error: Duplicate package import

  export 123 # error: Argument to `export` must be a constant
  export "hello" # error: Argument to `export` must be a constant
  export method # error: Argument to `export` must be a constant
       # ^^^^^^ error: Not enough arguments 
  export REFERENCE # Works; it's a constant.
  export AClass

  # AModule export should still work. The rest will fail / cause errors.
  export_methods 123, "hello", method, REFERENCE, AModule, AClass
               # ^^^ error: Argument to `export_methods` must be a constant
                    # ^^^^^^^ error: Argument to `export_methods` must be a constant
                             # ^^^^^^ error: Argument to `export_methods` must be a constant
                             # ^^^^^^ error: Not enough arguments
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only modules can be `include`d. This module or class includes `A::AClass`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only modules can be `include`d. This module or class includes `A::ASecondClass`
end
