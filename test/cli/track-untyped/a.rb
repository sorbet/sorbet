# typed: strong
extend T::Sig

sig { params(arg0: T.untyped).returns(Integer) }
def example_untyped(arg0)
  result = arg0.foo

  if result
    puts(result)
  end

  result
end
