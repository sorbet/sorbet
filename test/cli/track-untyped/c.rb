# typed: strong
extend T::Sig

sig { params(arg0: Integer).returns(T::Boolean) }
def example_typed(arg0)
  result = arg0.even?

  if result
    puts(result)
  end

  result
end
