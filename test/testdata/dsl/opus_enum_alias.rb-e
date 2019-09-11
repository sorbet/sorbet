# typed: true
# disable-fast-path: true
extend T::Sig

module Opus
  class Enum
    extend T::Generic
    def initialize(x = nil)
    end
  end
end

class MyEnum < Opus::Enum
  X = new
end

# We probably never want to allow this
AliasX = MyEnum::X

# We want to allow this eventually, but Sorbet isn't architected correctly for
# us to do it.
#
# Type alias parsing happens in ResolveConstantsWalk, which is before
# ResolveSignaturesWalk populates the resultType of static fields declared with
# T.let.
TypeAliasX = T.type_alias(MyEnum::X) # error: Constant `MyEnum::X` is not a class or type alias

sig {params(x: AliasX).void} # error: Constant `AliasX` is not a class or type alias
def with_class_alias(x); end

sig {params(x: TypeAliasX).void}
def with_type_alias(x); end
