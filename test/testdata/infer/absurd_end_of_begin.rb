# typed: strict
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

sig {returns(MyEnum)}
def get_my_enum
  MyEnum::X
end

begin
  x = get_my_enum
  y = get_my_enum
  case x
  when MyEnum::X
    raise
  when MyEnum::Y
    raise
  else T.absurd(x)
  end
  T.reveal_type(x) # error: `T.any(MyEnum::X, MyEnum::Y)`
  T.reveal_type(y) # error: `MyEnum`
rescue
  # Sorbet doesn't stitch the individual enum values back together once they've been split.
  T.reveal_type(x) # error: `T.nilable(T.any(MyEnum::X, MyEnum::Y))`
  T.reveal_type(y) # error: `T.nialble(MyEnum)`
end
