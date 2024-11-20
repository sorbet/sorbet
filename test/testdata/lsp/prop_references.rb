# typed: strict
class Module; include T::Sig; end

class A < T::InexactStruct
  const :some_const, String
  #      ^^^^^^^^^^ def: A#some_const

  prop :some_prop, String
  #     ^^^^^^^^^ def: A#some_prop

  prop :foreign_b, T.nilable(String), foreign: -> { B }
  #     ^^^^^^^^^ def: A#foreign_b
  #                                   ^^^^^^^ def: A#foreign_b_
end

a = A.new(some_const: '', some_prop: '')
a.some_const
# ^^^^^^^^^^ usage: A#some_const
a.some_prop=('')
# ^^^^^^^^^^ usage: A#some_prop
a.some_prop
# ^^^^^^^^^ usage: A#some_prop
a.foreign_b_
# ^^^^^^^^^^ usage: A#foreign_b_

class B < T::Struct
  const :some_const, String
  #      ^^^^^^^^^^ def: B#some_const
  #      ^^^^^^^^^^ usage: B#some_const

  prop :some_prop, String
  #     ^^^^^^^^^ def: B#some_prop
  #     ^^^^^^^^^ usage: B#some_prop

  sig { void }
  def example
    p(@some_const)
    # ^^^^^^^^^^^ usage: B#some_const
    p(@some_prop)
    # ^^^^^^^^^^ usage: B#some_prop
  end
end

b = B.new(some_const: '', some_prop: '')
b.some_const
# ^^^^^^^^^^ usage: B#some_const
b.some_prop=('')
# ^^^^^^^^^^ usage: B#some_prop
b.some_prop
# ^^^^^^^^^ usage: B#some_prop
