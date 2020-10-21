# typed: true
extend T::Sig

sig {params(input: T.any(Integer, T::Array[Integer])).returns(Integer)}
def get_value(input)
  case input
  when Integer
    input
  when T::Array[Integer]
    input.first || 0
  else
    T.absurd(input)
  end
end

T::Array[Integer].===(0)
