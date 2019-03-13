# typed: true

extend T::Sig

def foo
  target = [1,2].map do |x|
    3
    break x
    4
  end
  T.reveal_type(target) # error: Revealed type: `T.any(T::Array[T.noreturn], Integer)`
end
puts foo

sig { params(blk: T.proc.params(x: Integer).returns(String)).returns(String) }
def bar(&blk)
  "foo bar"
end

a = bar do |x|
  if x > 5
    break 10
  end
  "test"
end
T.reveal_type(a) # error: Revealed type: `T.any(String, Integer)`

b = bar do |x|
  if x > 5
    break
  end
  "test"
end
T.reveal_type(b) # error: Revealed type: `T.nilable(String)`

c = while 1.to_s == ""
      break :abc if 1.to_s == ""
    end
T.reveal_type(c) # error: Revealed type: `T.nilable(Symbol)`
