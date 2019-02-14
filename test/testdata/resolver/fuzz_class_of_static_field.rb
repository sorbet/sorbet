# typed: true
A1 = 1
A2 = T.type_alias(Integer)

extend T::Sig

sig {params(a: T.class_of(A1)).void} # error: T.class_of can't be used with a constant field
def foo(a); end

sig {params(a: T.class_of(A2)).void} # error: T.class_of can't be used with a T.type_alias
def foo(a); end
