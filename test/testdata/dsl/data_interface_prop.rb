# typed: strict

class Other
end

class SomeDataInterface
  prop :foo, String, ->{Other}
end

def test
  di = SomeDataInterface.new
  T.assert_type!(di.foo, T.nilable(String))
  T.assert_type!(di.foo, String) # error: does not have asserted type

  T.assert_type!(di.foo_, T.nilable(Other))
  T.assert_type!(di.foo_, Other) # error: does not have asserted type
end
