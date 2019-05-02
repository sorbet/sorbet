# typed: false
def test_csend
  foo&.bar
  foo&.bar { |x| x }
  foo&.bar = 5
  foo&.bar += 5
end

# desugar should not require `nil?` support
BasicObject.new&.__id__

# desugar should not conflate `false` and `nil`
class A
  extend T::Sig

  sig do
    params(x: T.any(FalseClass, NilClass))
      .returns(T.nilable(TrueClass))
  end
  def foo(x)
    x&.|true
  end
end
