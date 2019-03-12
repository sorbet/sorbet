# typed: true
class A
  B = type_member

  S = T.type_alias(Integer) # error: Type aliases are not allowed in generic classes
end


class B
  S = T.type_alias(S) # error: Type alias `B::S` expands to to an infinite type
end

class C
  S1 = T.type_alias(S2)
  S2 = T.type_alias(S1) # error: Type alias `C::S2` expands to to an infinite type
end

class D
  U = T.type_alias([U, U]) # error: Type alias `D::U` expands to to an infinite type
end

E = T.type_alias(E) # error: Type alias `E` expands to to an infinite type
