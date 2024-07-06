# typed: strict
class Module; include T::Sig; end

class A < T::InexactStruct
  prop :foreign_b, T.nilable(String), foreign: -> { B }
  #     ^^^^^^^^^ def: A#foreign_b
  #                                   ^^^^^^^ def: A#foreign_b_
end

class B < T::Struct
end

a = A.new(some_const: '', some_prop: '')
a.foreign_b_
# ^^^^^^^^^^ usage: A#foreign_b_
a.foreign_b_!
# ^^^^^^^^^^^ usage: A#foreign_b_
