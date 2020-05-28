# typed: true

extend T::Sig

sig {params(a: String).returns(String)}
def test(a:)
  a
end


test(
  a: 10
# ^^^^^ error: Expected `String` but found `Integer(10)`
)
