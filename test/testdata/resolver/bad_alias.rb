# typed: true
class A
  B = type_member

  S = T.type_alias(Integer) # error: Type aliases are not allowed in generic classes
end


class B
  S = T.type_alias(S) # error: Type alias expands to to an infinite type
end

class C
  S1 = T.type_alias(S2)
  S2 = T.type_alias(S1) # error: Type alias expands to to an infinite type
end

class D
  U = T.type_alias([U, U]) # error: Type alias expands to to an infinite type
end
