# typed: true
A1 = 1
A2 = T.type_alias {Integer}

extend T::Sig

sig {params(a: T.class_of(A1)).void}
#              ^^^^^^^^^^^^^^ error: T.class_of can't be used with a constant field
#                         ^^  error: Unexpected bare `Integer` value found in type position
def foo(a); end

sig {params(a: T.class_of(A2)).void} # error: T.class_of can't be used with a T.type_alias
def foo(a); end

class Foo
  extend T::Sig
  extend T::Generic

  A3 = type_member

  sig {params(a: T.class_of(A3)).void} # error: T.class_of can't be used with a T.type_member
  def foo(a); end
end
