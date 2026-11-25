# typed: true

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

def example
  MyEnum::X.
  #         ^ completion: serialize, ...
end # error: unexpected token "end"
