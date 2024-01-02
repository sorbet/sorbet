# typed: true
extend T::Sig

sig { returns(T::Array[Integer]) }
def foo
  return 1, ""
end

x = loop do
  break 1, 2
end

T.reveal_type(x)
