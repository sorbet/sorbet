# typed: strict
# spacer for exclude-from-file-update

class Child < Parent
  Elem1 = type_member
  Elem2 = type_member

  sig {params(x: Elem1, y: Elem2).returns(Elem2)}
  def example_child(x, y)
    T.reveal_type(x) # error: `Child::Elem1`
    T.reveal_type(y) # error: `Child::Elem2`

    y
  end
end

res = Parent[Integer, String].new.example(0, '')
#
#
T.reveal_type(res) # error: `Integer`

res = Child[Integer, String].new.example(0, '')
#
#
T.reveal_type(res) # error: `Integer`
res = Child[Integer, String].new.example_child(0, '')
#
#
T.reveal_type(res) # error: `String`
