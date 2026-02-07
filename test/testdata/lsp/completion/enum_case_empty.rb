# typed: true
extend T::Sig

class MyEnum < T::Enum
  enums do
  end
end

sig { params(my_enum: MyEnum).void }
def example(my_enum)
  my_enum.cas
  #       ^^^ error: does not exist
  #          ^ apply-completion: [A] item: 0
end
