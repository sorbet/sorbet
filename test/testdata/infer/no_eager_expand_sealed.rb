# typed: true
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

sig { params(enums: T::Array[MyEnum], orig: T.nilable(MyEnum)).void }
def example(enums, orig)
  current_enum = orig
  if current_enum.nil?
    return
  end
  T.reveal_type(current_enum) # error: `MyEnum`

  enums.each do |enum|
    # If we don't short circuit in dropSubtypesOf, we might eagerly expand
    # sealed subclasses, causing `current_enum` expand to `T.any(...)` instead
    # of remaining as `MyEnum`, which would cause a pinning error here.
    current_enum = enum
  end
end
