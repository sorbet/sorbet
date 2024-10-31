# typed: strict

class A < T::Struct

  const :name, String
#        ^^^^         error: Argument does not have asserted type `Float`
  prop :name, Integer
# ^^^^^^^^^^^^^^^^^^^ error: Expected `Integer`
# ^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`
# ^^^^^^^^^^^^^^^^^^^ error: Unknown argument
# ^^^^^^^^^^^^^^^^^^^ error: duplicate argument name
#       ^^^^          error: Argument does not have asserted type `Float`
#       ^^^^          error: Expected `Float`

  prop :age, Integer
# ^^^^^^^^^^^^^^^^^^ error: Expected `Integer` but found
#       ^^^          error: Argument does not have asserted type `Float`
#       ^^^          error: Expected `Float`
  const :name, Float
# ^^^^^^^^^^^^^^^^^^ error: Malformed `sig`
# ^^^^^^^^^^^^^^^^^^ error: Unknown argument
# ^^^^^^^^^^^^^^^^^^ error: duplicate argument name
#        ^^^^        error: Argument does not have asserted type `Float`

  const :age, Float
# ^^^^^^^^^^^^^^^^^ error: Malformed `sig`
# ^^^^^^^^^^^^^^^^^ error: Unknown argument
# ^^^^^^^^^^^^^^^^^ error: duplicate argument name
#        ^^^        error: Argument does not have asserted type `Float`

end
