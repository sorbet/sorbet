# typed: true
class A
  extend T::Generic
  B = type_member

  S = T.type_alias {Integer}
end


class B
  S = T.type_alias {S} # error: Unable to resolve right hand side of type alias `B::S`
end

class C
  S1 = T.type_alias {S2} # error: Unable to resolve right hand side of type alias `C::S1`
  S2 = T.type_alias {S1} # error: Unable to resolve right hand side of type alias `C::S2`
end

class D
  U = T.type_alias {[U, U]} # error: Unable to resolve right hand side of type alias `D::U`
end

E = T.type_alias {E} # error: Unable to resolve right hand side of type alias `E`
