# typed: true
extend T::Sig

class A; end

X = T.type_alias {A}
Y = T.type_alias {T.class_of(A)}

sig {returns(T.class_of(A))}
def test1
  X
end

sig {returns(T.class_of(A))}
def test2
  Y
end
