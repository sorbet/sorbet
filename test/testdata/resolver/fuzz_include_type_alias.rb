# typed: true

A = T.type_alias {Integer}

class B
  include A
  #       ^ error: Superclasses and mixins may not be type aliases
  #       ^ error: Expected `Module` but found `<Type: Integer>` for argument `arg0`
end
