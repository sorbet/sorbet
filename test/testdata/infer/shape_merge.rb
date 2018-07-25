# typed: true

T.assert_type!(
  {
    hi: "there",
  },
  {
    hi: String,
  }
)

T.assert_type!(
  {
    hi: "there",
  }.merge({
            llamas: 17
          }),
  {
    hi: String,
    llamas: Integer,
  }
)

T.assert_type!(
  {
    hi: "there",
    llamas: 8,
  }.merge({
            llamas: :none,
            alpacas: 5
          }),
  {
    hi: String,
    llamas: Symbol,
    alpacas: Integer,
  }
)

def has_kwargs(a:, b:, c:)
end

has_kwargs({}.merge(a: 1).merge(b: 3).merge(c: 4))
