# typed: strict
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new('y')
  end
end

class OtherEnum < T::Enum
  enums do
    Z = new
  end
end

class Stringy < String ; end

sig {params(t_enum: T::Enum,
            my_enum: MyEnum,
            other_enum: OtherEnum,
            x: MyEnum::X,
            y: MyEnum::Y,
            z: OtherEnum::Z,
            s: String,
            stringy: Stringy).void}
def f(t_enum, my_enum, other_enum, x, y, z, s, stringy)
  T.reveal_type(x===x)                # error: Revealed type: `TrueClass`

  T.reveal_type(t_enum===t_enum)      # error: Revealed type: `T::Boolean`
  T.reveal_type(my_enum===my_enum)    # error: Revealed type: `T::Boolean`

  T.reveal_type(t_enum===my_enum)     # error: Revealed type: `T::Boolean`
  T.reveal_type(my_enum===t_enum)     # error: Revealed type: `T::Boolean`

  T.reveal_type(t_enum===x)           # error: Revealed type: `T::Boolean`
  T.reveal_type(x===t_enum)           # error: Revealed type: `T::Boolean`

  T.reveal_type(my_enum===x)          # error: Revealed type: `T::Boolean`
  T.reveal_type(x===my_enum)          # error: Revealed type: `T::Boolean`

  T.reveal_type(x===y)                # error: Revealed type: `FalseClass`

  T.reveal_type(my_enum===other_enum) # error: Revealed type: `FalseClass`
  T.reveal_type(z===my_enum)          # error: Revealed type: `FalseClass`
  T.reveal_type(my_enum===z)          # error: Revealed type: `FalseClass

  T.reveal_type(t_enum===s)           # error: Revealed type: `T::Boolean`
  T.reveal_type(my_enum===s)          # error: Revealed type: `T::Boolean`
  T.reveal_type(x===s)                # error: Revealed type: `T::Boolean`
  T.reveal_type(x===stringy)          # error: Revealed type: `T::Boolean`
end
