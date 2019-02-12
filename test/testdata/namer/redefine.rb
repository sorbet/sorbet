# typed: true
extend T::Sig

sig {returns(Integer)}
def foo
  1
end
def foo
  2
end
T.reveal_type(foo) # error: Revealed type: `Integer`

def bar
  1
end
sig {returns(Integer)}
def bar
  2
end
T.reveal_type(bar) # error: Revealed type: `Integer`

sig {returns(Integer)}
def too_many_args
  1
end
def too_many_args(a) # error: redefined without matching argument count. Expected: `0`, got: `1`
  2
end
T.reveal_type(too_many_args(2)) # error: Revealed type: `T.untyped`

sig {params(a: Integer).returns(Integer)}
def wrong_args(a)
  1
end
def wrong_args(a:) # error: redefined with mismatched argument attribute `isKeyword`. Expected: `false`, got: `true`
  2
end
T.reveal_type(wrong_args(a: 2)) # error: Revealed type: `T.untyped`

sig {params(a: Integer).returns(Integer)}
def wrong_name(a)
  1
end
def wrong_name(b)
  2
end
T.reveal_type(wrong_name(3)) # error: Revealed type: `Integer`

sig {params(a: Integer).returns(Integer)}
def wrong_repeatedness(a)
  1
end
def wrong_repeatedness(*a) # error: redefined with mismatched argument attribute `isRepeated`. Expected: `false`, got: `true`
  2
end
T.reveal_type(wrong_repeatedness(3)) # error: Revealed type: `T.untyped`

sig {params(a: Integer).returns(Integer)}
def wrong_blockness(a)
  1
end
def wrong_blockness(&a) # error: redefined with mismatched argument attribute `isBlock`. Expected: `false`, got: `true`
  2
end
T.reveal_type(wrong_blockness()) # error: Revealed type: `T.untyped`
