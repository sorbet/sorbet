# typed: strict
extend T::Sig

sig {returns(Integer)}
def foo

end


sig {returns(Integer)}
def foo
  nil
end

sig {params(x: Integer).void}
def takes_integer(x)
end

sig {void}
def bar
  if T.unsafe(nil)
    x = ''
  else
    x = 1
  end

  takes_integer(x)
end

T.must(0)

sig {params(blk: T.proc.returns(Integer)).void}
def takes_int_block(&blk)
end

takes_int_block do
  ''
end

sig {params(blk: T.proc.returns(String)).void}
def takes_string_block(&blk)
  takes_int_block(&blk)
end

T.assert_type!(T.unsafe(nil), String)

T.reveal_type(42)

sig {params(x: T.any(Integer, String)).void}
def t_absurd(x)
  case x
  when String then 'string'
  else T.absurd(x)
  end
end

1&.even?

top_level_does_not_exist

T.let(nil, Integer)

T.cast(1, Integer)

sig {void}
def returns_void; end

returns_void.foo

sig {params(xs: T::Array[Integer]).void}
def takes_int_array(xs); end
xs = [1, 2, T.let(3, T.nilable(Integer))]
takes_int_array(xs)
T.let(xs, T::Array[Integer])
