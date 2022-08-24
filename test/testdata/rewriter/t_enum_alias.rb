# typed: true
extend T::Sig

class MyEnum < T::Enum
  enums do
  X = new
  end
end

# We probably never want to allow this
AliasX = MyEnum::X

TypeAliasX = T.type_alias {MyEnum::X}

sig {params(x: AliasX).void} # error: Constant `AliasX` is not a class or type alias
def with_class_alias(x); end

sig {params(x: TypeAliasX).void}
def with_type_alias(x); end
