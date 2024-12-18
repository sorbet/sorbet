# typed: true

class LongTypeParamsHoverTest
  extend T::Sig

  sig {type_parameters(:"very long spaced parameter name that will need to wrap")
    .params(x: T.type_parameter(:"very long spaced parameter name that will need to wrap"))
    .returns(T.type_parameter(:"very long spaced parameter name that will need to wrap"))}
  def test_method(x)
    # ^ hover-line: 2 sig do
    # ^ hover-line: 3   type_parameters(:"very long spaced parameter name that will need to wrap")
    # ^ hover-line: 4   .params(
    # ^ hover-line: 5     x: T.type_parameter(:"very long spaced parameter name that will need to wrap")
    # ^ hover-line: 6   )
    # ^ hover-line: 7   .returns(T.type_parameter(:"very long spaced parameter name that will need to wrap"))
    # ^ hover-line: 8 end
    # ^ hover-line: 9 def test_method(x); end
    x
  end
end
