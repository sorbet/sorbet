# typed: true

class Other
end

class SomeDataInterface
  prop :foo, String, ->{Other}
end

def test
  di = SomeDataInterface.new
  T.reveal_type(di.foo) # error: Revealed type: `String`

  T.reveal_type(di.foo_) # error: Revealed type: `T.nilable(Other)`
  T.reveal_type(di.foo_!) # error: Revealed type: `Other`
end
