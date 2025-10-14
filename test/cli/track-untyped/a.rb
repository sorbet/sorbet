# typed: strong
extend T::Sig

sig { params(arg0: T.untyped).returns(Integer) }
def example_untyped(arg0)
  result = arg0.foo

  if result
    puts(result)
  end

  if arg0.is_a?(Integer)
    T.reveal_type(arg0)
  elsif arg0.nil?
    T.reveal_type(arg0)
  end

  result
end
