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
def too_many_args(a) # error: `too_many_args`: Method redefined
  2
end
T.reveal_type(too_many_args(2)) # error: Revealed type: `T.untyped`

sig {params(a: Integer).returns(Integer)}
def wrong_args(a)
  1
end
def wrong_args(a:) # error: `wrong_args`: Method redefined
  2
end
T.reveal_type(wrong_args(a: 2)) # error: Revealed type: `T.untyped`

sig {params(a: Integer).returns(Integer)}
def wrong_name(a)
  1
end
def wrong_name(b) # error: `wrong_name`: Method redefined
  2
end
T.reveal_type(wrong_name(3)) # error: Revealed type: `T.untyped`

sig {params(a: Integer).returns(Integer)}
def wrong_repeatedness(a)
  1
end
def wrong_repeatedness(*a) # error: `wrong_repeatedness`: Method redefined
  2
end
T.reveal_type(wrong_repeatedness(3)) # error: Revealed type: `T.untyped`

sig {params(a: Integer).returns(Integer)}
def wrong_blockness(a)
  1
end
def wrong_blockness(*a) # error: `wrong_blockness`: Method redefined
  2
end
T.reveal_type(wrong_blockness(3)) # error: Revealed type: `T.untyped`
