# typed: true

class Parent < T::Enum
  enums do
    V0 = new
    V1 = new
  end
end

class Child < Parent
  #           ^^^^^^ error: `Parent` descends from `T::Enum` and thus cannot be subclassed by `Child`
  enums do
    V2 = new
  end
end
