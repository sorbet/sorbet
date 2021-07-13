# typed: strict
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
    Z = new
  end
end

T.reveal_type(MyEnum::X === MyEnum::X) # error: Revealed type: `TrueClass`
T.reveal_type(MyEnum::X === MyEnum::Y) # error: Revealed type: `FalseClass`

sig {params(x: MyEnum).void}
def example(x)
  if MyEnum::X === x
    T.reveal_type(MyEnum::X === x) # error: Revealed type: `TrueClass`
    T.reveal_type(MyEnum::Y === x) # error: Revealed type: `FalseClass`
    T.reveal_type(MyEnum::Z === x) # error: Revealed type: `FalseClass`
  else
    T.reveal_type(MyEnum::X === x) # error: Revealed type: `FalseClass`
    T.reveal_type(MyEnum::Y === x) # error: Revealed type: `T::Boolean`
    T.reveal_type(MyEnum::Z === x) # error: Revealed type: `T::Boolean`
    if MyEnum::Y === x
      T.reveal_type(MyEnum::X === x) # error: Revealed type: `FalseClass`
      T.reveal_type(MyEnum::Y === x) # error: Revealed type: `TrueClass`
      T.reveal_type(MyEnum::Z === x) # error: Revealed type: `FalseClass`
    else
      T.reveal_type(MyEnum::X === x) # error: Revealed type: `FalseClass`
      T.reveal_type(MyEnum::Y === x) # error: Revealed type: `FalseClass`
      T.reveal_type(MyEnum::Z === x) # error: Revealed type: `TrueClass`
    end
  end
end
