# typed: true

extend T::Sig

sig {returns(T.nilable(T::Array[Integer]))}
def foo
  nil
end

# this if used to cause an unreachable error
if (a, b = foo)
  puts 1
else
  puts 2
end

T.reveal_type((a, b = foo)) # error: Revealed type: `T.nilable(T::Array[Integer])`
T.reveal_type((c, *, d = 100)) # error: Revealed type: `Integer(100)`
