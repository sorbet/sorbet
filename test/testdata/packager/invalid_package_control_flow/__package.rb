# frozen_string_literal: true
# typed: strict
# enable-packager: true

# Constant definitions/assignments are not OK
SomeConstant = PackageSpec # error: Invalid expression in package: `Assign`

class MyPackage < PackageSpec
  extend T::Helpers # error: Invalid expression in package: `extend` is not allowed
  include T::Helpers # error: Invalid expression in package: `include` is not allowed

  # Things that do computation are not OK
  some_method(method_call_arg) # error: Invalid expression in package: Arguments to functions must be literals
  some_method(ConstantArg) # error: Invalid expression in package: Arguments to functions must be literals
  some_method("add" + "ition") # error: Invalid expression in package: Arguments to functions must be literals
  some_method(1 + 2) # error: Invalid expression in package: Arguments to functions must be literals

  # Complex args are not OK (we can choose to relax this)
  some_method([0,1,2])
  #           ^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
  #           ^^^^^^^ error: Invalid expression in package: `Array`
  some_method({prop: 10})
  #           ^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
  #           ^^^^^^^^^^ error: Invalid expression in package: `Hash`

  # Literals should be fine.
  some_method "Literal"
  some_method 1

  # Methods defs are not OK
  sig {void} # error: Invalid expression in package: Arguments to functions must be literals
# ^^^^^^^^^^ error: Invalid expression in package: `Block` not allowed
  def package_method; end
# ^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: `MethodDef`
# ^^^^^^^^^^^^^^^^^^      error: Invalid expression in package: `RuntimeMethodDefinition`

  sig {void} # error: Invalid expression in package: Arguments to functions must be literals
# ^^^^^^^^^^ error: Invalid expression in package: `Block` not allowed
  def self.static_method; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: `MethodDef`
# ^^^^^^^^^^^^^^^^^^^^^^      error: Invalid expression in package: `RuntimeMethodDefinition`

  # Var defs / assignments are not OK
  @hello = T.let(nil, T.nilable(String))
# ^^^^^^                                 error: Invalid expression in package: `UnresolvedIdent`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: `Assign`
#          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: `Cast` not allowed
#                               ^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
end
