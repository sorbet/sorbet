# typed: true

A = T.type_alias {Integer}

class B
  include A
  #       ^ error: Superclasses and mixins may not be type aliases
  #       ^ error: Expected `T::Module[T.anything]` but found `Runtime object representing type: Integer` for argument `arg0`
end
